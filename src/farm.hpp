#pragma once

#include <iostream>

class Crop {
public:
    enum Stage {
        Seed,
        Sprouted,
        Matured,
        Withered
    };

    enum class WaterResult {
        Watered,
        AlreadyWatered,
        CropFinished
    };

private:
    Stage current = Seed;
    int wateringProgress = 0;
    int grace = 2;
    bool wateredToday = false;

public:

    WaterResult Water(){

        if(current == Matured || current == Withered){
            return WaterResult::CropFinished;
        }

        if(wateredToday) {
            return WaterResult::AlreadyWatered;
        }
            wateredToday = true;
            return WaterResult::Watered;
    }   

    void DayCheck(){

        if(current == Matured || current == Withered){
            wateredToday = false;
            return;
        }  

        if (wateredToday == true) {
            wateringProgress++;
        }

        if (wateringProgress >= 3) {
            if (current == Seed) {
                current = Sprouted;
                wateringProgress = 0;
            }
            else if (current == Sprouted) {
                current = Matured;
                wateringProgress = 0;
            }
        }

        if(wateredToday == false){
            grace--;
            if (grace == 0) {
                current = Withered;
            }
        }
        else if (wateredToday == true) {
            if (grace < 2) {
                grace++;
            }
        }

        wateredToday = false;
    } 

    Stage GetStage() const{
        return current;
    }

};

struct Tile {
    bool hasCrop = false;
    Crop crop;
};
