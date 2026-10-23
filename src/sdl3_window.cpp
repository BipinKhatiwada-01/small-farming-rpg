#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
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

    bool running = true;

    while(running){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderingPointer, 40, 80, 55, 255);
        SDL_RenderClear(renderingPointer);
        SDL_SetRenderDrawColor(renderingPointer, 145, 100, 60, 255);
        
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {

                float x = 100 + col * 100;
                float y = 100 + row * 100;


                    SDL_FRect tileRect{x, y, 90.0f, 90.0f};

                    SDL_RenderFillRect(renderingPointer, &tileRect);

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