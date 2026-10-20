#include <iostream>

struct Crop {

    enum Stage{
        Seed,
        Sprouted,
        Matured,
        Withered
    };

    Stage current = Seed;
    int wateringProgress = 0;
    int grace = 2;
    bool wateredToday = false;

    void Water(){

        if(current == Matured || current == Withered){
            std::cout<<"You can't water these now, sadly..\n";
            return;
        }
        else{
            if(wateredToday == true) {
                std::cout<<"You have already watered the crop !!\n";
            }
            else if(wateredToday == false) {
                wateredToday = true;
                std::cout<<"Crop watered successfully\n";
            } 
        }

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

    
};

int main(){

    std::cout<<"*******YOUR FARM********\n";

    int day = 1;

    char farm[3][3];

    for(int row = 0; row < 3; row++){
        for(int col = 0; col < 3; col++){
            farm[row][col] = '.';
        }
    }

    for(int row = 0; row < 3; row++){
        for(int col = 0; col < 3; col++){
            std::cout<<farm[row][col];
        }
        std::cout<<"\n";
    }

    std::cout << "\nDay: " << day << '\n';

    std::cout<<"\n***********************\n";
    int r;
    std::cout << "Enter row (0 to 2): ";
    std::cin >> r;

    while (r < 0 || r >= 3) {
        std::cout << "Invalid row! Enter a value from 0 to 2: ";
        std::cin >> r;
    }

    int c;
    std::cout << "Enter column (0 to 2): ";
    std::cin >> c;

    while (c < 0 || c >= 3) {
        std::cout << "Invalid column! Enter a value from 0 to 2: ";
        std::cin >> c;
    }

    if (farm[r][c] == '.') {
        farm[r][c] = 'S';
        std::cout << "\nYour seed has been planted!\n";
    }   
    else {
        std::cout << "\nThat tile is already occupied!\n";
    }


    std::cout<<"\n*******YOUR FARM********\n";

    for(int row = 0; row < 3; row++){
        for(int col = 0; col < 3; col++){
            std::cout<<farm[row][col];
        }
        std::cout<<"\n";
    }




}