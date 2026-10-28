# Small farming rpg

A small farming game built with **C++17**. The project has two frontends: a graphical version made with **SDL3** and a console version. Both use the same core farming types for crops, tiles, and inventory.

The project began as a way to learn C++ game logic and SDL3 fundamentals by building a manageable farming loop: plant and water crops, advance days, harvest produce, manage resources, and buy more seeds.

## Features

- Fixed overhead view of a **3×3 farm** in an **800×600** SDL3 window.
- Pixel-art terrain and crop sprites rendered with SDL3 and SDL3_image.
- Four crop stages: `Seed`, `Sprouted`, `Matured`, and `Withered`.
- Watering and day advancement to progress crop growth.
- Harvesting that succeeds only for mature crops.
- Inventory for seeds and harvested produce.
- Simple economy: buy seeds, sell produce, and track coins.
- Persistent graphical HUD showing the day, seeds, produce, and coins.
- Temporary in-game notifications for action results.
- Console frontend for the same basic farming loop without SDL rendering.

## Gameplay

The basic loop is:

**Plant → Water → Advance day → Grow → Harvest → Sell produce → Buy seeds → Repeat**

### Starting resources and prices

| Resource or action | Starting value / rule |
|---|---:|
| Seeds | 3 |
| Produce | 0 |
| Coins | 0 |
| Buy one seed | 3 coins |
| Sell one produce | 5 coins |

Planting consumes one seed only when planting succeeds. Selling removes one produce and awards coins only when produce is available. Buying a seed spends coins only when the player can afford it. Failed actions should leave the relevant resources unchanged.

### SDL3 controls

First, **left-click a farm tile** to select it. Then use the following keys:

| Key | Action |
|---|---|
| `P` | Plant a seed on an empty selected tile |
| `W` | Water the crop on the selected tile |
| `E` | Advance the day and update crop growth |
| `H` | Harvest a mature crop from the selected tile |
| `I` | Display inventory and coin balance in a notification |
| `S` | Sell one produce for 5 coins |
| `B` | Buy one seed for 3 coins, if affordable |

The HUD keeps the day and resource totals visible. A selected tile is outlined, and action feedback appears as a temporary notification.

### Console controls

Run the console version to access its numbered menu:

1. Plant a crop
2. Water a crop
3. End day
4. Harvest
5. Sell produce
6. Buy a seed
7. Inspect inventory
8. Quit

When prompted for a tile, enter its row and column. Both coordinates use zero-based indexing, from `0` to `2`.

## Requirements

### Graphical version

- C++17 compiler, such as `g++`
- GNU Make
- `pkg-config`
- SDL3 development libraries
- SDL3_image development library
- SDL3_ttf development library
- The project's crop and terrain assets
- A font available at the path configured in `src/sdl3_window.cpp`

The SDL3 source loads these assets using paths relative to the project root:

- `assets/vectoraith/crops.png`
- `assets/vectoraith/terrain_summer_expanded.png`

The font path is currently set in the source to `/usr/share/fonts/google-noto/NotoSans-Regular.ttf`. If that font is not installed at that location on your system, update the path to an available font.

## Build and run

Run build commands from the **project root**, so relative asset paths resolve correctly.

### SDL3 version

Build with the project's Makefile:

```bash
make
```

Run the graphical game:

```bash
./output/sdl3_window
```

To clean and rebuild:

```bash
make clean
make
```

The Makefile obtains SDL3, SDL3_image, and SDL3_ttf compiler and linker flags through `pkg-config`.

### Console version

The console and SDL3 source files each define their own `main()`, so **build them as separate executables**. The console entry point is `src/main.cpp`.

```bash
mkdir -p output
g++ -std=c++17 -Wall -Wextra -Wpedantic \
    src/main.cpp \
    -o output/console_farming_rpg
```

Run it with:

```bash
./output/console_farming_rpg
```

The console source includes `farm.hpp` and `inventory.hpp` from the `src/` directory.

## Project structure

