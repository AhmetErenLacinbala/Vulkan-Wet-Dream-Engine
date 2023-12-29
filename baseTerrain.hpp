#pragma once

#include <string>

namespace lve{
    class BaseTerrain
    {
        public:
        BaseTerrain(const std::string &filepath);
        std::vector<std::vector<float>> terrainData;

        private:
        //void populateTerrainData (std::vector <char>& terrain);
        std::string filepath;
        void readFile(const std::string &filepath);
        uint32_t terrainSize = 0;
    };

}