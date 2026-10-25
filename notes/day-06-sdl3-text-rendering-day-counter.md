# Day 6 — SDL3 Text Rendering Day Counter

> **Project:** `small-farming-rpg`  
> **Source file:** `sdl3_window.cpp`  
> **Environment:** GNU/Linux  
> **Checkpoint:** The game displays a day label inside the window and updates it when the player presses `E` to advance the day.

---

## What we accomplished

During Day 6, we added text rendering to the SDL3 game using **SDL3_ttf**.

By the end of the session, the game could:

- Initialize SDL3_ttf.
- Load a font already available on the system.
- Render the text `Day 1` into an image-like surface.
- Convert that surface into a texture that SDL's renderer can draw.
- Draw the text texture inside the game window.
- Create a replacement text texture when the day number changes.
- Keep the old texture until the new one has been created successfully.
- Update the destination rectangle's width and height to match the new text.

The main new idea was not simply “how to draw text.” It was learning that **text is rendered into image data, converted into a texture, and then drawn like the other textures in the game**.

## 1. Why did we need SDL_ttf?

Before Day 6, we were drawing terrain and crop sprites from image textures. SDL's renderer can draw those textures, but it does not turn a string such as `"Day 1"` into letters by itself.

SDL3_ttf is a companion library that lets us load a font and render text using it.

We added this header with our other includes:

```cpp
#include <SDL3_ttf/SDL_ttf.h>
```

This header makes SDL_ttf's types and functions available to our C++ source file. Including a header is not by itself enough to link the library, so we also added `sdl3-ttf` to the `pkg-config` build command.

Our command became:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic \
    sdl3_window.cpp -o sdl3_window \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
