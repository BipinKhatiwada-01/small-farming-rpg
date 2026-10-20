# Day 1 --- Farm Tiles & Crop Struct

> **Project:** Small farming RPG\
> **Checkpoint:** Tile Grid + Crop Lifecycle\
> **Status:** Complete for Day 1\
> **Next:** Connect crop objects to farm tiles

------------------------------------------------------------------------

## 1. What I Built Today

Today, I worked on two independent parts of my farming RPG:

1.  A **farm tile grid** that tracks which tiles are empty or occupied.
2.  A **`Crop` struct** that manages a crop's growth, watering, and
    survival.

These systems are not connected yet. The grid tracks tile occupancy,
while the struct manages an individual crop's lifecycle.

## 2. The Farm Tile Grid

### Representation

The farm uses a two-dimensional character array:

``` cpp
char farm[3][3];
```

Each character represents the state of a tile.

  Symbol   Meaning
  -------- --------------------------------
  `.`      Empty tile
  `S`      Tile containing a planted seed

The grid is initialized with `'.'` in every position.

Example:

``` text
*******
YOUR FARM
*******
. . .
. . .
. . .
```

After planting a seed at row `1`, column `2`:

``` text
. . .
. . S
. . .
```

*The example uses zero-based indexing.*

### Planting Logic

The planting process follows these steps:

1.  Ask the player for a row and column.
2.  Validate that both coordinates are within the grid.
3.  Check whether the selected tile contains `'.'`.
4.  If empty, change it to `'S'`.
5.  Otherwise, inform the player that the tile is occupied.

### Important Rules

-   A 3 × 3 grid has nine tiles.
-   Valid indices are `0`, `1`, and `2`.
-   Array bounds must be checked **before accessing an element**.
-   A tile's character currently represents its occupancy state.

For example:

``` cpp
if (r < 0 || r >= 3 || c < 0 || c >= 3)
{
    // Invalid coordinates
}
```

This prevents invalid array access.

### Mental Model

The grid answers:

> **Which tiles are empty, and which are occupied?**

It does not yet know anything about watering, crop growth, or harvests.

------------------------------------------------------------------------

## 3. The `Crop` Struct

A struct groups related data and behavior into one type.

Instead of tracking every crop property separately, I can create a
`Crop` object that manages its own state.

### Crop State

  Member               Purpose
  -------------------- -----------------------------------------------------------
  `Stage`              Defines the possible crop stages
  `current`            Stores the crop's current stage
  `wateringProgress`   Counts successful watering days toward growth
  `grace`              Tracks the crop's remaining tolerance for missed watering
  `wateredToday`       Records whether the crop has been watered today

The stages are:

``` cpp
enum Stage
{
    Seed,
    Sprouted,
    Matured,
    Withered
};
```

The enum gives each stage a meaningful name instead of requiring
arbitrary numbers or characters.

For example, `current == Matured` is easier to understand than
`current == 2`.

### Default State

My struct contains these initial values:

``` cpp
Stage current = Seed;
int wateringProgress = 0;
int grace = 2;
bool wateredToday = false;
```

This means a newly created crop starts as a seed, has no accumulated
watering progress, has two days of missed-watering tolerance, and has
not been watered today.

------------------------------------------------------------------------

## 4. The `Water()` Method

The `Water()` method handles the player's attempt to water a crop.

Its rules are:

-   Matured crops cannot be watered.
-   Withered crops cannot be watered.
-   A crop can only be watered once per day.
-   Successful watering sets `wateredToday` to `true`.

### Why `wateredToday` Matters

Watering and growth are separate events.

`Water()` records that watering occurred. It does not immediately
advance the crop's growth stage.

The daily update later checks this flag and determines what happens.

This separation is useful because it prevents the watering action from
directly controlling the entire growth system.

------------------------------------------------------------------------

## 5. The `DayCheck()` Method

`DayCheck()` processes the crop's daily update.

It must be called **once per in-game day** for the crop to behave
correctly.

### Growth Rules

Each successful watering day increases `wateringProgress`.

After three successful watering days:

-   `Seed` becomes `Sprouted`.
-   `Sprouted` becomes `Matured`.

When a stage transition occurs, `wateringProgress` resets to zero.

The crop does not continue growing after reaching `Matured`.

### Missed-Watering Rules

The `grace` value begins at `2`.

  -----------------------------------------------------------------------
  Event                               Result
  ----------------------------------- -----------------------------------
  Crop watered                        Growth progress increases

  Crop misses one day                 Grace decreases by one

  Crop misses two consecutive days    Crop becomes withered

  Crop is watered after missing one   Grace recovers by one, up to its
  day                                 maximum of two
  -----------------------------------------------------------------------

A crop that is not watered does not gain growth progress that day.

However, its previously accumulated growth progress is retained.

### Why Reset `wateredToday`?

At the end of the daily update, the flag is reset:

``` cpp
wateredToday = false;
```

This prepares the crop for the next in-game day.

Without this reset, a crop watered yesterday could incorrectly count as
watered again today.

### Why Handle Matured and Withered Crops Early?

These stages are terminal under the current design.

-   A matured crop no longer needs to grow.
-   A withered crop cannot recover through watering.

The method resets the daily watering flag and returns early for these
stages.

This prevents further growth or grace-period changes.

------------------------------------------------------------------------

## 6. How the Two Systems Differ

The farm grid and the crop struct currently have different
responsibilities.

``` mermaid
flowchart TD
    A["Farm Grid<br/>char farm[3][3]"] --> B["Tracks tile occupancy"]
    C["Crop Struct<br/>Crop"] --> D["Tracks crop lifecycle"]
    B -. "Not connected yet" .-> C
```

The grid knows whether a tile is empty or occupied.

The crop object knows its growth stage, watering progress, and survival
state.

**Important:** I have not yet connected a crop object to a particular
grid position. That is the next development step, not part of this
checkpoint.

------------------------------------------------------------------------

## 7. Key Lessons

-   A two-dimensional array can represent a farm's rows and columns.
-   Array bounds must be validated before indexing.
-   A struct groups related state and behavior.
-   An enum makes state transitions more readable.
-   Methods can modify an object's internal state.
-   Watering is an action; daily growth is a separate update.
-   Boolean flags can track whether an event occurred during a
    particular day.
-   Separating responsibilities makes future features easier to reason
    about.

## 8. Things to Watch Out For

-   **Calling `DayCheck()` multiple times per day:** This would advance
    growth and reduce grace too quickly.
-   **Forgetting to reset `wateredToday`:** Yesterday's watering could
    incorrectly carry into the next day.
-   **Accessing the grid before validating coordinates:** This can
    result in out-of-bounds access.
-   **Treating every `Crop` object as a planted crop:** A
    default-constructed crop starts at `Seed`, even if no seed has
    actually been planted on the farm. This is one reason the grid and
    crop objects still need to be connected carefully.
-   **Confusing occupancy with lifecycle:** A tile being occupied and a
    crop being mature are two different pieces of information.

## 9. Checkpoint Summary

At the end of Day 1, I have:

-   [x] Created a 3 × 3 farm grid.
-   [x] Initialized the tiles as empty.
-   [x] Implemented coordinate validation.
-   [x] Implemented basic planting and occupancy checks.
-   [x] Defined the crop growth stages.
-   [x] Implemented watering restrictions.
-   [x] Implemented growth progression.
-   [x] Implemented missed-watering tolerance and withering.
-   [ ] Connected crop objects to grid tiles.
-   [ ] Integrated daily updates with the farm.
-   [ ] Implemented harvesting.

### Self-Check

Before moving on, I should be able to explain:

1.  Why does `wateringProgress` exist separately from `wateredToday`?
2.  Why must `DayCheck()` run exactly once per day?
3.  Why can a default `Crop` object exist even when no seed has been
    planted?
4.  Why is checking array bounds important before accessing
    `farm[r][c]`?
5.  Why are tile occupancy and crop growth separate responsibilities?

**Next checkpoint:** Connect the farm grid to individual crop objects
while keeping the responsibilities of each system clear.