```text
.
├── assets/
│   └── vectoraith/
│       ├── crops.png
│       ├── terrain_summer_expanded.png
│       └── README.md
├── notes/
│   ├── day-09-final.md
│   └── extras/
│       ├── day-01-farm-tiles-and-crop-struct.md
│       ├── day-02-farm-tiles-and-crop-integration.md
│       ├── day-03-console-farming-prototype.md
│       ├── day-04-sdl3-window-rendering-and-selection.md
│       ├── day-05-sdl3-plant-water-and-day-cycle.md
│       ├── day-06-sdl3-text-rendering-day-counter.md
│       ├── day-07-inGame-notifications.md
│       ├── day-08-fixes-and-build-workflow.md
│       └── day-04-sdl3-coordinate-formulas.md
├── output/                 # Build output (generated)
├── src/
│   ├── farm.hpp
│   ├── inventory.hpp
│   ├── main.cpp             # Console frontend
│   └── sdl3_window.cpp      # SDL3 frontend
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

The `output/` directory contains generated binaries and is not source code. The exact contents of this directory can vary depending on which targets have been built.

## How the code is organised

### `Crop` — one plant's state

`Crop` stores its growth stage and watering state. Its `Water()` method returns a `Crop::WaterResult`, allowing callers to distinguish successful watering, watering twice in one day, and crops that are already mature or withered. `DayCheck()` processes the crop when the day advances, using the project's watering-progress and grace-period rules.

### `Tile` — a farm tile and its crop

`Tile` stores a `hasCrop` flag and a `Crop` object. Its `Harvest()` method returns a `Tile::HarvestResult`: `Harvested`, `NoCrop`, `NotMature`, or `Withered`.

On a successful harvest, `Tile::Harvest()` marks the tile as empty and resets its contained crop. The console and SDL3 callers respond to the returned result rather than duplicating the tile-clearing logic.

### `Inventory` — player resources

`Inventory` stores seed and produce counts privately. Its public methods add, remove, and read these resources. Removal methods return `bool`, allowing callers to check whether a resource was actually removed before completing a transaction.

The coin balance is stored separately in each frontend's game state. The project keeps this simple and does not introduce a separate economy or player class.

### SDL3 frontend

`src/sdl3_window.cpp` handles window creation, events, tile selection, drawing, HUD updates, notifications, and cleanup. It uses SDL3_ttf to render text and SDL3_image to load the sprite sheets.

`UpdateTextTexture()` creates a text surface and texture, updates the destination size, replaces the old texture only after successfully creating the new one, and releases the temporary surface. The persistent HUD and temporary notifications reuse this helper.

### Console frontend

`src/main.cpp` provides a numbered menu and text feedback. It uses the same `Crop`, `Tile`, and `Inventory` types, while handling console input and output independently from SDL3.

## Notes and learning record

The `notes/` directory contains the detailed Day 09 final notes and earlier daily learning notes under `notes/extras/`. They record implementation details, design decisions, SDL3 rendering and input, C++ concepts, and the final verification plan.

## Testing checklist

Before considering a build complete, verify the following locally:

- [ ] The SDL3 version builds with `make` and launches from the project root.
- [ ] The console version builds and launches separately.
- [ ] The HUD starts at Day 1 with 3 seeds, 0 produce, and 0 coins.
- [ ] Planting on an empty tile consumes one seed.
- [ ] Planting on an occupied tile or with no seeds consumes no additional resources.
- [ ] Watering and day advancement progress crops according to the current crop rules.
- [ ] Immature and withered crops cannot be harvested.
- [ ] Harvesting a mature crop clears the tile and adds one produce.
- [ ] Selling with produce removes one produce and adds 5 coins.
- [ ] Selling with no produce does not award coins.
- [ ] Buying a seed costs 3 coins; an unaffordable purchase changes nothing.
- [ ] The HUD reflects resource changes, and temporary notifications still appear.

## Scope and known limitations

This is a small learning project, not a full farming simulation. It intentionally keeps a fixed overhead camera, a 3×3 farm, one generic produce type, fixed prices, and keyboard-driven actions. It does not currently include a moving player character, seasons, multiple crop species, save/load, crafting, NPCs, or a full shop interface.

The console and graphical frontends run as separate programs. They share the core `Crop`, `Tile`, and `Inventory` logic, but each maintains its own runtime state.