```

The package name used with `pkg-config` is `sdl3-ttf`. The include name is `SDL3_ttf/SDL_ttf.h`. These names serve different purposes, so they do not need to look identical.

As the number of dependencies grows, we can move this command into a `Makefile` so we do not need to type it out every time. That is a useful future improvement, but it was not necessary to understand text rendering today.

## 2. Initialize SDL_ttf

We already initialize SDL itself before creating the window and renderer. SDL_ttf needs its own initialization too:

```cpp
if(!TTF_Init()) {
    std::cerr << "SDL_ttf failed to initialize: "
              << SDL_GetError() << '\n';

    // Release resources that were already created.
    // The actual cleanup statements in sdl3_window.cpp
    // should stay in the same order as our other error paths.
    SDL_DestroyTexture(terrainSheet);
    SDL_DestroyTexture(cropSheet);
    SDL_DestroyRenderer(renderingPointer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
}
```

The important idea is the order of operations:

1. Initialize SDL.
2. Create the window and renderer.
3. Load the textures needed by the game.
4. Initialize SDL_ttf before using its font functions.

If initialization fails, the program should stop instead of continuing as though text rendering is available.

The cleanup shown here is included to explain the failure path. Keep the exact cleanup appropriate to the resources that have been created at that point in your own file.

## 3. Open a font

We chose a font already installed on the GNU/Linux system, rather than adding a new font asset to the repository:

```cpp
TTF_Font* font = TTF_OpenFont(
    "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
    24.0f
);

if(font == nullptr) {
    std::cerr << "Font failed to load: " << SDL_GetError() << '\n';
    // Clean up the resources already created, then return.
}
```

### What is `TTF_Font* font`?

`TTF_OpenFont()` loads a font and returns a pointer to the resulting `TTF_Font` object. We store that pointer in `font` so we can use the same loaded font when rendering text.

If the function returns `nullptr`, the font was not opened successfully, and we must not continue as though `font` were valid.

### What does `24.0f` mean?

It is the font's point-size argument. We chose 24 as a starting size for the day label. It is not the same thing as specifying that the destination rectangle must be 24 pixels high; the rendered text's actual surface dimensions are provided by SDL_ttf.

### Resource ownership

Because opening a font creates a resource, we must close it when we are finished:

```cpp
TTF_CloseFont(font);
```

Close the font **before** shutting down SDL_ttf:

```cpp
TTF_CloseFont(font);
TTF_Quit();
SDL_Quit();
```

That is the relevant order for the font/library shutdown. Keep texture cleanup before renderer cleanup, because the textures belong to the renderer.

---

## 4. The most important part: how text rendering works

This is the main concept from Day 6. Read the stages in order; each stage produces something the next stage needs.

```text
C++ text ("Day 1")
        |
        v
TTF_RenderText_Blended(...)
        |
        v
SDL_Surface*  — rendered text pixels in surface memory
        |
        v
SDL_CreateTextureFromSurface(...)
        |
        v
SDL_Texture*  — texture that SDL's renderer can draw
        |
        v
SDL_RenderTexture(...)
        |
        v
Text appears in the game window
```

These stages are related, but they are **not the same operation**.

### Stage A — The text string

The first input is the text we want to show, for example:

```cpp
"Day 1"
```

At first, the string was fixed text. Later, we made the string from the current `day` value so it could change when the day advances.

A string is only the characters we want to show. It is not yet a picture, surface, or texture, so SDL's renderer cannot draw the string directly with `SDL_RenderTexture()`.

### Stage B — Render the string into an `SDL_Surface`

We used SDL_ttf like this:

```cpp
SDL_Color textColor{255, 255, 255, 255};

SDL_Surface* daySurface = TTF_RenderText_Blended(
    font, "Day 1", 0, textColor
);
```

Let's unpack the important parts:

- `font` is the font we opened earlier.
- `"Day 1"` is the text to render.
- `0` tells this function to use a null-terminated string and determine its length automatically.
- `textColor` is the text color. The four values in `SDL_Color` are red, green, blue, and alpha. `255, 255, 255, 255` means fully opaque white.
- The function returns an `SDL_Surface*` containing the rendered text pixels, or `nullptr` if rendering failed.

A **surface** is an image-like block of pixel data in memory. At this point, SDL_ttf has turned the characters into visible pixels, but we have not yet made the renderer's texture from those pixels.

Check the result before using it:

```cpp
if(daySurface == nullptr) {
    std::cerr << "Text rendering failed: " << SDL_GetError() << '\n';
    // Clean up and handle the failure.
}
```

The exact error message is less important than the rule: never assume an allocation or resource-creation function succeeded without checking when failure is possible.

### Stage C — Convert the surface into an `SDL_Texture`

Next, we create a texture from the surface:

```cpp
SDL_Texture* dayTexture = SDL_CreateTextureFromSurface(
    renderingPointer, daySurface
);
```

This uses:

- `renderingPointer`: the renderer that will draw the texture.
- `daySurface`: the rendered pixels that will become the texture's image.

A **texture** is the representation our SDL renderer uses for drawing this image. This is the same kind of object we used for our crop and terrain sprite sheets.

The surface and texture are different resources. Creating the texture does **not** mean that the surface variable has turned into a texture. We now have two resources, and each has its own cleanup function.

Once the texture has been created successfully, the temporary surface is no longer needed for drawing. So we destroy the surface:

```cpp
SDL_DestroySurface(daySurface);
```

Before destroying a surface, take any measurements we still need from it. In our program, we use its width and height to set the destination rectangle.

### Stage D — Draw the texture using `SDL_RenderTexture`

The draw call is:

```cpp
SDL_RenderTexture(
    renderingPointer,
    dayTexture,
    nullptr,
    &dayRect
);
```

We already used `SDL_RenderTexture()` to draw our terrain, so the function itself is not new. The main difference is what texture we pass to it.

The arguments mean:

| Argument | Meaning in this call |
|---|---|
| `renderingPointer` | The renderer doing the drawing. |
| `dayTexture` | The texture containing the rendered text. |
| `nullptr` | Use the entire texture as the source; we do not need to select a smaller region of the text image. |
| `&dayRect` | The destination rectangle: the position and size of the text on the window. |

Remember the distinction from our terrain code:

- `backgroundSource` was already a pointer to a source rectangle, so we passed it directly.
- `dayRect` is a rectangle object, so we pass its address as `&dayRect`.
- `nullptr` in the source-rectangle argument means the whole texture should be used.

We place the draw call in the **rendering loop**, before `SDL_RenderPresent(renderingPointer)`. The loop clears and redraws the frame, so the day label needs to be drawn on each frame along with the background, soil, crops, and selection outline.

The texture does not have to be created on every frame. We create it when the text needs to change, then draw the currently stored texture on each frame.

### Surface versus texture: a quick comparison

| `SDL_Surface` | `SDL_Texture` |
|---|---|
| Holds rendered pixel data in surface memory. | Holds the image in a form the renderer can draw. |
| Created here by `TTF_RenderText_Blended()`. | Created here by `SDL_CreateTextureFromSurface()`. |
| We read its dimensions to size the destination rectangle. | We pass it to `SDL_RenderTexture()`. |
| Destroyed with `SDL_DestroySurface()`. | Destroyed with `SDL_DestroyTexture()`. |

A useful mental model is: **surface = rendered image data; texture = drawable image for the renderer.** This is a practical model for our current SDL workflow, not a claim that surfaces and textures are interchangeable.

---

## 5. Positioning the text: `dayRect`

We want the label near the top-left corner of the window. Its destination rectangle stores where it goes and how large it should be:

```cpp
SDL_FRect dayRect{
    24.0f,
    24.0f,
    static_cast<float>(daySurface->w),
    static_cast<float>(daySurface->h)
};
```

The fields are ordered as `x`, `y`, `w`, and `h`:

- `x = 24.0f`: 24 pixels from the window's left edge.
- `y = 24.0f`: 24 pixels from the window's top edge.
- `w`: the rendered surface's width, converted to `float`.
- `h`: the rendered surface's height, converted to `float`.

The `static_cast<float>(...)` conversions explicitly convert the integer dimensions provided by the surface to the floating-point values used by `SDL_FRect`.

Why read the surface dimensions rather than guess them? Because the actual width and height depend on the chosen font, font size, and text. We use the dimensions SDL_ttf produced so the texture's destination rectangle begins at the correct size.

We then destroy the surface. This is safe because the rectangle now stores copies of the width and height; it does not retain pointers to the surface's dimensions.

---

## 6. Why the day label did not change when `day` changed

At startup, we rendered the literal text `"Day 1"` into a surface and then created `dayTexture` from that surface.

When the player presses `E`, our game logic increments the integer:

```cpp
day++;
```

But increasing `day` only changes the integer in C++ memory. It does not rewrite pixels that were already rendered into a surface or texture.

Think of `dayTexture` as a small picture of the words that existed when we created it. It is not a live connection to the variable `day`.

Therefore, to display the new day, we need to:

1. Build a new string from the updated `day` value.
2. Render that string into a new surface.
3. Convert the surface into a new texture.
4. If the new texture was created successfully, update the destination size and replace the old texture.
5. Destroy the temporary surface.
6. Continue drawing the current `dayTexture` in the usual rendering loop.

This work belongs in the `SDLK_E` event handler because that is when the day changes. We do **not** need to recreate the text texture every frame.

### Converting the current day into text

We use the standard string utility:

```cpp
std::string dayText = "Day: " + std::to_string(day);
```

`std::to_string(day)` converts the integer to text. If `day` equals `2`, it produces `"2"`, and the combined string becomes `"Day: 2"`.

This needs:

```cpp
#include <string>
```

SDL_ttf's render function takes a C-style string in this call, so we use:

```cpp
dayText.c_str()
```

That gives the function a pointer to the characters in the `std::string`. The `std::string` remains the C++ string object; `c_str()` does not mean we must manually allocate or free another string.

One small consistency note: our original startup text was `"Day 1"`, while the update text became `"Day: 2"`. The game works, but the formatting differs. A later small polish task would be to use the same format for both, such as `"Day: 1"` at startup.

---

## 7. The confusing part: safely replacing `dayTexture`

This was the section you said you did not fully understand yet:

```cpp
if(newDayTexture != nullptr) {
    dayRect.w = static_cast<float>(newDaySurface->w);
    dayRect.h = static_cast<float>(newDaySurface->h);

    SDL_DestroyTexture(dayTexture);
    dayTexture = newDayTexture;
}
```

Let's go slowly. This code is only reached after we have attempted to create a new texture from `newDaySurface`.

### First: what does the `if` test mean?

```cpp
if(newDayTexture != nullptr) {
```

`SDL_CreateTextureFromSurface()` can fail. If it fails, `newDayTexture` will be `nullptr`.

The condition asks: **did we actually get a valid new texture?**

Only if the answer is yes should we replace the old one. If creation failed, the old `dayTexture` is still available, so the game can keep displaying the last successfully rendered day label.

### Next: update the rectangle's size

```cpp
dayRect.w = static_cast<float>(newDaySurface->w);
dayRect.h = static_cast<float>(newDaySurface->h);
```

`newDaySurface->w` and `newDaySurface->h` are the new text surface's width and height. The arrow operator `->` accesses a member through a pointer; here, `newDaySurface` points to the surface, and `w`/`h` are its dimensions.

We copy those values into `dayRect` so the destination rectangle matches the newly rendered text.

Why do that? Text can change width as its content changes. For example, `Day: 8` and `Day: 10` do not necessarily have the same width. If we kept the old rectangle width, SDL would draw the new texture using an outdated destination size. Updating the width and height lets the new text use its appropriate dimensions.

We do this **before destroying `newDaySurface`**, because this is where we still have access to its dimensions. Once we have copied the numbers into `dayRect`, the rectangle does not depend on the surface staying alive.

### Then: destroy the old texture

```cpp
SDL_DestroyTexture(dayTexture);
```

At this point, `dayTexture` is the texture from the previous day label. We no longer want to keep it after a valid replacement has been created.

Textures use resources, so repeatedly replacing a texture without destroying the previous one could leave old textures allocated and cause a resource leak.

The ordering matters: the new texture was created successfully **before** this line. We do not throw away the current working texture before knowing that we have a replacement.

### Finally: make `dayTexture` refer to the new texture

```cpp
dayTexture = newDayTexture;
```

Both variables are `SDL_Texture*` pointers. This assignment copies the pointer value, so after the assignment, the variable used by the rendering loop—`dayTexture`—points to the new texture.

The next time this runs:

```cpp
SDL_RenderTexture(renderingPointer, dayTexture, nullptr, &dayRect);
```

it draws the new text texture.

The assignment does **not** copy the pixels from one texture to another. The new texture already exists. We're updating which texture our main `dayTexture` variable refers to.

### The sequence in one picture

```text
A day changes
     |
     v
Render the new words into newDaySurface
     |
     v
Try to create newDayTexture
     |
     v
Was newDayTexture created?
     |
     +---- No: keep the old dayTexture
     |
     +---- Yes:
             1. Copy new surface dimensions into dayRect
             2. Destroy the old dayTexture
             3. Assign newDayTexture to dayTexture
     |
     v
Destroy the temporary newDaySurface
     |
     v
The rendering loop draws dayTexture, now the replacement
```

### Why not destroy the old texture first?

Suppose we destroyed `dayTexture` before trying to create the new one. If texture creation then failed, there would be no valid day texture left to draw.

The safer order is:

1. Create the replacement.
2. Check that it exists.
3. Only then destroy the old texture and switch the pointer.

This is a general resource-management pattern: **keep the working resource until its replacement is ready.**

### What about the surface?

After the `if` block, our code destroys the temporary surface:

```cpp
SDL_DestroySurface(newDaySurface);
```

This is outside the `if(newDayTexture != nullptr)` block but inside the check that `newDaySurface != nullptr`. That placement means the surface gets destroyed whether the texture conversion succeeded or failed, as long as a surface was created in the first place.

The texture has its own image data/resources, so destroying the source surface after creating the texture does not destroy the texture too. Each resource must be cleaned up using its own function.

### A mental model

Imagine you have a working printed sign that says **Day 1**.

1. Make a new sign that says **Day 2**.
2. Check that the new sign was made successfully.
3. Work out the new sign's dimensions and update the frame where it will be shown.
4. Once the replacement exists, discard the old sign.
5. Make your `dayTexture` variable refer to the new sign.

You do not throw away the old sign while the new one is still being made. That's the important idea behind the conditional block.

---

## 8. The whole dynamic-update process

Here is the process used when the `E` key advances the day. This is a conceptual outline of the code we built during the session, not a replacement for the actual code in `sdl3_window.cpp`.

```cpp
// The day has advanced.
day++;

// Build the updated label.
std::string dayText = "Day: " + std::to_string(day);

// Render the new string into a temporary surface.
SDL_Surface* newDaySurface = TTF_RenderText_Blended(
    font, dayText.c_str(), 0, textColor
);

if(newDaySurface != nullptr) {
    // Make a replacement texture from the new surface.
    SDL_Texture* newDayTexture = SDL_CreateTextureFromSurface(
        renderingPointer, newDaySurface
    );

    if(newDayTexture != nullptr) {
        // Match the destination rectangle to the new text dimensions.
        dayRect.w = static_cast<float>(newDaySurface->w);
        dayRect.h = static_cast<float>(newDaySurface->h);

        // Replace the old texture only after the new one exists.
        SDL_DestroyTexture(dayTexture);
        dayTexture = newDayTexture;
    }

    // The temporary surface is no longer needed.
    SDL_DestroySurface(newDaySurface);
}
```

The complete real code should also log or otherwise handle surface/texture creation failures, as in the version we used. The main lesson here is the order of the operations.

## 9. Resource cleanup at shutdown

By the end of this session, the program has resources for the font and the text texture in addition to its existing crop and terrain resources.

Make sure the final cleanup section accounts for them:

```cpp
SDL_DestroyTexture(dayTexture);
SDL_DestroyTexture(terrainSheet);
SDL_DestroyTexture(cropSheet);
TTF_CloseFont(font);
SDL_DestroyRenderer(renderingPointer);
SDL_DestroyWindow(window);
TTF_Quit();
SDL_Quit();
```

This shows the intended relationships and ordering at a high level:

- Destroy textures before destroying the renderer that created them.
- Close the loaded font before quitting SDL_ttf.
- Quit SDL_ttf before the final SDL shutdown.

Keep the actual cleanup section consistent with the resources that were successfully created. In error-handling branches, only destroy resources that exist at that point. If the code has already cleaned up a resource on an earlier path and exits, do not continue into the normal cleanup path afterward.

---

## 10. Common confusions and mistakes

### “I changed `day`. Why doesn't the text change?”

Because `dayTexture` is an image generated earlier. It does not continuously read the `day` variable. Render a new string and create a replacement texture when the day changes.

### “Why do we render the text to a surface and then make a texture?”

`TTF_RenderText_Blended()` provides rendered pixel data as an `SDL_Surface`. `SDL_CreateTextureFromSurface()` converts it into a texture the renderer can draw. These are separate steps and separate resources.

### “Why is `nullptr` the source rectangle in `SDL_RenderTexture()`?”

It means use the entire texture. For a terrain sprite sheet, we selected a smaller source rectangle; for the day label, the entire texture is the text we want to display.

### “Why pass `&dayRect`?”

`dayRect` is an actual rectangle object. `&dayRect` gives SDL the address of that object. By contrast, our source rectangle variable may already be a pointer, so we pass that pointer directly.

### “Why destroy the old texture?”

Because the new texture has replaced its role. The old texture is no longer needed, and leaving it allocated each time the day changes can accumulate unused resources.

### “Why do we destroy the surface after creating the texture?”

The surface was needed as the source for texture creation. After the texture exists and we have copied the surface's dimensions into `dayRect`, we do not need the surface anymore.

### “Why create the new texture only when `E` is pressed?”

That is when this label's content changes. Creating it only on change avoids repeatedly allocating and destroying textures every rendered frame.

---

## 11. Day 6 checkpoint checklist

Use this as a quick test of your understanding later. You do not need to recite the SDL API from memory; try to explain the relationships in your own words.

- [ ] I can explain why we added SDL3_ttf.
- [ ] I know why SDL_ttf has to be initialized and why the font must be closed.
- [ ] I can describe the path from a string to an `SDL_Surface`, then to an `SDL_Texture`, then to `SDL_RenderTexture()`.
- [ ] I can explain what each important argument in `TTF_RenderText_Blended()` represents.
- [ ] I can explain why `nullptr` is used for the source rectangle when drawing the text texture.
- [ ] I understand why `day` changing does not automatically change `dayTexture`.
- [ ] I can explain why `dayRect.w` and `dayRect.h` are updated from the new surface dimensions.
- [ ] I can explain why the new texture is created before the old one is destroyed.
- [ ] I can explain what `dayTexture = newDayTexture;` changes—and what it does not change.
- [ ] I know why `SDL_DestroySurface()` and `SDL_DestroyTexture()` are different cleanup operations.
- [ ] I know why we update the text texture when the day changes rather than recreating it every frame.

## 12. Small practice exercises

No need to do these immediately. They are here for when you revisit the notes.

1. **Change the label wording.** Make the initial label and updated label use the same format, for example `Day: 1` and `Day: 2`.
2. **Explain the replacement block aloud.** Without reading the explanation above, describe the purpose of each of these lines:

   ```cpp
   dayRect.w = static_cast<float>(newDaySurface->w);
   dayRect.h = static_cast<float>(newDaySurface->h);
   SDL_DestroyTexture(dayTexture);
   dayTexture = newDayTexture;
   ```

3. **Predict a failure case.** If `SDL_CreateTextureFromSurface()` fails, why is it better to keep the current `dayTexture`?
4. **Predict what happens if we forget to replace the texture.** If `day` becomes `2` but `dayTexture` still points to the original texture, what will the rendering loop display?

## Final takeaway

The biggest lesson from Day 6 is that drawing text is a small resource pipeline, not a direct instruction to “print these characters on the screen.” We use SDL_ttf to rasterize the string into a surface, convert that surface to a renderer texture, and draw that texture using the same SDL rendering function we already knew.

When the text changes, we create a replacement texture. We update its destination dimensions and switch to it only after creation succeeds, then clean up resources that are no longer needed.

That is the foundation for more in-game UI later: inventory labels, selected-tile information, messages, menu items, and other text can build on the same concepts.
