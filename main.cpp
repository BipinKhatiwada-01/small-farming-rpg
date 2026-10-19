#include <iostream>

int main(){

    std::cout<<"*******YOUR FARM********\n";

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

    std::cout << "\nYour seed has been planted!\n";

 
    std::cout<<"\n*******YOUR FARM********\n";

    for(int row = 0; row < 3; row++){
        for(int col = 0; col < 3; col++){
            std::cout<<farm[row][col];
        }
        std::cout<<"\n";
    }




}