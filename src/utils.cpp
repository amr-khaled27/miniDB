#include "HelperFunctions.hpp"
#include <iostream>
#include <vector>

std::vector<int> split(std::string timestamp) {
    // format "dd:mm:yyyy ss:mm:hh (pm/am)"
    // "23:04:2026 07:30:00 PM"
    std::cout << "Split Helper Hit\n";
    std::vector<int> result;
    std::string temp;
    bool am = false;
    for (char c : timestamp)  {
        if ((c == ':' || c == ' ') && !temp.empty()) {
            result.push_back(std::stoi(temp));
            temp = "";
        } else if (c >= '0' && c <= '9') {
            temp += c;
        } else if (c == 'a' || c == 'A'){
            am = true;
            break;
        } else if (c == 'p' || c == 'P') {
            break;
        }
    }

    if (am && result[3] == 12) {
        result[3] = 0;
    }
    if (!am && result[3] != 12) {
        result[3] += 12;
    }

    return result;
}

int timestampToUnix(std::string timestamp) {
    int sum = 0;
    std::vector<int> format = split(timestamp);

    // Year
    int days = 0;
    for (int i = 1970; i < format[2]; i++) {
        if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0)) {
            days += 366;
        } else {
            days += 365;
        }
    }
    std::cout<<days;
    return sum;
}
