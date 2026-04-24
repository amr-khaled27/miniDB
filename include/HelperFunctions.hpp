#pragma once
#include <string>
#include <vector>
#include <iostream>

long long timestampToUnix(const std::string&);

std::vector<int> split(std::string timestamp);

long long dateToDays(int year, int month, int day);