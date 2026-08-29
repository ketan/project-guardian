#pragma once

#include <string>

class String : public std::string {
public:
    using std::string::string;
    using std::string::operator=;

    bool isEmpty() const { return empty(); }
};
