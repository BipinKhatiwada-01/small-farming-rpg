#include <iostream>
#include <limits>

#include "farm.hpp"
#include "inventory.hpp"

namespace {
constexpr int FARM_SIZE = 3;
constexpr int SEED_PRICE = 3;
constexpr int PRODUCE_SELL_PRICE = 5;

void GetCoordinates(int& row, int& col) {
    while (true) {
        std::cout << "Enter row (0 to " << FARM_SIZE - 1 << "): ";

        if (std::cin >> row) {
            if (row >= 0 && row < FARM_SIZE) {
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
                break;
            }
        } else {
            std::cin.clear();
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid row! Enter a value from 0 to "
                  << FARM_SIZE - 1 << ".\n";
    }

    while (true) {
        std::cout << "Enter column (0 to " << FARM_SIZE - 1 << "): ";

        if (std::cin >> col) {
            if (col >= 0 && col < FARM_SIZE) {
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
                break;
            }
        } else {
            std::cin.clear();
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
        std::cout << "Invalid column! Enter a value from 0 to "
                  << FARM_SIZE - 1 << ".\n";
    }
}
} // namespace

int main() {
    int day = 1;
    Tile farm[FARM_SIZE][FARM_SIZE];
    Inventory playerInventory;
    int money = 0;
    bool running = true;

    while (running) {
        std::cout << "==================== YOUR FARM ====================\n";
        std::cout << "Day: " << day << '\n';
        std::cout << "Seeds: " << playerInventory.GetSeedCount()
                  << " | Produce: " << playerInventory.GetProduceCount()
                  << " | Coins: " << money << "\n\n";

        for (int row = 0; row < FARM_SIZE; row++) {
            for (int col = 0; col < FARM_SIZE; col++) {
                if (!farm[row][col].hasCrop) {
                    std::cout << "🟫";
                } else if (farm[row][col].crop.GetStage() == Crop::Seed) {
                    std::cout << "🟤";
                } else if (farm[row][col].crop.GetStage() == Crop::Sprouted) {
                    std::cout << "🌱";
                } else if (farm[row][col].crop.GetStage() == Crop::Matured) {
                    std::cout << "🌿";
                } else if (farm[row][col].crop.GetStage() == Crop::Withered) {
                    std::cout << "🍂";
                }
            }
            std::cout << '\n';
        }

        std::cout << "\n============== Small Farming RPG ==============\n";
        std::cout << "1. Plant a crop (1 seed)\n";
        std::cout << "2. Water a crop\n";
        std::cout << "3. End day\n";
        std::cout << "4. Harvest\n";
        std::cout << "5. Sell produce (" << PRODUCE_SELL_PRICE << " coins each)\n";
        std::cout << "6. Buy a seed (" << SEED_PRICE << " coins each)\n";
        std::cout << "7. Inspect inventory\n";
        std::cout << "8. Quit\n";

        int choice;

        while (true) {
            std::cout << "Choose an action: ";

            if (std::cin >> choice) {
                if (choice >= 1 && choice <= 8) {
                    std::cin.ignore(
                        std::numeric_limits<std::streamsize>::max(), '\n'
                    );
                    break;
                }

                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
            } else {
                std::cin.clear();
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n'
                );
            }

            std::cout << "Invalid choice! Enter a value from 1 to 8.\n";
        }

        if (choice == 1) {
            int row;
            int col;
            GetCoordinates(row, col);

            Tile& selectedTile = farm[row][col];

            if (selectedTile.hasCrop) {
                std::cout << "That tile is already occupied!\n";
            } else if (!playerInventory.RemoveSeed()) {
                std::cout << "You don't have any seeds. Buy one for "
                          << SEED_PRICE << " coins.\n";
            } else {
                selectedTile.crop = Crop{};
                selectedTile.hasCrop = true;

                std::cout << "Your seed has been planted! Seeds remaining: "
                          << playerInventory.GetSeedCount() << '\n';
            }
        } else if (choice == 2) {
            int row;
            int col;
            GetCoordinates(row, col);

            if (farm[row][col].hasCrop) {
                Crop::WaterResult waterResult = farm[row][col].crop.Water();

                if (waterResult == Crop::WaterResult::AlreadyWatered) {
                    std::cout << "You have already watered this crop!\n";
                } else if (waterResult == Crop::WaterResult::CropFinished) {
                    std::cout << "You can't water this crop anymore.\n";
                } else if (waterResult == Crop::WaterResult::Watered) {
                    std::cout << "Crop watered successfully.\n";
                }
            } else {
                std::cout << "Can't water an empty tile!\n";
            }
        } else if (choice == 3) {
            for (int row = 0; row < FARM_SIZE; row++) {
                for (int col = 0; col < FARM_SIZE; col++) {
                    if (farm[row][col].hasCrop) {
                        farm[row][col].crop.DayCheck();
                    }
                }
            }

            day++;
            std::cout << "Day " << day << " begins!\n";
        } else if (choice == 4) {
            int row;
            int col;
            GetCoordinates(row, col);

            Tile::HarvestResult harvestResult = farm[row][col].Harvest();

            if (harvestResult == Tile::HarvestResult::Harvested) {
                playerInventory.AddProduce();

                std::cout << "Harvested the crop! Produce: "
                          << playerInventory.GetProduceCount() << '\n';
            } else if (harvestResult == Tile::HarvestResult::Withered) {
                std::cout << "Crop has withered; can't harvest it.\n";
            } else if (harvestResult == Tile::HarvestResult::NotMature) {
                std::cout << "Crop is not mature enough to harvest.\n";
            } else if (harvestResult == Tile::HarvestResult::NoCrop) {
                std::cout << "No crop here!\n";
            }
        } else if (choice == 5) {
            if (playerInventory.RemoveProduce()) {
                money += PRODUCE_SELL_PRICE;

                std::cout << "Sold 1 produce for " << PRODUCE_SELL_PRICE
                          << " coins. Total coins: " << money << '\n';
            } else {
                std::cout << "No produce to sell. Coins remain at "
                          << money << ".\n";
            }
        } else if (choice == 6) {
            if (money >= SEED_PRICE) {
                money -= SEED_PRICE;
                playerInventory.AddSeed();

                std::cout << "Bought 1 seed. Seeds: "
                          << playerInventory.GetSeedCount()
                          << " | Coins: " << money << '\n';
            } else {
                std::cout << "Not enough coins to buy a seed. You need "
                          << SEED_PRICE << " coins; you have "
                          << money << ".\n";
            }
        } else if (choice == 7) {
            std::cout << "Inventory — Seeds: "
                      << playerInventory.GetSeedCount()
                      << " | Produce: "
                      << playerInventory.GetProduceCount()
                      << " | Coins: " << money << '\n';
        } else if (choice == 8) {
            std::cout << "========== Thanks for playing! ==========\n";
            running = false;
        }

        std::cout << '\n';
    }

    return 0;
}