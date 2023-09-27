#include "lve_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve
{

    LvePipeline::LvePipeline(const std::string &vertFilepath, const std::string &fragFilepath)
    {
        createGraphicsPipeline(vertFilepath, fragFilepath);
    }

    std::vector<char> LvePipeline::readFile(const std::string &filepath)
    {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
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

        std::vector<char> buffer(fileSize);

        // seekg() is used to set the position of the input stream to the beginning of the file.
        file.seekg(0);

        // Vector.data() returns a pointer to the first element of the vector.
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
    void LvePipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath)
    {
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        std::cout << "Vertex Shader Code Size: " << vertCode.size() << std::endl;
        std::cout << "Fragmant Shader Code Size: " << fragCode.size() << std::endl;
    }
}