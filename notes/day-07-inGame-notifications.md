# Day 7 — In-Game Notifications and Separating Game Logic from Presentation

Today we added the foundation of an in-game notification system to the existing SDL3 farming-game experiment.

The game can now show messages in its window when the player tries to plant or water crops. We also made the text-rendering steps reusable and changed the crop-watering logic so that it reports an outcome instead of deciding how that outcome must be printed.

The main ideas from today are:

- Store the message separately from the code that displays it.
- Reuse one function to turn different strings into text textures.
- Update a shared texture after a group of branches has chosen the message.
- Return a result from game logic so different interfaces can respond in their own way.
- Replace a texture safely without destroying the old one before the replacement exists.

This document records the work completed today, how the pieces fit together, and the remaining details to revisit.

---

## 1. What we wanted to improve

Before Day 7, the game had a day counter rendered in the window, but most gameplay feedback appeared in the terminal. For example, planting and watering could print messages such as:

- `Seed planted!`
- `That tile is already occupied !`
- `Select a tile first !`
- `You can't apply water there !`

The goal was to display this feedback inside the game window too.

We did **not** add a coordinate label for the selected tile. We decided that displaying coordinates to the player would not meaningfully improve the game at this stage.

We also decided not to implement automatic notification timeouts today. Currently, a notification stays visible until another notification replaces it. That behaviour is acceptable for this checkpoint and can be reconsidered later.

## 2. The notification state

We introduced a string to store the current message:

```cpp
std::string notificationText = "";
```

This string holds the message we want to display. For example:

```cpp
notificationText = "Seed planted!";
```

It is useful to keep the message separate from its visual representation.

- `notificationText` is the **content** of the message.
- `notificationTexture` is the **renderable image** containing that message.
- `notificationRect` describes where the text texture should appear and how large it should be drawn.

The texture and destination rectangle were declared separately from the day counter's texture and rectangle because both UI elements need to be visible at the same time.

The notification texture begins as `nullptr` because no notification texture has been created yet:

```cpp
SDL_Texture* notificationTexture = nullptr;
```

The notification rectangle was initially placed near the bottom-left of the 800 × 600 window:

```cpp
SDL_FRect notificationRect{
    24.0f,
    548.0f,
    0.0f,
    0.0f
};
```

The values mean:

- `x = 24`: 24 pixels from the left edge.
- `y = 548`: 548 pixels from the top edge.
- `w = 0` and `h = 0`: the dimensions are not known until text has been rendered. Our text-update function fills these in.

The exact placement can be adjusted later when the rest of the interface has taken shape.

## 3. The text-rendering pipeline, revisited

Day 6 introduced the pipeline for displaying text. Day 7 reused that pipeline in a general-purpose helper function.

The basic sequence is:

```text
std::string
    ↓
TTF_RenderText_Blended()
    ↓
SDL_Surface
    ↓
SDL_CreateTextureFromSurface()
    ↓
SDL_Texture
    ↓
SDL_RenderTexture()
    ↓
Text appears in the game window
```

### 3.1 A string is not a picture

A string such as `"Seed planted!"` is text data. It is not yet a set of pixels that our SDL renderer can draw as a texture.

### 3.2 `TTF_RenderText_Blended()` creates a surface

A simplified version of the call we use is:

```cpp
SDL_Surface* textSurface = TTF_RenderText_Blended(
    font, text.c_str(), 0, color
);
```

This asks SDL_ttf to render the characters using the selected font and color. The result is an `SDL_Surface` containing pixel data for the rendered text.

The arguments are:

- `font`: the font we loaded earlier.
- `text.c_str()`: a pointer to the characters in the C++ string, in null-terminated C-string form.
- `0`: let SDL_ttf determine the text length from the terminating null character.
- `color`: the text's foreground color.

A surface is a real image in memory. We call it an *intermediate* image in this process because we use it to create the texture that our renderer will draw.

### 3.3 `SDL_CreateTextureFromSurface()` creates a texture

