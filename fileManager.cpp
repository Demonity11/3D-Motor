#include "fileManager.h"
#include <filesystem>
#include "lexer.h"
#include "Context.h"
#include "imgui/imgui.h"

std::vector<std::string> readFile(const std::string& filename)
{
	std::ifstream file{ "save/" + filename};

	if (!file.is_open())
	{
        Toast toast
        {
            "Read File Error",
            "Read File Error: Failed to open file '" + filename + "'.",
            ImColor{ 255, 0, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);
		return {};
	}

	else
	{
		std::vector<std::string> output{};
		std::string line;

		while (std::getline(file, line))
        {
			output.push_back(line);
		}

        Toast toast
        {
            "Read File",
            "Read File: File '" + filename + "' was read successfully.",
            ImColor{ 0, 255, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);
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

    std::ofstream file{ fullPath, std::ios::out | std::ios::trunc };

    if (!file.is_open())
    {
        Toast toast
        {
            "Write File Error",
            "Write File Error: Failed to open file '" + filename + "'.",
            ImColor{ 255, 0, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);
        return -1;
    }

    file << data;
    file.close();

    Toast toast
    {
        "Write File",
        "Write File: File '" + filename + "' was written successfully.",
        ImColor{ 0, 255, 0, 255 },
        Context::defaultToastDuration,
        Context::defaultToastDuration
    };

    Context::toastNotifications.push_back(toast);
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
            Toast toast
            {
                "Delete File",
                "Delete File: File '" + filename + "' was deleted successfully.",
                ImColor{ 0, 255, 0, 255 },
                Context::defaultToastDuration,
                Context::defaultToastDuration
            };

            Context::toastNotifications.push_back(toast);
        }
        else 
        {
            Toast toast
            {
                "Delete File",
                "Delete File Error: File '" + filename + "' was not found.",
                ImColor{ 255, 0, 0, 255 },
                Context::defaultToastDuration,
                Context::defaultToastDuration
            };

            Context::toastNotifications.push_back(toast);
            return -1;
        }
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        Toast toast
        {
            "File System Error",
            "File System Error: " + std::string(e.what()),
            ImColor{ 0, 255, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);
        return -1;
    }

    return 0;
}

bool validateFileName(const std::string& filename)
{
    if (filename.empty())
    {
        Toast toast
        {
            "Filename Error",
            "Filename Error: Filename is empty.",
            ImColor{ 255, 0, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);
        return false;
    }

    if (!isAlpha(filename[0]))
    {
        Toast toast
        {
            "Filename Error",
            "Filename Error: Filename '" + filename + "' must start with an alpha character.",
            ImColor{ 255, 0, 0, 255 },
            Context::defaultToastDuration,
            Context::defaultToastDuration
        };

        Context::toastNotifications.push_back(toast);

        return false;
    }

    for (char c : filename)
    {
        if (!isAlnum(c))
        {
            Toast toast
            {
                "Filename Error",
                "Filename Error: Filename '" + filename + "' contain one or more characters that are not alphanumeric.",
                ImColor{ 255, 0, 0, 255 },
                Context::defaultToastDuration,
                Context::defaultToastDuration
            };

            Context::toastNotifications.push_back(toast);
            return false;
        }
    }

    const std::string filenamePlusExtension{ filename + ".geo" };
    for (const auto& entry : std::filesystem::directory_iterator("save/"))
    {
        if (filenamePlusExtension == entry.path().filename().string())
        {
            Toast toast
            {
                "Filename Error",
                "Filename Error: Filename '" + filename + "' already exist.",
                ImColor{ 255, 0, 0, 255 },
                Context::defaultToastDuration,
                Context::defaultToastDuration
            };

            Context::toastNotifications.push_back(toast);

            return false;
        }
    }

    return true;
}