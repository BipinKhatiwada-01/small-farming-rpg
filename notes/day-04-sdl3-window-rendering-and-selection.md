# Day 4 — SDL3 Foundations: Window, Rendering, Farm Grid, and Tile Selection

> **Checkpoint:** The console farming RPG still builds independently. A separate SDL3 test program opens a graphical window, processes quit and mouse-button events, draws a 3×3 farm grid using the shared `Tile` and `Crop` types, shows a marker based on crop stage, and draws a black outline around the selected tile.
>
> **Next:** Use the selected coordinates to perform one gameplay action in the graphical interface, beginning with planting. Do not add unrelated systems yet.

## 1. Project state and file responsibilities

The project now has these relevant files:

```text
farming-rpg/
├── main.cpp               # Existing console game and console entry point
├── farm.hpp               # Shared Crop class and Tile struct
└── sdl3_window.cpp    # SDL3 experiment and graphical entry point
```

`main.cpp` and `sdl3_window.cpp` each have a `main()` function, so they are built as **separate executables**, never linked together into the same executable.

`farm.hpp` holds the shared type definitions. It starts with `#pragma once` and includes `<iostream>` because the current `Crop::Water()` method prints messages with `std::cout`.

The `Crop` and `Tile` types remain the source of truth for farm state:

- `Tile::hasCrop` says whether a crop is planted on a tile.
- `Tile::crop.GetStage()` reports the lifecycle stage of that crop.
- `Crop::Water()` and `Crop::DayCheck()` still own their existing rules.
- The SDL renderer reads this state to decide what to draw; it should not invent or own crop-growth rules.

The current graphical program uses a temporary test setup:

```cpp
Tile farm[3][3];
farm[1][1].hasCrop = true;
```

This makes the center tile occupied so the rendering can be tested before graphical planting exists.

## 2. Environment and build

Environment reported by the developer:

- GNU/Linux
- SDL3 development package installed: `SDL3-devel`
- `pkg-config --modversion sdl3` returned `3.4.18`
- C++ standard remains C++17; no standard change was needed for these milestones.

The existing console target is built independently:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o farm
```

The SDL target uses SDL3's `pkg-config` flags to find headers and libraries:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic \
    sdl3_window.cpp -o sdl_window \
    $(pkg-config --cflags --libs sdl3)
```

Run the graphical test with:

```bash
./sdl_window
```

The program remains attached to the terminal while running. The SDL window is a separate desktop window; closing it sends a quit event and lets the program clean up.

## 3. SDL lifecycle

### Initialization

```cpp
if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Video failed to initialize: "
              << SDL_GetError() << '\n';
    return 1;
}
```

SDL3's `SDL_Init()` returns a `bool`: `true` means success; `false` means failure. The negation operator (`!`) makes the error branch run when initialization failed. Call it once and check that call's return value.

### Window and renderer

```cpp
SDL_Window* window = SDL_CreateWindow(
    "Farming RPG - SDL3 Test", 800, 600, 0);
SDL_Renderer* renderingPointer = SDL_CreateRenderer(window, nullptr);
```

- `SDL_Window*` stores a pointer to the window managed by SDL.
- `SDL_Renderer*` stores a pointer to a 2D renderer associated with the window.
- `nullptr` asks SDL to choose a suitable renderer driver.
- Check both results against `nullptr` before using them.
- If renderer creation fails, destroy the already-created window and call `SDL_Quit()`.

Cleanup order is the reverse of creation dependencies:

```cpp
SDL_DestroyRenderer(renderingPointer);
SDL_DestroyWindow(window);
SDL_Quit();
```

Do not destroy a window when its creation failed and its pointer is `nullptr`.

## 4. The event loop

The persistent loop now responds to SDL events rather than exiting after a fixed two-second delay.

```cpp
bool running = true;

while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }

    // Draw a frame here.
    SDL_Delay(16);
}
```

`SDL_Event` is a local object that can hold one event. `SDL_PollEvent(&event)` receives the address of that object so SDL can fill it. The inner loop drains currently pending events. The outer loop keeps the application running until `running` becomes `false`.

`SDL_Delay(16)` is a temporary throttle while learning. It is not the mechanism that keeps the window alive, and it is not yet a proper frame-timing system.

## 5. Rendering a frame

The current rendering sequence is:

1. Select a background draw color.
2. Clear the renderer's target.
3. Draw the farm tiles.
4. Draw crop markers on occupied tiles.
5. Draw the selection border last, if appropriate.
6. Present the completed frame.

Core pattern:

```cpp
SDL_SetRenderDrawColor(renderingPointer, 40, 80, 55, 255);
SDL_RenderClear(renderingPointer);

// Draw the scene here.

SDL_RenderPresent(renderingPointer);
```

`SDL_SetRenderDrawColor(renderer, r, g, b, a)` changes the renderer's current draw color. It does not assign a color to a rectangle. The color active when a drawing function runs is what that drawing operation uses.

`SDL_RenderClear()` clears the target using the current draw color. Drawing operations then add shapes to the frame. `SDL_RenderPresent()` presents the completed frame. The scene is redrawn every frame.

### Rectangles

```cpp
SDL_FRect tileRect{x, y, 90.0f, 90.0f};
SDL_RenderFillRect(renderingPointer, &tileRect);
```

`SDL_FRect` stores the rectangle's top-left position (`x`, `y`) and dimensions (`w`, `h`) as floating-point values. `&tileRect` passes its address to SDL.

`SDL_RenderFillRect()` fills the rectangle. `SDL_RenderRect()` draws its outline without filling the interior.

## 6. Drawing the 3×3 farm grid

