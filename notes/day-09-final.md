# Day 09 — Final Day: Completing the Small Farming RPG

> **Project:** Small Farming RPG  
> **Language:** C++17  
> **Platform:** GNU/Linux  
> **Libraries:** SDL3, SDL3_image, SDL3_ttf  
> **Main graphical source:** `src/sdl3_window.cpp`  
> **Game rules:** `src/farm.hpp`  
> **Player resources:** `src/inventory.hpp`  
> **Build:** existing Makefile (`make`, `make clean`)  
> **Final scope:** fixed overhead farm, 3×3 grid, basic farming economy, persistent HUD

---

## 1. Day 09 objectives

The goal of the final day was to turn the mechanics built throughout the project into a small, coherent game, then stop adding features and prepare the project for closure.

The scope was deliberately kept small:

- Retain the fixed overhead view, the 800×600 window, and the 3×3 farm.
- Complete the resource loop: seeds → planting → watering and growth → harvesting → produce → selling → coins → buying seeds.
- Make resources visible without requiring the player to repeatedly request an inventory notification.
- Preserve the existing `Crop`, `Tile`, and `Inventory` responsibilities instead of introducing a large manager or UI framework.
- Keep the console version available as a second interface over the same farming rules.
- Finish with a local build and a focused gameplay test rather than adding more features.

The project is a **small fixed-view farm-management game**, not a free-roaming Stardew Valley clone. A moving player character, camera, collision system, seasons, multiple crop species, and a larger world were intentionally deferred.

## 2. Final feature overview

### Farming actions

| Action | SDL3 control | What it does |
|---|---|---|
| Select a tile | Left mouse click | Selects a farm tile within the 3×3 grid. |
| Plant | `P` | Plants on an empty tile if the player has a seed. |
| Water | `W` | Requests watering on the selected crop. |
| Advance day | `E` | Calls `DayCheck()` for crops and increments the day. |
| Harvest | `H` | Attempts to harvest the selected tile. |
| Inspect inventory | `I` | Shows a temporary resource summary notification. |
| Sell produce | `S` | Removes one produce and awards 5 coins if produce exists. |
| Buy seed | `B` | Spends 3 coins to add one seed if the player can afford it. |

The graphical version also has terminal output retained during development, so actions can be observed both in the game window and in the terminal.

### Starting resources and economy

The `Inventory` object starts with:

- **3 seeds**
- **0 produce**

The graphical and console programs each track `money` separately, starting at **0 coins**.

The prototype economy uses two fixed prices:

- One seed costs **3 coins**.
- One produce sells for **5 coins**.

The values are deliberately simple constants, not a balancing system. Selling one produce yields enough money to buy one seed, with two coins left over.

## 3. Architecture: who owns what?

The most important design outcome is that the game rules are not owned by the SDL event handlers. The interfaces ask the game model to perform operations and then display the results.

### `Crop` — one plant's state

`Crop` owns the state of an individual plant:

- `current`: the current growth stage.
- `wateringProgress`: accumulated successful watering days used by the growth logic.
- `grace`: tracks the crop's tolerance for days without watering.
- `wateredToday`: prevents multiple successful watering actions during the same in-game day.

Its public operations include:

- `Water()` — returns a `Crop::WaterResult` rather than printing messages.
- `DayCheck()` — processes the crop for the end of a day.
- `GetStage() const` — reads the current growth stage.

The stages are `Seed`, `Sprouted`, `Matured`, and `Withered`.

`Water()` can return:

- `Watered`
- `AlreadyWatered`
- `CropFinished`

The last result is returned when a crop is already `Matured` or `Withered` and cannot be watered further.

### `Tile` — crop occupancy and harvesting

A `Tile` contains:

- `hasCrop`, which states whether a crop occupies the tile.
- `crop`, the `Crop` object itself.
- `Tile::HarvestResult`, the outcomes of an attempted harvest.
- `Harvest()`, which implements the harvest rules.

The harvest outcomes are:

- `Harvested`
- `NoCrop`
- `NotMature`
- `Withered`

The current `Harvest()` operation follows this sequence:

1. If `hasCrop` is false, return `NoCrop`.
2. If the crop is `Seed` or `Sprouted`, return `NotMature`.
3. If the crop is `Withered`, return `Withered`.
4. Otherwise, clear the occupancy flag, reset the contained crop with `crop = Crop{}`, and return `Harvested`.

A successful harvest resets both the tile's occupancy and the old crop's internal state. The reset matters because the `Crop` object remains a member of `Tile` even after the tile becomes empty. If the next planting only re-enabled `hasCrop`, the old object's matured state could accidentally be reused.

`HarvestResult` is a scoped enumeration declared inside `Tile`, so callers refer to values such as `Tile::HarvestResult::Harvested`. This makes it clear which operation the result belongs to and prevents implicit conversions to integers.

