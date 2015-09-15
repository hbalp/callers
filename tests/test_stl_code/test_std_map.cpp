#include <map>
#include <string>
#include <iostream>

// from http://stackoverflow.com/questions/8602068/whats-the-difference-between-stdmultimapkey-value-and-stdmapkey-stds
void someFunction()
{
  typedef std::map<std::string, int> MapType;
  MapType myMap;

  // insertion
  myMap.insert(MapType::value_type("test", 42));
  myMap.insert(MapType::value_type("other-test", 0));

  // search
  std::map<std::string, int>::iterator /*auto*/ it = myMap.find("test");
  if (it != myMap.end())
    std::cout << "value for " << it->first << " is " << it->second << std::endl;
  else
    std::cout << "value not found" << std::endl;
}

int main()
{
  someFunction();
  return 0;
}