The grid uses these current layout constants (still written directly into the code at this checkpoint):

- Window: 800 × 600 pixels
- First tile's top-left corner: `(400, 180)`
- Tile width and height: `90 × 90` pixels
- Distance between tile starting positions: 100 pixels
- Gap between tiles: 10 pixels

The coordinates are calculated using nested loops:

```cpp
float x = 400 + col * 100;
float y = 180 + row * 100;
```

The column determines horizontal position; the row determines vertical position. `row` and `col` are logical farm indices, not pixel coordinates.

The tile's ground is drawn first:

- Empty tile: green grass color.
- Occupied tile: brown soil color.

If `farm[row][col].hasCrop` is true, the renderer reads `farm[row][col].crop.GetStage()` and draws a smaller `30 × 30` marker at `(x + 30, y + 30)`.

Why add 30? The marker is 30 pixels wide and the tile is 90 pixels wide, so centering leaves `(90 - 30) / 2 = 30` pixels on each side. The same applies vertically.

**Important draw-order rule:** draw the ground first and the crop marker afterward. If the full-size tile is drawn after the marker, it covers the marker completely.

Current marker mapping:

| Crop stage | Marker appearance |
|---|---|
| `Crop::Seed` | Tan |
| `Crop::Sprouted` | Bright green |
| `Crop::Matured` | Darker green |
| `Crop::Withered` | Gray-brown |

This is simple placeholder art intended to verify state-to-rendering behavior, not final art direction.

## 7. Mouse selection and coordinate conversion

The selected tile is stored before the main loop so the selection persists across frames:

```cpp
int selectedRow = -1;
int selectedCol = -1;
```

`-1` means no tile is selected. Valid farm indices are 0, 1, and 2.

Mouse selection handles a left-button press:

```cpp
event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
event.button.button == SDL_BUTTON_LEFT
```

The mouse event carries pixel coordinates. Convert them to positions relative to the top-left of the grid:

```cpp
int relativeX = static_cast<int>(event.button.x) - 400;
int relativeY = static_cast<int>(event.button.y) - 180;
```

First reject points outside the grid's 300 × 300 logical footprint:

```cpp
if (relativeX < 0 || relativeX >= 300 ||
    relativeY < 0 || relativeY >= 300) {
    continue;
}
```

Then reject the 10-pixel gaps between tiles. `%` is the remainder operator:

```cpp
if (relativeX % 100 >= 90 ||
    relativeY % 100 >= 90) {
    continue;
}
```

For a valid tile click, divide by the 100-pixel tile pitch:

```cpp
selectedCol = relativeX / 100;
selectedRow = relativeY / 100;
```

Integer division maps a pixel position inside each 100-pixel cell to the appropriate index. The modulo tests ensure that positions in the 10-pixel gaps are not treated as tile clicks.

### Drawing the selection

After drawing the ground and any crop marker, compare the tile being rendered against the stored selection:

```cpp
if (row == selectedRow && col == selectedCol) {
    SDL_SetRenderDrawColor(renderingPointer, 0, 0, 0, 255);
    SDL_RenderRect(renderingPointer, &tileRect);
}
```

The outline is black by design. Draw it last so it appears on top of the ground and marker.

The click mapping must use the same grid origin as the rendering code. If the grid's drawing origin changes, update the mouse offset too—or later, define shared layout constants so the two cannot drift apart.

## 8. Current checkpoint: what is and is not implemented

### Working

- SDL3 development environment installed and detected with `pkg-config`.
- SDL video initialization with an error check.
- Window and renderer creation, including failure checks.
- Event loop handles the SDL quit event.
- Renderer clears and presents frames.
- 3×3 tile grid is drawn with nested loops.
- Drawing reads `Tile::hasCrop` and `Crop::GetStage()`.
- A black outline tracks a clicked tile.
- Console and SDL executables share the `Crop` and `Tile` definitions through `farm.hpp`.
- Console build was rechecked after extracting the shared types.

### Temporary or postponed

- `farm[1][1].hasCrop = true` is hardcoded test data; no graphical planting exists yet.
- Mouse selection does not yet change gameplay.
- No graphical watering action or day-advance control exists.
- `Crop::Water()` prints directly to `std::cout`; graphical feedback will eventually need a presentation-neutral result/message design, but do not refactor it until needed for the next task.
- No harvesting, inventory, shops, saving/loading, textures, sprite sheets, or audio.
- `SDL_Delay(16)` is a temporary loop throttle.
- The SDL file is still named `sdl_window_test.cpp`; naming/reorganization can wait until the test becomes a more complete graphical program.

## 9. Next milestone

Use the existing selected row and column to perform one gameplay action from the SDL interface. Start with planting a seed in an empty selected tile. Preserve `Crop` as the owner of crop rules and `Tile::hasCrop` as the occupancy flag. Once planting is verified, remove the hardcoded center test crop.

## 10. Self-check questions

1. Why should `farm` be declared outside the main rendering loop?
2. Why do we destroy the renderer before destroying the window?
3. What does `SDL_PollEvent(&event)` do with the address of `event`?
4. Why must `SDL_RenderPresent()` come after drawing the frame?
5. Why does the grid's column affect `x` and row affect `y`?
6. Why do we draw the ground before drawing the crop marker?
7. Why is the marker offset `x + 30`, `y + 30`?
8. Why do we subtract `(400, 180)` from the mouse coordinates?
9. What do `% 100` checks do for clicks in the gaps?
10. Why should the renderer read `GetStage()` rather than directly modify crop lifecycle state?
11. What's the difference between the selected tile and a tile that actually contains a crop?
12. Why are the console and SDL programs built as separate executables?
