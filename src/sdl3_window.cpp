#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include "farm.hpp"
#include <iostream>

int main() {
    
    if(!SDL_Init(SDL_INIT_VIDEO)){

        std::cerr << "Video failed to initialize: " << SDL_GetError() << '\n';

        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Farming RPG - SDL3 Test", 800, 600, 0);

    if(window == nullptr){

        std::cerr << "Window failed to initialize: " << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderingPointer = SDL_CreateRenderer(window, nullptr);

    if(renderingPointer == nullptr){

        std::cerr << "Rendering failed: " << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;

    } 

    Tile farm[3][3];
    farm[1][1].hasCrop = true;

    int selectedRow = -1;
    int selectedCol = -1;

    bool running = true;

    while(running){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                    event.button.button == SDL_BUTTON_LEFT) {

                int relativeX = static_cast<int>(event.button.x) - 400;
                int relativeY = static_cast<int>(event.button.y) - 180;

                if (relativeX < 0 || relativeX >= 300 ||
                    relativeY < 0 || relativeY >= 300) {
                continue;
                }

                if (relativeX % 100 >= 90 ||
                    relativeY % 100 >= 90) {
                continue;
                }

                selectedCol = relativeX / 100;
                selectedRow = relativeY / 100;
            }
        }

        SDL_SetRenderDrawColor(renderingPointer, 40, 80, 55, 255);
        SDL_RenderClear(renderingPointer);

        
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {

                float x = 400 + col * 100;
                float y = 180 + row * 100;

                
                if (farm[row][col].hasCrop) {
                    SDL_SetRenderDrawColor(renderingPointer, 131, 100, 60, 255);
                }
                else {
                    SDL_SetRenderDrawColor(renderingPointer, 50, 130, 10, 255);
                }

                SDL_FRect tileRect{x, y, 90.0f, 90.0f};
                SDL_RenderFillRect(renderingPointer, &tileRect);

                if (row == selectedRow && col == selectedCol) {
                    SDL_SetRenderDrawColor(renderingPointer, 0, 0, 0, 255);
                    SDL_RenderRect(renderingPointer, &tileRect);
                }
       
                if (farm[row][col].hasCrop) {

                    Crop::Stage stage = farm[row][col].crop.GetStage();

                    if (stage == Crop::Seed) {
                        SDL_SetRenderDrawColor(renderingPointer, 180, 140, 70, 255);
                    }
                    else if (stage == Crop::Sprouted) {
                        SDL_SetRenderDrawColor(renderingPointer, 120, 200, 80, 255);
                    }
                    else if (stage == Crop::Matured) {
                        SDL_SetRenderDrawColor(renderingPointer, 45, 130, 50, 255);
                    }
                    else {
                        SDL_SetRenderDrawColor(renderingPointer, 160, 125, 75, 255);
                    }

                    SDL_FRect stageRect{x + 30, y + 30, 30.0f, 30.0f};
                    SDL_RenderFillRect(renderingPointer, &stageRect);
                }
            }
        }


        SDL_RenderPresent(renderingPointer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderingPointer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}