### `Inventory` — the player's resource counts

`Inventory` lives in `inventory.hpp`. It privately stores `produceCount` and `seedCount`.

Its public methods are:

| Method | Responsibility |
|---|---|
| `AddProduce()` | Adds one produce. |
| `GetProduceCount() const` | Reads the produce count without modifying it. |
| `RemoveProduce()` | Removes one produce and returns `true`; returns `false` if none is available. |
| `AddSeed()` | Adds one seed. |
| `GetSeedCount() const` | Reads the seed count without modifying it. |
| `RemoveSeed()` | Removes one seed and returns `true`; returns `false` if none is available. |

The getters are `const` member functions because they inspect the object but do not modify it. The removal methods return `bool`, allowing callers to tell whether removal actually happened.

Coins remain a separate `int money` in each program's main game state. For this project's current scale, that is sufficient; a dedicated economy or player class was not necessary.

## 4. The resource loop and transaction rules

The full gameplay loop is:

1. Start with three seeds.
2. Select an empty tile and plant; consume one seed only if planting succeeds.
3. Water the crop and advance days until it grows.
4. Harvest only when the crop is mature; successful harvesting adds one produce to inventory.
5. Sell one produce; only a successful removal awards 5 coins.
6. Buy a seed; only spend 3 coins if the player has enough.
7. Plant again and repeat.

### Why action ordering matters

The central rule across buying, selling, and planting is:

> **Only change the player's resources after validating that the action can succeed.**

For planting, the selected coordinates must be valid, the tile must be empty, and a seed must be available. Occupied tiles must not consume seeds.

For selling, `RemoveProduce()` must succeed before `money` increases. This prevents a player from repeatedly selling with an empty inventory to create unlimited coins.

For buying, the program checks `money >= SEED_PRICE` before subtracting the price and calling `AddSeed()`.

For harvesting, the caller adds produce only when `Tile::Harvest()` returns `Harvested`. The graphical and console interfaces do not manually clear the tile after the method has already done so.

These checks are small examples of preserving game-state invariants: failed operations leave the relevant resources unchanged.

## 5. SDL3 input and selection flow

The SDL3 program uses two kinds of input:

- Mouse input chooses a tile.
- Keyboard input invokes an action on the selected tile or on the player's resources.

### Tile selection

`selectedRow` and `selectedCol` begin at `-1`, indicating that there is no selection yet.

When the left mouse button is pressed:

1. The mouse position is converted into coordinates relative to the farm origin (`GRID_X`, `GRID_Y`).
2. The code checks whether those relative coordinates fall inside the farm's pixel bounds.
3. It calculates the column and row by dividing the relative position by `TILE_SIZE`.

Before a tile-specific key handler indexes `farm[selectedRow][selectedCol]`, it checks that the row and column are within `[0, FARM_SIZE)`. This avoids reading outside the farm array when nothing has been selected.

### Key-repeat prevention

The key handlers include `!event.key.repeat`. This means the action is processed on an initial key-down event, not repeatedly while the key is held. This matters for actions such as buying, selling, planting, and advancing the day, where repeated input could unintentionally perform several transactions.

### Shared rules, different presentation

SDL3 handles input, notifications, and rendering. `Crop`, `Tile`, and `Inventory` hold the rules and resource state. The console interface calls the same core operations but reports outcomes with `std::cout` instead of rendered text.

Both programs have their own `main()` and should be built as separate executables. They can share `farm.hpp` and `inventory.hpp`; they should not be linked together into one program because that would combine two `main()` definitions.

## 6. SDL3 rendering and visual structure

The game keeps its existing 800×600 window and 3×3 farm. The farm uses 96-pixel tiles, beginning at `(GRID_X, GRID_Y) = (256, 156)`. Its total footprint is 288×288 pixels and is centred around the window's middle.

The background is tiled with 48×48 terrain pieces. The farm soil is drawn over the grass background, and crop sprites are drawn over the appropriate farm tile.

### Texture assets

The graphical program loads:

- `./assets/vectoraith/crops.png` — crop sprite sheet.
- `./assets/vectoraith/terrain_summer_expanded.png` — terrain tiles.

Both textures use `SDL_SCALEMODE_NEAREST` so pixel-art edges remain sharp when sprites are scaled up.

Crop stages select different source rectangles from the crop sheet. The program scales the selected sprite by `spriteScale` and positions it horizontally centred, with its bottom aligned near the bottom of the tile. The selected tile is outlined on top of its ground and crop artwork.

### Persistent HUD

Day 09's main presentation improvement was a persistent HUD showing:

- Day
- Seeds
- Produce
- Coins

Previously, the day and resource information was split between the day text and short-lived notifications. The HUD now keeps the most important resource values visible throughout play, so the player does not have to press `I` to check the current totals.

