#include <iostream>
#include <limits>

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

    Stage GetStage() const{
        return current;
    }

};

struct Tile {
    bool hasCrop = false;
    Crop crop;
};

void GetCoordinates(int& row, int& col){

    while (true) {
        std::cout << "Enter row (0 to 2): ";

        if (std::cin >> row) {
            if (row >= 0 && row < 3) {
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
                break;
            }
        }
        else {
            std::cin.clear();
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "Invalid row! Enter a value from 0 to 2.\n";
    }

    while (true) {
        std::cout << "Enter column (0 to 2): ";

        if (std::cin >> col) {
            if (col >= 0 && col < 3) {
            std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
            );
                break;
            }
        }
        else {
            std::cin.clear();
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "Invalid column! Enter a value from 0 to 2.\n";
    }

}

int main(){

    int day = 1;
    Tile farm[3][3];

    bool running = true;

    while(running){

        std::cout<<"====================YOUR FARM=====================\n";
        std::cout << "\nDay: " << day << '\n';

        for(int row = 0; row < 3; row++){
            for(int col = 0; col < 3; col++){

                if (farm[row][col].hasCrop == false) {
                    std::cout<<"🟫";
                }                
                else if (farm[row][col].crop.GetStage() == Crop::Seed ) {
                    std::cout<<"🟤";
                }
                else if (farm[row][col].crop.GetStage() == Crop::Sprouted) {
                    std::cout<<"🌱";
                }
                else if (farm[row][col].crop.GetStage() == Crop::Matured) {
                    std::cout<<"🌿";
                }
                else if (farm[row][col].crop.GetStage() == Crop::Withered) {
                    std::cout<<"🍂";
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

        while (true) {
            std::cout << "Choose an action: ";

            if (std::cin >> choice) {
                if (choice >= 1 && choice <= 4) {
                    std::cin.ignore(
                        std::numeric_limits<std::streamsize>::max(), '\n'
                    );
                    break;
                }

                // An integer was read, but it was out of range.
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
            }
            else {
                
                // The input operation failed.
                std::cin.clear();
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
            }

                std::cout << "Invalid choice! Enter a value from 1 to 4.\n";
        
        }

        if (choice == 1) {

            int r;
            int c;

            GetCoordinates(r, c);

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
            int c;

            GetCoordinates(r, c);
            
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