# Day 2 — Farm Tiles and Crop Integration

> **Checkpoint:** The current working program after introducing `Tile` and replacing the separate character grid and crop array with one `Tile` grid.
>
> **Status:** The program compiles and runs. The farm displays as a 3×3 grid, and planting at a valid empty coordinate marks that tile as occupied and displays `S` there.

## 1. Goal of this checkpoint

Connect the farm's tile occupancy to the crop object associated with that tile, while keeping each concept responsible for the right information.

The central design decision is to use one array of `Tile` objects instead of maintaining two separate arrays that must stay synchronized.

## 2. Current design

```text
Tile farm[3][3]
│
└── Each farm[row][col] is a Tile
    ├── hasCrop  : bool
    └── crop     : Crop
        ├── current           : Stage
        ├── wateringProgress  : int
        ├── grace             : int
        └── wateredToday      : bool
```

### `Crop` class

`Crop` is responsible for the lifecycle and rules of one crop.

- Public nested enum `Stage`: `Seed`, `Sprouted`, `Matured`, `Withered`.
- Private state: `current`, `wateringProgress`, `grace`, `wateredToday`.
- Public operations: `Water()` and `DayCheck()`.

The state is private so unrelated code cannot change the growth and watering variables directly. The crop's existing behavior has not yet been connected to menu actions or a farm-wide daily update.

### `Tile` struct

```cpp
struct Tile {
    bool hasCrop = false;
    Crop crop;
};
```

- `hasCrop` tells the game whether the tile is occupied.
- `crop` stores the crop state associated with that location.
- New tiles start with `hasCrop == false`.
- A tile contains a `Crop` object even while empty. That object existing in memory does **not** mean a crop has been planted in the game world.

This relationship is an example of **composition**: a `Tile` contains a `Crop` object.

## 3. How planting works

The program asks the player for a row and column, validates that both coordinates are in the range `0` to `2`, and checks the selected tile.

Planting is permitted only when `farm[r][c].hasCrop` is `false`. The planting operation initializes the crop stored in the selected tile and marks the tile as occupied. If the tile is already occupied, the program rejects the attempt.

Conceptually:

1. Check whether `farm[r][c]` is empty.
2. Reset `farm[r][c].crop` to a fresh default `Crop` state.
3. Set `farm[r][c].hasCrop` to `true`.
4. Print the planting-success message.

The crop should be initialized **before** setting `hasCrop` to `true`, so the tile is not marked occupied until its crop state has been initialized.

## 4. How the farm is displayed

The display reads the tile's occupancy flag:

- If `hasCrop` is `true`, print `S`.
- Otherwise, print `.`.
- Print a newline after each row.

An empty farm looks like:

```text
...
...
...
```

After planting at row `1`, column `2`, it looks like:

```text
...
..S
...
```

A `bool` printed directly through `std::cout` normally appears as `0` or `1`. That is why displaying `hasCrop` directly produced a numeric grid. The game display instead converts the state into symbols meaningful to the player.

## 5. Why the old two-array design was replaced

The earlier design used:

```cpp
char farm[3][3];
Crop crops[3][3];
```

That approach was workable, but the two arrays represented the same locations and had to remain synchronized. It also created confusion because every `Crop` object defaulted to the `Seed` stage, including objects at empty tiles.

The current design uses:

```cpp
Tile farm[3][3];
```

Now each tile holds both its occupancy information and its associated crop state. `hasCrop` is the source of truth for whether a crop currently occupies that tile; `crop.current` describes the growth stage of the crop object.

## 6. C++ concepts practised

### Member access (`.`)

For a selected tile:

- `farm[r][c].hasCrop` accesses its occupancy flag.
- `farm[r][c].crop` accesses the `Crop` object stored in it.

The expression `farm[r][c]` is now a `Tile` object, not a character. Therefore, `std::cout << farm[r][c]` does not work automatically: C++ has no built-in rule for how to print an entire custom `Tile` object. The program chooses specific data to display instead.

### Composition

A struct or class can contain an object of another type. Here, `Tile` contains a `Crop`.

### Encapsulation

`Crop` uses `private` data and `public` methods. Its implementation controls changes to its internal lifecycle state.

### Game state versus display

`hasCrop` is game state; `'.'` and `'S'` are display symbols derived from that state. Keeping these separate makes it easier to change how the game is rendered later, including when a graphical renderer is introduced.

## 7. Current scope: implemented versus not implemented

### Implemented and verified

- A 3×3 farm represented by `Tile farm[3][3]`.
- Empty tiles by default (`hasCrop == false`).
- Row and column validation for indices `0`, `1`, and `2`.
- Planting on an empty tile.
- Rejecting a planting attempt on an occupied tile.
- Correct symbolic grid display.
- The `Crop` lifecycle logic remains encapsulated in the `Crop` class.

### Not implemented yet

- A repeated gameplay/menu loop.
- Player-selected watering actions.
- Checking tile occupancy before watering.
- Calling `DayCheck()` once per planted crop at the end of an in-game day.
- Advancing the day and displaying crop stages.
- Harvesting mature crops.
- Clearing a withered crop and reusing its tile.
- SDL3 graphics, mouse input, or a graphical interface.

The current program performs a planting interaction and then exits; the next goal is to introduce a repeated gameplay loop.

## 8. Build and test

Build with the project's warning-enabled command:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o farm
```

Run:

```bash
./farm
```

Verified manual test:

1. Start the program: the farm displays as three rows of `...`.
2. Enter row `1` and column `2`.
3. The program reports that the seed was planted.
4. The farm displays `..S` on the second row.

The farm starts fresh on each program run; state is not saved between runs.

## 9. Self-check questions

1. Why does a default `Crop` object in `Stage::Seed` not prove that a crop has been planted?
2. What is the responsibility of `Tile`, and what is the responsibility of `Crop`?
3. Why does `farm[r][c]` no longer print directly through `std::cout`?
4. Why is `hasCrop` preferable to treating the crop's growth stage as the occupancy indicator?
5. What should the game check before calling `Water()` on a tile's crop?
6. Why should the crop state be initialized before `hasCrop` is set to `true`?

## 10. Next checkpoint

Build the gameplay loop around the existing program without rewriting the data model. Start with a repeating menu and actions for Plant, Water, End Day, and Quit. Add and test these actions incrementally. A watering action must reject empty tiles, and an end-of-day action must update every planted crop exactly once.