The implementation adds `UpdateHudTexture()`. It builds a string from `day`, the inventory getters, and `money`, then delegates texture creation to `UpdateTextTexture()`.

A `hudNeedsUpdate` flag records when a relevant value has changed. After processing events, the main loop refreshes the HUD texture when needed. Successful planting, harvesting, selling, buying seeds, and advancing the day mark the HUD as needing an update. Failed transactions do not alter the resource counts, so they do not need to refresh the resource HUD.

The HUD is drawn on a dark translucent panel. The blend mode is configured with `SDL_SetRenderDrawBlendMode(..., SDL_BLENDMODE_BLEND)` so alpha is respected by rendered panels.

### Temporary notifications

Notifications still use `notificationTexture`, `notificationRect`, `notificationText`, and `notificationStartTime`.

After an action, the program sets `notificationText` and calls `UpdateTextTexture()`. The start time is reset only if the text texture update succeeds. During rendering, the notification is drawn only while:

`SDL_GetTicks() - notificationStartTime < 1500`

This makes the notification temporary while the resource HUD remains persistent.

### `UpdateTextTexture()` and resource management

`UpdateTextTexture()` is reusable for both notifications and the HUD. It:

1. Calls `TTF_RenderText_Blended()` to create an `SDL_Surface` for the requested text.
2. Returns `false` and reports an error if surface creation fails.
3. Calls `SDL_CreateTextureFromSurface()` to make a texture.
4. If texture creation fails, destroys the temporary surface and returns `false`.
5. Updates the destination width and height based on the text surface dimensions.
6. Destroys the previous texture if one exists, but only after a new texture was created successfully.
7. Stores the new texture, destroys the temporary surface, and returns `true`.

The important resource-management detail is that a failed attempt to create the new texture does not destroy the old texture first. The old display can therefore remain available if text creation fails.

## 7. Console version

The console source uses a menu with the following choices:

1. Plant a crop.
2. Water a crop.
3. End day.
4. Harvest.
5. Sell produce.
6. Buy a seed.
7. Inspect inventory.
8. Quit.

At the top of each menu cycle, it prints the day, seed count, produce count, coin balance, and a visual 3×3 representation using emoji for empty soil and crop stages.

`GetCoordinates()` validates row and column input before the farm array is accessed. For menu and coordinate input, the code clears `std::cin` after failed numeric input and discards the rest of the line using `std::numeric_limits<std::streamsize>::max()`. This avoids a malformed input remaining in the stream and causing the next prompt to fail repeatedly.

The console uses the same transaction rules as SDL3. It adds produce only for a successful harvest, consumes seeds only when planting can proceed, awards money only after produce removal succeeds, and buys a seed only after checking the balance.

The two interfaces have separate `main()` functions but share the same crop/tile and inventory model. This is useful for learning because the gameplay rules can be exercised through a simple console interface independently of graphics.

## 8. SDL initialization and cleanup

The SDL program initializes the video subsystem, creates a window and renderer, loads the image textures, initializes SDL_ttf, and opens a font. It checks critical setup operations for failure and reports errors through `SDL_GetError()`.

During shutdown, it destroys the notification, terrain, crop, and HUD textures; destroys the renderer and window; closes the font; and shuts down SDL_ttf and SDL. The program also has early cleanup paths for failures during initialization.

The font currently uses this Linux path:

`/usr/share/fonts/google-noto/NotoSans-Regular.ttf`

This path works only on systems where that font is installed at that location. If the project is moved to another computer, the font path is a portability point to review.

## 9. Important C++ lessons from the final milestone

### Scoped enums communicate outcomes

`Tile::HarvestResult` and `Crop::WaterResult` communicate a specific set of operation outcomes. Callers compare against meaningful enumerators instead of relying on magic integer values.

### Return results instead of printing inside the model

`Crop::Water()` and `Tile::Harvest()` return results. The console decides what to print; SDL3 decides what notification to render. This prevents the core model from being tied to one interface.

### Encapsulation still permits whole-object assignment

`Crop`'s stage and watering fields are private, so `Tile` cannot directly assign to them. However, the `Crop` object is a public member of `Tile`, and assigning `crop = Crop{}` replaces the existing object with a default-initialized `Crop`. That resets the whole object's state without exposing its private fields.

### `const` getters describe intent

`GetProduceCount() const` and `GetSeedCount() const` read inventory state without mutating it. The `const` after the member function's parameter list is a promise that the function will not modify the object's ordinary member state.

### State validation comes before state mutation

Planting, selling, and buying all verify the conditions for success before consuming or awarding resources. Harvesting is centralised in `Tile::Harvest()` so that callers do not duplicate the tile-state reset.

### Keep presentation code separate from resource data

The inventory stores counts; the HUD reads them and displays them. A persistent HUD is not the inventory itself. It is a presentation of the current game state.

