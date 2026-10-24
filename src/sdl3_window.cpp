#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include "farm.hpp"
#include <iostream>

int main() {
    
    constexpr int WINDOW_W = 800;
    constexpr int WINDOW_H = 600;

    constexpr int GRID_X = 256;
    constexpr int GRID_Y = 156;
    constexpr int TILE_SIZE = 96;
    constexpr int FARM_SIZE = 3;

    constexpr int BACKGROUND_TILE_SIZE = 48;

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Video failed to initialize: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Farming RPG - SDL3 Test", WINDOW_W, WINDOW_H, 0);

    if(window == nullptr) {
        std::cerr << "Window failed to initialize: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderingPointer = SDL_CreateRenderer(window, nullptr);

    if(renderingPointer == nullptr) {
        std::cerr << "Rendering failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* cropSheet = IMG_LoadTexture(renderingPointer, "./assets/vectoraith/crops.png");

    if(cropSheet == nullptr) {
        std::cerr << "Failed to load crop sheet: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetTextureScaleMode(cropSheet, SDL_SCALEMODE_NEAREST);

    SDL_Texture* terrainSheet = IMG_LoadTexture(
        renderingPointer,
        "./assets/vectoraith/terrain_summer_expanded.png"
    );

    if(terrainSheet == nullptr) {
        std::cerr << "Failed to load terrain sheet: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(cropSheet);
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetTextureScaleMode(terrainSheet, SDL_SCALEMODE_NEAREST);

    // These rectangles select 16x16 tiles from the terrain sheet.
    const SDL_FRect grassSource{0.0f, 16.0f, 16.0f, 16.0f};
    const SDL_FRect wildGrassA{16.0f, 0.0f, 16.0f, 16.0f};
    const SDL_FRect wildGrassB{32.0f, 0.0f, 16.0f, 16.0f};
    const SDL_FRect sandyGroundSource{16.0f, 48.0f, 16.0f, 16.0f};

    Tile farm[FARM_SIZE][FARM_SIZE];
    int day = 1;

    int selectedRow = -1;
    int selectedCol = -1;

    bool running = true;

    while(running) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                    event.button.button == SDL_BUTTON_LEFT) {

                int relativeX = static_cast<int>(event.button.x) - GRID_X;
                int relativeY = static_cast<int>(event.button.y) - GRID_Y;
                int farmPixelSize = TILE_SIZE * FARM_SIZE;

                if(relativeX < 0 || relativeX >= farmPixelSize ||
                   relativeY < 0 || relativeY >= farmPixelSize) {
                    continue;
                }

                selectedCol = relativeX / TILE_SIZE;
                selectedRow = relativeY / TILE_SIZE;
            }
            else if(event.type == SDL_EVENT_KEY_DOWN &&
                    event.key.key == SDLK_P && !event.key.repeat) {

                if(selectedRow < 0 || selectedRow >= FARM_SIZE ||
                   selectedCol < 0 || selectedCol >= FARM_SIZE) {
                    std::cout << "Select a tile first !\n";
                }
                else if(farm[selectedRow][selectedCol].hasCrop) {
                    std::cout << "That tile is already occupied !\n";
                }
                else {
                    farm[selectedRow][selectedCol].hasCrop = true;
                    std::cout << "Seed planted !!\n";
                }
            }
            else if(event.type == SDL_EVENT_KEY_DOWN &&
                    event.key.key == SDLK_W && !event.key.repeat) {

                if(selectedRow < 0 || selectedRow >= FARM_SIZE ||
                   selectedCol < 0 || selectedCol >= FARM_SIZE) {
                    std::cout << "Select a tile first !\n";
                }
                else if(!farm[selectedRow][selectedCol].hasCrop) {
                    std::cout << "You can't apply water there\n";
                }
                else {
                    farm[selectedRow][selectedCol].crop.Water();
                }
            }
            else if(event.type == SDL_EVENT_KEY_DOWN &&
                    event.key.key == SDLK_E && !event.key.repeat) {

                for(int row = 0; row < FARM_SIZE; row++) {
                    for(int col = 0; col < FARM_SIZE; col++) {
                        if(farm[row][col].hasCrop) {
                            farm[row][col].crop.DayCheck();
                        }
                    }
                }

                day++;
                std::cout << "Day " << day << " begins!\n";
            }
        }

        // Clear the frame, then draw the grass around the farm.
        SDL_SetRenderDrawColor(renderingPointer, 30, 43, 34, 255);
        SDL_RenderClear(renderingPointer);

        for(int bgRow = 0, bgY = 0; bgY < WINDOW_H; bgRow++, bgY += BACKGROUND_TILE_SIZE) {
            for(int bgCol = 0, bgX = 0; bgX < WINDOW_W; bgCol++, bgX += BACKGROUND_TILE_SIZE) {
                const SDL_FRect* backgroundSource = &grassSource;
                int pattern = (bgCol * 7 + bgRow * 11) % 9;

                if(pattern == 0) {
                    backgroundSource = &wildGrassA;
                }
                else if(pattern == 5) {
                    backgroundSource = &wildGrassB;
                }

                SDL_FRect backgroundTile{
                    static_cast<float>(bgX),
                    static_cast<float>(bgY),
                    static_cast<float>(BACKGROUND_TILE_SIZE),
                    static_cast<float>(BACKGROUND_TILE_SIZE)
                };

                SDL_RenderTexture(renderingPointer, terrainSheet, backgroundSource, &backgroundTile);
            }
        }

        // Draw the farm plot over the background.
        for(int row = 0; row < FARM_SIZE; row++) {
            for(int col = 0; col < FARM_SIZE; col++) {
                float x = static_cast<float>(GRID_X + col * TILE_SIZE);
                float y = static_cast<float>(GRID_Y + row * TILE_SIZE);

                SDL_FRect tileRect{
                    x, y,
                    static_cast<float>(TILE_SIZE),
                    static_cast<float>(TILE_SIZE)
                };

                // Every tile in this 3x3 plot is farmable, so show soil even when empty.
                SDL_RenderTexture(renderingPointer, terrainSheet, &sandyGroundSource, &tileRect);

                if(farm[row][col].hasCrop) {
                    Crop::Stage stage = farm[row][col].crop.GetStage();
                    SDL_FRect sourceRect{};

                    if(stage == Crop::Seed) {
                        sourceRect = SDL_FRect{195.0f, 18.0f, 11.0f, 11.0f};
                    }
                    else if(stage == Crop::Sprouted) {
                        sourceRect = SDL_FRect{209.0f, 12.0f, 13.0f, 17.0f};
                    }
                    else if(stage == Crop::Withered) {
                        sourceRect = SDL_FRect{225.0f, 7.0f, 14.0f, 22.0f};
                    }
                    else if(stage == Crop::Matured) {
                        sourceRect = SDL_FRect{241.0f, 7.0f, 14.0f, 22.0f};
                    }

                    // Scale the sprite evenly and keep the crop bottoms aligned.
                    constexpr float spriteScale = 3.0f;
                    float spriteW = sourceRect.w * spriteScale;
                    float spriteH = sourceRect.h * spriteScale;

                    SDL_FRect destinationRect{
                        x + (TILE_SIZE - spriteW) / 2.0f,
                        y + TILE_SIZE - 8.0f - spriteH,
                        spriteW,
                        spriteH
                    };

                    SDL_RenderTexture(renderingPointer, cropSheet, &sourceRect, &destinationRect);
                }

                // Draw the selected tile's outline on top of the artwork.
                if(row == selectedRow && col == selectedCol) {
                    SDL_SetRenderDrawColor(renderingPointer, 0, 0, 0, 255);
                    SDL_RenderRect(renderingPointer, &tileRect);
                }
            }
        }

        SDL_RenderPresent(renderingPointer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(terrainSheet);
    SDL_DestroyTexture(cropSheet);
    SDL_DestroyRenderer(renderingPointer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}