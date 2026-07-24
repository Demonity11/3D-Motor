#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> readFile(const std::string& filename);
int writeFile(const std::string& filename, const std::string& data);
int removeFile(const std::string& filename);
bool validateFileName(const std::string& filename);

#endif
