# Day 2 — Farm Tiles, Crop Integration, and Gameplay Loop

> **Checkpoint:** The current working program after integrating `Tile` and `Crop`, and adding the first persistent gameplay loop.
>
> **Status:** The user reports that the program builds and the manual tests for planting, watering, ending the day, and quitting work. The farm persists across menu actions during one run. Crop stages are not yet shown in the farm display.

## 1. Goal of this checkpoint

Move from a one-action planting program to a small, menu-driven farming game while keeping the farm's data model coherent.

The key design decisions are:

- One `Tile` object represents each farm location.
- Each tile tracks whether a crop occupies it.
- Each tile contains the `Crop` object associated with that location.
- The game loop repeats actions without recreating the farm.
- End Day processes each planted crop once before advancing the in-game day.

## 2. Current design

```text
Tile farm[3][3]
│
└── Each farm[row][col] is a Tile
    ├── hasCrop : bool
    └── crop    : Crop
        ├── current           : Stage
        ├── wateringProgress  : int
        ├── grace             : int
        └── wateredToday      : bool
```

### `Crop` class

`Crop` owns the lifecycle rules for an individual crop.

- Public nested enum `Stage`: `Seed`, `Sprouted`, `Matured`, `Withered`.
- Private state: `current`, `wateringProgress`, `grace`, and `wateredToday`.
- Public methods: `Water()` and `DayCheck()`.
- `Water()` rejects matured and withered crops and prevents watering more than once before a daily update.
- `DayCheck()` advances growth progress after watering, handles the grace counter, withers a crop when grace reaches zero, and resets `wateredToday`.
- Matured and withered are terminal stages in the current design.

The state is private so `main()` cannot arbitrarily edit the crop's stage and counters. Later, if the game needs to display the crop's stage, we can add a public read-only accessor instead of exposing the state directly.

### `Tile` struct

```cpp
struct Tile {
    bool hasCrop = false;
    Crop crop;
};
```

- `hasCrop` tells the game whether a crop occupies this tile.
- `crop` stores the crop state associated with this location.
- New tiles start with `hasCrop == false`.
- A tile contains a `Crop` object even while empty. That object existing in memory does **not** mean a crop exists in the game world.

This relationship is an example of **composition**: a `Tile` contains a `Crop` object.

## 3. Why the old two-array design was replaced

The earlier design used:

```cpp
char farm[3][3];
Crop crops[3][3];
```

This approach was workable, but the arrays represented the same locations and had to remain synchronized. It also created confusion because every `Crop` object defaulted to the `Seed` stage, including objects at empty tiles.

The current design uses:

```cpp
Tile farm[3][3];
```

Now each tile holds both its occupancy information and its associated crop state. `hasCrop` is the source of truth for whether a crop occupies the tile; the crop's private `current` member represents its lifecycle stage.

## 4. Planting and displaying the farm

The program asks for row and column coordinates, validates that both values are in the range `0` to `2`, and checks the selected tile.

Planting succeeds only when `farm[r][c].hasCrop` is `false`. The current planting sequence is:

1. Create a fresh local `Crop` object.
2. Assign it to `farm[r][c].crop`.
3. Set `farm[r][c].hasCrop` to `true`.
4. Print the success message.

Initializing the crop before setting `hasCrop` to `true` ensures the tile's crop is ready before the game marks the tile as occupied.

The farm display derives a symbol from occupancy:

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

A `bool` printed directly through `std::cout` normally appears as `0` or `1`. The game display instead converts the Boolean state into symbols meaningful to the player.

**Current limitation:** the display only shows whether a tile is occupied. Sprouted, matured, and withered crops still appear as `S` until we add a safe way to read and display their stages.

## 5. The gameplay loop

The `main()` function creates persistent game state before the loop:

```cpp
int day = 1;
Tile farm[3][3];
bool running = true;
```

The loop repeats while `running` is true. Each iteration displays the current day and farm, shows the menu, reads a choice, and performs an action.

Current menu actions:

1. Plant a crop.
2. Water a crop.
3. End day.
4. Quit.

The farm array is declared **outside** the `while` loop. This is essential: the loop repeats actions, not the creation of a new farm. If the array were declared and initialized on each iteration, game state would be reset repeatedly.

## 6. Watering a crop

The Water action:

1. Reads and validates the row and column.
2. Checks `farm[r][c].hasCrop`.
3. Rejects an empty tile with a helpful message.
4. If occupied, calls the selected crop's method:

```cpp
farm[r][c].crop.Water();
```

This expression selects the tile, accesses its `crop` member, and calls the crop's public `Water()` method.

`main()` does not duplicate the crop's watering rules. The `Crop` class decides whether watering succeeds, has already happened today, or is forbidden because the crop is matured or withered.

The success message is currently printed by `Crop::Water()`. Avoid printing another unconditional success message in `main()`, because the method may reject the watering attempt.

## 7. Ending the day

The End Day action uses nested loops to visit all nine farm coordinates. For each occupied tile, it calls `DayCheck()` exactly once:

```cpp
if (farm[row][col].hasCrop == true) {
    farm[row][col].crop.DayCheck();
}
```

After both loops finish, the program increments `day`.

The order matters:

1. Visit the farm.
2. Update each planted crop once for the day that just ended.
3. Increment the day.
4. Return to the menu, where the updated day and farm are displayed.

Empty tiles must be skipped even though each `Tile` contains a default-constructed `Crop` object. The occupancy flag tells us whether the crop object should participate in the game simulation.

## 8. C++ concepts practised

### Member access (`.`)

- `farm[r][c].hasCrop` accesses the occupancy flag of a selected tile.
- `farm[r][c].crop` accesses the `Crop` object stored in that tile.
- `farm[r][c].crop.Water()` calls a method on that crop.

### Composition

One type can contain an object of another type. `Tile` contains a `Crop`, allowing each farm location to keep its crop state together with its occupancy state.

### Encapsulation

`Crop` keeps lifecycle variables private and exposes public operations. Other parts of the game ask the crop to perform behavior instead of directly changing its internals.

### Persistent state versus repeated behavior

The farm and day are created before the game loop, so they persist. The menu and selected action are processed repeatedly inside the loop.

### Conditional iteration

Nested loops visit every tile; `hasCrop` determines which tiles should receive a daily update.

### Game state versus display

The Boolean `hasCrop` is game state. The `S` and `.` characters are display symbols derived from it. This separation will also be useful when we introduce graphical rendering.

## 9. Implemented and verified

The user reports that the current program builds and the following behavior works:

- A 3×3 farm starts empty.
- Row and column validation rejects numeric coordinates outside `0`, `1`, and `2`.
- Planting on an empty tile succeeds.
- Planting on an occupied tile is rejected.
- The farm retains its state across repeated menu actions during one run.
- The Water action rejects empty tiles.
- A planted crop accepts one watering per day and rejects a second watering before End Day.
- End Day calls `DayCheck()` for every planted crop once, skips empty tiles, and increments the day afterward.
- After End Day, the crop can be watered on the new day.
- Quit exits the game loop.

Build command:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o farm
```

Run command:

```bash
./farm
```

### Known limitations

- Numeric range validation is implemented, but non-numeric input (for example, entering `abc` where an integer is expected) is not yet handled robustly.
- Crop stages are private and not yet displayed in the farm grid.
- Harvesting matured crops is not implemented.
- Withered crops remain on their tiles; clearing them and reusing the tile is not implemented.
- The game state is not saved between program runs.
- No SDL3 window or graphical input is implemented yet.

## 10. Self-check questions

1. Why does a default `Crop` object in `Stage::Seed` not prove that a crop has been planted?
2. What is the responsibility of `Tile`, and what is the responsibility of `Crop`?
3. Why does `farm[r][c]` no longer print directly through `std::cout`?
4. Why must we check `hasCrop` before calling `Water()` or `DayCheck()`?
5. Why should the crop be initialized before `hasCrop` becomes `true`?
6. Why are `farm` and `day` declared outside the game loop?
7. Why is `day++` placed after the loop that updates the crops?
8. Why should `main()` avoid printing a success message after every call to `Water()`?

## 11. Next checkpoint

The first playable farming cycle now exists: **Plant → Water → End Day → repeat**, with Quit available from the menu.

A useful next feature is to make crop stages visible in the display. Since `Crop` state is private, first introduce a public read-only way to inspect the stage, then use it to display different symbols for seeds, sprouts, mature crops, and withered crops. After that, implement harvesting and clearing withered crops according to the planned rules.
