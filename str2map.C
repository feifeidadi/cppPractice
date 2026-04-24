// Requirement put a string "k1=v1,k2=v2,k3=v3,k4=v4,..." into std::map<string, string>
// Compile: g++ -std=c++17 -Wall -Wextra -O2 str2map.C -o str2map

#include <iostream>
#include <map>
#include <boost/tokenizer.hpp>

// We can use std::multimap if needed
using strMap = std::map<std::string, std::string>;

/*
 * inputStr contains key-value pairs separated by commas
 * For example: "k1=v1,k2=v2,k3=v3,k4=v4,..."
 * Output: map =  { k1 : v1,
 *                  k2 : v2,
 *                  k3 : v3,
 *                  k4 : v4
 *                }
 */
void strToMap(const std::string inputStr, strMap& map)
{
  map.clear();
  const boost::char_separator<char> sep(",");
  boost::tokenizer<boost::char_separator<char>> tokens(inputStr, sep);
  for (const auto& token : tokens)
  {
    const auto pos = token.find('=');
    if (pos != std::string::npos)
    {
      map.emplace(token.substr(0, pos), token.substr(pos+1));
    }
  }
}

int main()
{
  strMap map;
  strToMap("k1=v1,k2=v2,k3=v3,k4=v4,X=,,,", map);
  for (const auto& [key, value] : map)
  {
    std::cout << key << " : " << value << std::endl;
  }

  strToMap("", map); // Map will be cleared if the input string is empty
  if (map.empty())
  {
    std::cout << "Empty map" << std::endl;
  }

  return 0;
}