```cpp
SDL_Texture* newTexture = SDL_CreateTextureFromSurface(
    renderer, textSurface
);
```

This converts the surface's pixel data into an SDL texture suitable for use by the renderer.

A surface and a texture are both image representations, but they serve different roles in this workflow:

- The surface is the rendered image data produced by SDL_ttf.
- The texture is the representation we pass to the renderer to draw.

Once the texture has been created successfully and we have taken any information we need from the surface, the surface can be destroyed.

### 3.4 `SDL_RenderTexture()` draws the texture

The notification is drawn inside the main rendering loop, before `SDL_RenderPresent()`:

```cpp
if(notificationTexture != nullptr) {
    SDL_RenderTexture(
        renderingPointer,
        notificationTexture,
        nullptr,
        &notificationRect
    );
}
```

Here:

- `renderingPointer` is the renderer.
- `notificationTexture` is the texture containing the text.
- `nullptr` means use the whole texture as the source.
- `&notificationRect` tells SDL where and how large to draw it.

We check for `nullptr` because the notification texture may not exist before the first message is created successfully.

The draw call belongs in the rendering loop because the scene is cleared and redrawn every frame. Creating or updating a texture does not automatically draw it into the window.

## 4. Why we created `UpdateTextTexture()`

Initially, each gameplay branch would have needed to repeat the surface creation, texture creation, dimension updates, and cleanup. That would quickly make the event handlers long and repetitive.

We made a reusable function instead. Its interface is approximately:

```cpp
bool UpdateTextTexture(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const std::string& text,
    SDL_Color color,
    SDL_Texture*& texture,
    SDL_FRect& destination
);
```

The function accepts:

- the renderer to create the texture for;
- the font;
- the string to render;
- the text color;
- a reference to the caller's texture pointer;
- a reference to the caller's destination rectangle.

It returns `true` if the update succeeds and `false` if surface or texture creation fails.

### 4.1 Why `SDL_Texture*& texture`?

`texture` is a pointer to an `SDL_Texture`. The `&` in the parameter type means the function receives a **reference to the caller's pointer**.

That lets the function update the actual pointer owned by the caller.

For example, the caller can pass `notificationTexture`. If the helper creates a replacement, this assignment inside the function:

```cpp
texture = newTexture;
```

updates the caller's `notificationTexture` pointer too. The next rendering frame then uses the replacement texture.

This is different from passing `SDL_Texture* texture` by value. Passing only a pointer by value would give the function a copy of the pointer; assigning a new address to that copy would not replace the caller's pointer.

### 4.2 Why `SDL_FRect& destination`?

We want to modify the caller's actual destination rectangle, not a local copy. Text strings have different lengths, so their rendered textures can have different dimensions.

The helper updates the rectangle using the rendered surface's dimensions:

```cpp
destination.w = static_cast<float>(textSurface->w);
destination.h = static_cast<float>(textSurface->h);
```

This makes the destination rectangle fit the new texture's width and height. The rectangle's `x` and `y` stay as they were, so the message stays in its chosen location.

## 5. The critical replacement sequence

The successful portion of `UpdateTextTexture()` follows this order:

```cpp
SDL_Texture* newTexture = SDL_CreateTextureFromSurface(
    renderer, textSurface
);

if(newTexture == nullptr) {
    SDL_DestroySurface(textSurface);
    return false;
}

destination.w = static_cast<float>(textSurface->w);
destination.h = static_cast<float>(textSurface->h);

if(texture != nullptr) {
    SDL_DestroyTexture(texture);
}

texture = newTexture;

SDL_DestroySurface(textSurface);
return true;
```

This example focuses on the sequence; retain the current function's error messages as appropriate.

Let's unpack why the order matters.

### Step 1: Create the new texture first

```cpp
SDL_Texture* newTexture = SDL_CreateTextureFromSurface(
    renderer, textSurface
);
```

The old texture is still available at this point. We try to create a replacement without giving up the current message first.

### Step 2: If creation fails, preserve the old texture

