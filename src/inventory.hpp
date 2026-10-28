#pragma once

class Inventory {
private:
    int produceCount = 0;
    int seedCount = 3;

public:
    void AddProduce() {
        produceCount++;
    }

    int GetProduceCount() const {
        return produceCount;
    }

    bool RemoveProduce() {
        if (produceCount > 0) {
            produceCount--;
            return true;
        }

        return false;
    }

    void AddSeed() {
        seedCount++;
    }

    int GetSeedCount() const {
        return seedCount;
    }

    bool RemoveSeed() {
        if (seedCount > 0) {
            seedCount--;
            return true;
        }

        return false;
    }
};
