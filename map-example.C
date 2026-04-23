// Requirement put a string "k1=v1,k2=v2,k3=v3,k4=v4,..." into std::map<string, string>
// Compile: g++ -std=c++17 -Wall -Wextra -O2 map-example.C

#include <iostream>
#include <map>

// We can use std::multimap if needed
using strMap = std::map<std::string, std::string>;

/*
 * inputStr contains key-value pairs separated by commas
 * For example: "k1=v1,k2=v2,k3=v3,k4=v4,..."
 * Output: map =  { k1 : v1,
                    k2 : v2,
                    k3 : v3,
                    k4 : v4
                  }
 */
int strToMap(const std::string inputStr, strMap& map)
{
  if (inputStr.empty())
  {
    return -1;
  }

  map.clear();
  size_t start = 0, end = 0;
  do {
    end = inputStr.find(',', start);
    const auto keyVal = inputStr.substr(start, end - start); // string before ','

    const auto pos = keyVal.find('=');
    if (pos != std::string::npos)
    {
      const auto key = keyVal.substr(0, pos);
      const auto value = keyVal.substr(pos+1);
      //std::cout << key << " = " << value << std::endl;
      if (!key.empty())
      {
        map[key] = value;
      }
    }
    start = end + 1;
  } while (end != std::string::npos);

  return 0;
}

int main()
{
  strMap map;
  strToMap("k1=v1,k2=v2,k3=v3,k4=v4,X=", map);

  for (const auto& [key, value] : map)
  {
    std::cout << key << " : " << value << std::endl;
  }

  return 0;
}