```cpp
if(newTexture == nullptr) {
    SDL_DestroySurface(textSurface);
    return false;
}
```

If texture creation fails, the helper frees the temporary surface and returns `false`. It does **not** destroy the existing texture or overwrite its pointer. The last successfully created notification can therefore remain visible.

### Step 3: Update the new size

```cpp
destination.w = static_cast<float>(textSurface->w);
destination.h = static_cast<float>(textSurface->h);
```

We use the surface's dimensions before destroying it. A longer message, such as `"That tile is already occupied !"`, can be wider than `"Seed planted!"`.

### Step 4: Destroy the previous texture if there is one

```cpp
if(texture != nullptr) {
    SDL_DestroyTexture(texture);
}
```

The first notification may have no previous texture because the pointer starts as `nullptr`. On later messages, it normally points to the previous notification texture.

The condition prevents us from trying to destroy a texture that does not exist.

### Step 5: Make the caller use the replacement

```cpp
texture = newTexture;
```

The order of assignment matters. We want the caller's texture pointer to refer to the new texture.

The opposite assignment, `newTexture = texture`, would overwrite the local pointer to the replacement with the old pointer instead. It would not update the caller's pointer and could lose access to the new texture.

### Step 6: Release the temporary surface

```cpp
SDL_DestroySurface(textSurface);
return true;
```

Once the texture is created and we have read the surface dimensions, we no longer need the surface for this update. The texture remains available for rendering.

**The big idea:** create the replacement successfully first; only then release the previous texture and update the pointer that the rendering loop uses.

## 6. Wiring the notification into gameplay

### 6.1 Planting (`P`)

The planting handler now chooses an appropriate notification for the possible outcomes:

- no tile selected;
- selected tile already contains a crop;
- planting succeeds.

Each branch assigns `notificationText`. We then call `UpdateTextTexture()` once after the complete `if / else if / else` chain.

The structure is conceptually:

```cpp
if(noTileSelected) {
    notificationText = "Select a tile first !";
}
else if(tileIsOccupied) {
    notificationText = "That tile is already occupied !";
}
else {
    farm[selectedRow][selectedCol].hasCrop = true;
    notificationText = "Seed planted!";
}

UpdateTextTexture(
    renderingPointer, font, notificationText, textColor,
    notificationTexture, notificationRect
);
```

The names `noTileSelected` and `tileIsOccupied` above are explanatory placeholders, not actual variables from the project.

Why put the call after the branches? Each branch decides **what the message should say**; the shared call handles **how that message gets rendered**. This removes repeated code and makes the handler easier to maintain.

The terminal messages remain in the SDL3 version for now. We chose to keep them as useful development feedback alongside the in-game notifications.

### 6.2 Watering (`W`)

The watering handler uses the same shared-call idea for the validation cases:

- no tile selected;
- selected tile has no crop;
- selected tile contains a crop and the watering logic must determine the result.

Once the chain has selected `notificationText`, a single `UpdateTextTexture()` call after the chain refreshes the notification.

This placement matters because a call placed inside the last `else` would run only when a crop exists. It would not refresh the message for the earlier validation failures.

## 7. Making `Crop::Water()` report an outcome

We found a design problem: `Crop::Water()` both applied watering rules and printed terminal messages. That made the crop model responsible for deciding how its result should be presented.

We introduced a result enum inside `Crop`:

```cpp
enum class WaterResult {
    Watered,
    AlreadyWatered,
    CropFinished
};
```

The `Water()` method's return type changed from `void` to `WaterResult`. It now reports one of three outcomes:

- `WaterResult::Watered`: the crop was watered successfully;
- `WaterResult::AlreadyWatered`: it had already been watered today;
- `WaterResult::CropFinished`: it is mature or withered and cannot be watered.

The watering rules themselves remain in `Crop::Water()`. The method still sets `wateredToday = true` on a successful watering. We removed its `std::cout` messages, leaving the method to perform the game logic and return its result.

### 7.1 Why use an enum instead of only `bool`?

