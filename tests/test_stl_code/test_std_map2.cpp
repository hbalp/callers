#include <iostream>
#include <map>

int main(int argc, char** argv) {
  std::map<int, std::string> amap;
  std::map<std::string, int> arevmap;

  amap.insert(std::make_pair(3, "toto"));
  amap.insert(std::make_pair(3, "tata"));
  amap.insert(std::make_pair(4, "titi"));
  amap.insert(std::make_pair(-1, "stop"));

  arevmap.insert(std::make_pair("toto", 3));
  arevmap.insert(std::make_pair("tata", 3));
  arevmap.insert(std::make_pair("titi", 4));
  arevmap.insert(std::make_pair("stop", -1));

  for (const auto& pair : amap)
     std::cout << pair.first << ' ' << pair.second << '\n';
  for (const auto& pair : arevmap)
     std::cout << pair.first << ' ' << pair.second << '\n';

  return 0;
}

