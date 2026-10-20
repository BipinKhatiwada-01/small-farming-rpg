#include <iostream>

class Crop {
public:
    enum Stage {
        Seed,
        Sprouted,
        Matured,
        Withered
    };

private:
    Stage current = Seed;
    int wateringProgress = 0;
    int grace = 2;
    bool wateredToday = false;

public:

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

struct Tile {
    bool hasCrop = false;
    Crop crop;
};

int main(){

    int day = 1;
    Tile farm[3][3];

    bool running = true;

    while(running){

        std::cout<<"====================YOUR FARM=====================\n";
        std::cout << "\nDay: " << day << '\n';

        for(int row = 0; row < 3; row++){
            for(int col = 0; col < 3; col++){
                if (farm[row][col].hasCrop == true) {
                    std::cout<<"S";
                }
                else if (farm[row][col].hasCrop == false) {
                    std::cout<<".";
                }
            }
                std::cout<<"\n";
        }

        std::cout<<"\n==============Small farming RPG==================\n";
        std::cout<<"1. Plant a crop\n";
        std::cout<<"2. Water a crop\n";
        std::cout<<"3. End day\n";
        std::cout<<"4. Quit\n";

        int choice;
        std::cout<<"\n\nChoose an action: ";
        std::cin>>choice;

        while(choice < 1 || choice > 4) {
            std::cout << "Invalid choice! Enter 1 to 4: ";
            std::cin >> choice;
        }

        if (choice == 1) {

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

            if (farm[r][c].hasCrop == false) { 

                Crop crop;
                farm[r][c].crop = crop;

                farm[r][c].hasCrop = true;

                std::cout << "\nYour seed has been planted!\n";
            }   
            else {
                std::cout << "\nThat tile is already occupied!\n";
            }

        }
        else if (choice == 2) {
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
            
            if (farm[r][c].hasCrop == true) { 

                farm[r][c].crop.Water();

            }   
            else {
                std::cout << "\nCan't water in an empty land!\n";
            }
        }
        else if (choice == 3) {
            for(int row = 0; row < 3; row++){
                for(int col = 0; col < 3; col++){
                    if (farm[row][col].hasCrop == true) {

                        farm[row][col].crop.DayCheck();

                    }
                }
            }
            day++;
        }
        else if (choice == 4) {
            std::cout<<"==========Thanks for playing !!=================\n";
            running = false;
       } 

    }

}