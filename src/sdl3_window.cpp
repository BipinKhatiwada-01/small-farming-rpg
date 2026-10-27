#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include "farm.hpp"
#include <iostream>

bool UpdateTextTexture(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const std::string& text,
    SDL_Color color,
    SDL_Texture*& texture,
    SDL_FRect& destination
) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(
        font, text.c_str(), 0, color
    );

    if(textSurface == nullptr) {
        std::cerr << "Failed to render text: "
                  << SDL_GetError() << '\n';
        return false;
    }

   SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

   if(newTexture == nullptr) {
        std::cerr << "Failed to convert Texture from surface: "
                 << SDL_GetError() << '\n';
        
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
}

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

    if(!TTF_Init()) {
        std::cerr << "SDL_ttf failed to initialize: "
              << SDL_GetError() << '\n';

        SDL_DestroyTexture(terrainSheet);
        SDL_DestroyTexture(cropSheet);
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont(
    "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
    24.0f
    );

    if(font == nullptr) {
        std::cerr << "Font failed to load: " << SDL_GetError() << '\n';

        TTF_Quit();
        SDL_DestroyTexture(terrainSheet);
        SDL_DestroyTexture(cropSheet);
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor{255, 255, 255, 255};

    SDL_Surface* daySurface = TTF_RenderText_Blended(
        font, "Day 1", 0, textColor
    );

    if(daySurface == nullptr) {
        std::cerr << "Text rendering failed: " << SDL_GetError() << '\n';

        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyTexture(terrainSheet);
        SDL_DestroyTexture(cropSheet);
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* dayTexture = SDL_CreateTextureFromSurface(
        renderingPointer, daySurface
    );

    if(dayTexture == nullptr) {
        std::cerr << "Failed to create day texture: "
              << SDL_GetError() << '\n';

        SDL_DestroySurface(daySurface);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyTexture(terrainSheet);
        SDL_DestroyTexture(cropSheet);
        SDL_DestroyRenderer(renderingPointer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_FRect dayRect{
        24.0f,
        24.0f,
        static_cast<float>(daySurface->w),
        static_cast<float>(daySurface->h)
    };

    SDL_DestroySurface(daySurface);
    // These rectangles select 16x16 tiles from the terrain sheet.
    const SDL_FRect grassSource{0.0f, 16.0f, 16.0f, 16.0f};
    const SDL_FRect wildGrassA{16.0f, 0.0f, 16.0f, 16.0f};
    const SDL_FRect wildGrassB{32.0f, 0.0f, 16.0f, 16.0f};
    const SDL_FRect sandyGroundSource{16.0f, 48.0f, 16.0f, 16.0f};

    SDL_Texture* notificationTexture = nullptr;

    SDL_FRect notificationRect{
        24.0f,
        548.0f,
        0.0f,
        0.0f
    };

    Tile farm[FARM_SIZE][FARM_SIZE];
    int day = 1;

    std::string notificationText = "";

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

                    notificationText = "Select a tile first !";
                    std::cout << notificationText << '\n';
                }
                else if(farm[selectedRow][selectedCol].hasCrop) {

                    notificationText = "That tile is already occupied !";
                    std::cout << notificationText << '\n';
                }                    
                else {
                    farm[selectedRow][selectedCol].hasCrop = true;
                    notificationText = "Seed planted!";
                    std::cout << notificationText << '\n';
                }

                UpdateTextTexture
                (
                    renderingPointer, 
                    font, notificationText, textColor,
                    notificationTexture, 
                    notificationRect
                );
            }
            else if(event.type == SDL_EVENT_KEY_DOWN &&
                    event.key.key == SDLK_W && !event.key.repeat) {

                if(selectedRow < 0 || selectedRow >= FARM_SIZE ||
                   selectedCol < 0 || selectedCol >= FARM_SIZE) {
                    notificationText = "Select a tile first !";
                    std::cout << notificationText << '\n';
                }
                else if(!farm[selectedRow][selectedCol].hasCrop) {
                    notificationText = "You can't apply water there !";
                    std::cout << notificationText << '\n';
                }
                else {
                    Crop::WaterResult waterResult = farm[selectedRow][selectedCol].crop.Water();

                    if (waterResult == Crop::WaterResult::Watered) {
                        notificationText = "Crop watered !";
                        std::cout << notificationText << '\n';
                    }
                    else if (waterResult == Crop::WaterResult::AlreadyWatered) {
                        notificationText = "You have already watered this crop !";
                        std::cout << notificationText << '\n';
                    }
                    else if (waterResult == Crop::WaterResult::CropFinished) {
                        notificationText = "You can't water these now sadly..";
                        std::cout << notificationText << '\n';                       
                    
                    }
                }

                UpdateTextTexture
                (
                        renderingPointer, 
                        font, notificationText, textColor,
                        notificationTexture, 
                        notificationRect
                );
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
                std::string dayText = "Day " + std::to_string(day);

                SDL_Surface* newDaySurface = TTF_RenderText_Blended(
                    font, dayText.c_str(), 0, textColor
                );

                if(newDaySurface != nullptr) {
                    SDL_Texture* newDayTexture = SDL_CreateTextureFromSurface(
                        renderingPointer, newDaySurface
                    );

                    if(newDayTexture != nullptr) {
                        dayRect.w = static_cast<float>(newDaySurface->w);
                        dayRect.h = static_cast<float>(newDaySurface->h);

                        SDL_DestroyTexture(dayTexture);
                        dayTexture = newDayTexture;
                    }
                    else {
                        std::cerr << "Failed to update day texture: "
                            << SDL_GetError() << '\n';
                    }

                    SDL_DestroySurface(newDaySurface);
                }
                else {
                    std::cerr << "Failed to render day text: "
                        << SDL_GetError() << '\n';
                }

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

        SDL_RenderTexture(
            renderingPointer,
            dayTexture,
            nullptr,
            &dayRect
        );

        if(notificationTexture != nullptr){
            SDL_RenderTexture(
                renderingPointer, 
                notificationTexture, 
                nullptr, 
                &notificationRect
            );
        }

        SDL_RenderPresent(renderingPointer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(terrainSheet);
    SDL_DestroyTexture(cropSheet);
    SDL_DestroyTexture(dayTexture);
    SDL_DestroyRenderer(renderingPointer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}