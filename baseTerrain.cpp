#include "baseTerrain.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <cstddef>

namespace lve {

BaseTerrain::BaseTerrain(const std::string &filepath) {
    readFile(filepath);
};
void BaseTerrain::readFile(const std::string &filepath) {
    std::ifstream file{filepath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filepath);
    }
    // Read from the end of the file to get its size
    // The reason of using static_cast is making the typecasting safer and readable.
    // The reason of using size_t instead of unsigned int is that size_t is platform independent.
    // size of int may vary from platform to platform.
    // but size_t is always unsigned and is guaranteed to be big enough to contain the size of the biggest object the host system can handle which is 64-bit for a 64-bit system.
    // reason of using tellg() is that it returns the position of the current character in the input stream.
    // since we are already at the end of the file, we can use this to get the size of the file.
    // by using std::ios::ate, we are setting the position of the input stream to the end of the file.
    size_t fileSize = static_cast<size_t>(file.tellg());
    size_t floatCount = fileSize / sizeof(float);

    std::vector<float> buffer(floatCount);

    // seekg() is used to set the position of the input stream to the beginning of the file.
    file.seekg(0);

    // Vector.data() returns a pointer to the first element of the vector.
    //file.read(buffer.data(), fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    terrainSize = static_cast<size_t>(sqrt(floatCount));
    terrainData.resize(terrainSize, std::vector<float>(terrainSize));

    std::cout << sqrt(buffer.size()) << std::endl;
    terrainSize = sqrt(buffer.size());

    //terrainData.resize(terrainSize, std::vector<char>(terrainSize));
    for (size_t i = 0; i < terrainSize; ++i) {
        for (size_t j = 0; j < terrainSize; ++j) {
            terrainData[i][j] = buffer[i * terrainSize + j];
        }
    }
    std::cout<<"terrain Size: "<<terrainData.size() << std::endl;
}

} // namespace lve
