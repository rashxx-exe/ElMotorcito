#pragma once
#include <string>
#include <map>

class json
{
public:
    std::map<std::string, std::string> values;

    std::string& operator[](const std::string& key)
    {
        return values[key];
    }

    std::string dump(int indent = 0) const
    {
        std::string out = "{\n";
        for (auto it = values.begin(); it != values.end(); ++it)
        {
            out += "  \"" + it->first + "\": \"" + it->second + "\"";
            if (std::next(it) != values.end()) out += ",";
            out += "\n";
        }
        out += "}";
        return out;
    }
};