A boolean result could distinguish success from failure, but it would not say which failure happened. The enum tells the caller the specific outcome, which allows it to select the right message.

### 7.2 Both callers handle the result

There were two uses of `.Water()` in the project:

1. The SDL3 event handler.
2. The older console game's watering handler.

Both callers needed updating before removing the printing from `Crop::Water()`.

The SDL3 caller stores the returned value and maps it to an in-game notification, such as `"Crop watered !"` or `"You have already watered this crop !"`.

The console caller stores the same result and prints the appropriate terminal message using its own conditional statements.

This preserves console feedback while allowing the SDL3 version to use the notification UI.

### 7.3 The responsibility split

After this change:

- `Crop::Water()` decides whether watering succeeds and returns the outcome.
- The console caller decides what to print in the terminal.
- The SDL3 caller decides what message to show in the game window.

This is a simple example of separating **game logic** from **presentation**. The crop does not need to know whether it is being used by a console interface or a graphical interface.

## 8. What we deliberately kept

We chose to keep `std::cout` notifications in the SDL3 event handlers as well as the in-game notifications. The terminal output remains useful while developing and debugging, so we did not remove it merely for the sake of making the terminal quiet.

We also did not add notification timers. A notification currently remains on screen until another message replaces it. That is the current behaviour, not an accidental claim that timed notifications are complete.

## 9. Common mistakes to remember

### Drawing text is separate from creating its texture

Updating `notificationText` only changes a C++ string. It does not update an existing texture automatically. We have to render the new string and create a replacement texture.

### A new texture needs a pointer update

Creating `newTexture` does not change `notificationTexture` by itself. The helper must assign the new pointer to the referenced caller pointer:

```cpp
texture = newTexture;
```

### Don't destroy the old texture before creating its replacement

If creation fails, preserving the old texture prevents the notification from losing its last working image. Destroy the old texture only after confirming that the replacement was created successfully.

### Use the surface before destroying it

The surface provides the rendered width and height. Read those dimensions before calling `SDL_DestroySurface(textSurface)`.

### Shared calls must be outside the entire branch chain

If a message needs to be updated for every outcome, put the shared `UpdateTextTexture()` call after the complete `if / else if / else` chain, not inside only the final branch.

### Remember resource cleanup

Since `notificationTexture` is now a resource, ensure it is destroyed before the renderer is destroyed during shutdown:

```cpp
SDL_DestroyTexture(notificationTexture);
```

Check the existing cleanup section before adding this line, so it is not added twice. Texture destruction must happen while the renderer still exists.

## 10. Day 7 summary

By the end of Day 7, we had:

- Added `notificationText`, `notificationTexture`, and `notificationRect`.
- Created a reusable `UpdateTextTexture()` helper.
- Learned how that helper generates a new text surface and texture, updates dimensions, replaces the old texture safely, and releases the temporary surface.
- Displayed notifications inside the game window and retained terminal output for development.
- Removed repeated texture-update calls from the planting and watering branch chains by using shared calls after the branches.
- Added `Crop::WaterResult` with three possible outcomes.
- Changed `Crop::Water()` to return a result instead of printing its own messages.
- Updated both the console and SDL3 callers to handle that result themselves.

The most important design lesson is that the crop should decide **what happened**, while the caller decides **how to communicate it**. The most important SDL lesson is that dynamic text must be rendered into a new texture when its contents change.

## 11. Follow-up items for a future session

These were not the focus of today's work and should be handled deliberately later:

- Decide whether notifications should disappear after a period of time or remain until replaced.
- Decide when terminal output should remain useful in the SDL3 version.
- Check that `UpdateTextTexture()`'s boolean return value is handled appropriately at its call sites, especially if rendering or texture creation fails.
- Confirm that `notificationTexture` is included in shutdown cleanup before the renderer is destroyed.
- Continue looking for repeated UI-update logic that can be shared without making the code harder to understand.

Don't tackle all these items at once. They are a checklist for future work, not a demand to expand the current feature immediately.