### Avoid premature abstraction

The project did not need a large game manager, economy class, or UI framework merely to track two resource types and one coin counter. A small `Inventory` class and a `money` variable are adequate at this scale.

## 10. Current gameplay rules and a crop-growth detail to verify

The current `Crop::DayCheck()` implementation behaves as follows:

- If a crop is Matured or Withered, it resets `wateredToday` and returns without changing growth.
- If watered that day, it increments `wateringProgress`.
- After `wateringProgress` reaches 3, a Seed becomes Sprouted or a Sprouted crop becomes Matured, and `wateringProgress` is reset to zero.
- If not watered, it decrements `grace`; reaching zero changes the crop to Withered.
- If watered, it restores `grace` by one up to its maximum of 2.
- It clears `wateredToday` at the end of the day check for active crops.

**One rule deserves an explicit final check before the project is archived:** `wateringProgress` is not reset when a day is missed, so the current logic counts successful watering days cumulatively rather than enforcing three consecutive watering days. Also, with `grace` initially set to 2, two day checks without watering from a full grace value take it to zero and wither the crop. If the intended design was “three consecutive watering days to grow, a one-day watering grace period, and withering after three missed days,” the implementation does not exactly match that design yet.

This note is not claiming that the current behaviour makes the game unplayable. It records the actual code behaviour so the intended rule can be deliberately accepted or adjusted instead of forgotten.

## 11. Final verification checklist

Before calling the repository fully closed, run the actual local build and test both programs. The graphical version was run locally during the session and the persistent HUD was reported to look correct; this checklist is a repeatable final regression pass.

### Build checks

- [ ] Run `make clean` followed by `make` for the SDL3 target.
- [ ] Build the console program separately; do not link the console and SDL sources into one executable because each defines `main()`.
- [ ] Confirm no compiler warnings have been introduced, or review any that remain.

### Gameplay checks

- [ ] The game starts with 3 seeds, 0 produce, and 0 coins.
- [ ] Pressing `P` without selecting a tile displays a message and does not access an invalid array element.
- [ ] Planting on an occupied tile does not consume a seed.
- [ ] Planting on an empty tile consumes exactly one seed.
- [ ] Attempting to plant with zero seeds does not create a crop.
- [ ] Watering an empty tile fails cleanly.
- [ ] Watering the same crop twice in one day returns `AlreadyWatered` on the second attempt.
- [ ] Advancing the day updates crop state and increments the day.
- [ ] Immature crops cannot be harvested.
- [ ] Withered crops cannot be harvested.
- [ ] Harvesting an empty tile returns `NoCrop`.
- [ ] Successful harvest clears the tile, resets the crop, and adds exactly one produce.
- [ ] Selling with no produce does not add coins.
- [ ] Selling one produce removes one produce and adds 5 coins.
- [ ] Buying a seed with at least 3 coins spends exactly 3 coins and adds one seed.
- [ ] Buying a seed with fewer than 3 coins changes neither money nor seed count.
- [ ] The persistent HUD stays visible and reflects the day, seed count, produce count, and money after changes.
- [ ] Notifications appear for about 1.5 seconds and do not replace the persistent HUD.
- [ ] Closing the window exits cleanly.

### Repository close-out

- [ ] Review `git status` and `git diff`.
- [ ] Keep unrelated or generated files out of the final commit.
- [ ] Commit the completed milestone with a clear message, for example: `Finish farming RPG gameplay and HUD`.
- [ ] Keep this Day 09 note in the repository's notes folder or project root so it remains with the code.

## 12. Final scope and deliberate omissions

This project ends as a compact 2D fixed-overhead farm-management game. The implemented focus is crop state, tile interaction, resource accounting, a minimal economy, SDL3 rendering, input handling, text textures, notifications, and a persistent HUD.

The following were intentionally left for a future project rather than added at the last minute:

- A controllable player sprite and movement.
- A camera or scrolling world.
- A larger farm or a larger window.
- Multiple crop species and per-crop prices.
- Seasons and weather.
- NPCs, quests, crafting, and elaborate progression.
- A full inventory or shop screen.
- Save/load support.

These omissions are scope decisions, not unfinished requirements for this small prototype. The project has served its purpose if it builds reliably, the core loop behaves consistently, and the code remains understandable enough to extend later.

## 13. The central takeaway

The most valuable result is not merely that crops appear on screen. The game now has a chain of connected systems:

**input → game rules → state changes → resource rewards → HUD and notifications**

The console and SDL3 interfaces can use the same core crop, tile, and inventory operations while presenting outcomes differently. This is the foundation of a maintainable small game: rules have clear owners, operations report their results, and the interface reflects the current state.

**Day 09 goal:** finish the current scope, test it, document it, commit it, and stop expanding it. A small complete game is a better finish than a large project that never stops growing.
