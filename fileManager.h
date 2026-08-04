#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

auto readFile(const std::string& filename)							 -> std::vector<std::string>;
auto writeFile(const std::string& filename, const std::string& data) -> int;
auto removeFile(const std::string& filename)						 -> int;
auto validateFileName(const std::string& filename)					 -> bool;

#endif
