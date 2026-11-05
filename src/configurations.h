#pragma once
#include <istream>
#include <string>
#include <vector>

// UTF-8 text: one name per line. Empty lines and # comments are ignored.
inline std::vector<std::string> ReadConfigurations(std::istream& input) {
    std::vector<std::string> names;
    std::string line;
    bool firstLine = true;
    while (std::getline(input, line)) {
        if (firstLine && line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3);
        firstLine = false;
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == '#') continue;
        const auto last = line.find_last_not_of(" \t\r");
        names.push_back(line.substr(first, last - first + 1));
    }
    return names;
}
