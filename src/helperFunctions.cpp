#include "HelperFunctions.hpp"

std::vector<int> split(std::string timestamp) {
    // format "dd:mm:yyyy hh:mm:ss (pm/am)"
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

long long timestampToUnix(const std::string& timestamp) {
    std::vector<int> parts = split(timestamp);
    if (parts.size() != 6) return -1;
    
    // parts: [day, month, year, hour, minute, second]
    struct tm t = {0};
    t.tm_year = parts[2] - 1900;  // years since 1900
    t.tm_mon = parts[1] - 1;       // months 0-11
    t.tm_mday = parts[0];          // day of month
    t.tm_hour = parts[3];          // hours
    t.tm_min = parts[4];           // minutes
    t.tm_sec = parts[5];           // seconds
    
    // mktime handles ALL calendar complexity: leap years, month lengths, everything
    return timegm(&t);  // timegm = UTC version of mktime
}