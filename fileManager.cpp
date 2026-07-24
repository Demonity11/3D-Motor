#include "fileManager.h"
#include <filesystem>

std::vector<std::string> readFile(const std::string& filename)
{
	std::ifstream file{ "save/" + filename};

	if (!file.is_open())
	{
		std::cerr << "ERROR::FAILED_TO_OPEN_FILE\n";
		return {};
	}

	else
	{
		std::vector<std::string> output{};
		std::string line;

		while (std::getline(file, line))
		{
			std::cout << line << "\n";
			output.push_back(line);
		}

		return output;
	}

	return {};
}

int writeFile(const std::string& filename, const std::string& data)
{
    std::filesystem::path p(filename);

    if (p.extension() != ".geo") 
    {
        p += ".geo";
    }

    std::filesystem::path fullPath{ "save/" / p };

    std::cout << "Writing: " << fullPath.string() << "\n";

    std::ofstream file{ fullPath, std::ios::out | std::ios::trunc };

    if (!file.is_open())
    {
        std::cerr << "ERROR::FAILED_TO_OPEN <" << fullPath.string() << ">\n";
        return -1;
    }

    file << data;
    file.close();

    return 0;
}

int removeFile(const std::string& filename)
{
    std::filesystem::path p(filename);

    if (p.extension() != ".geo")
    {
        p += ".geo";
    }

    std::filesystem::path fullPath{ "save/" / p };

    try 
    {
        if (std::filesystem::remove(fullPath)) 
        {
            std::cout << "File deleted successfully.\n";
        }
        else 
        {
            std::cout << "File not found.\n";
            return -1;
        }
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "ERROR::FILESYSTEM::" << e.what() << "\n";
        return -1;
    }

    return 0;
}