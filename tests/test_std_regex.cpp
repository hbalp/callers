#include <string>
#include <iostream>
#include <regex>

//using namespace std;

int main(int argc, char * argv[]) 
{
  std::string test = "test replacing \"these characters\"";
  std::regex reg("[^\\w]+");
  std::string by("_");
  test = std::regex_replace(test, reg, by);
  std::cout << test << std::endl;
}

// Local Variables:
// compile-command: "clang++ -g -std=c++11 -o test_std_regex test_std_regex.cpp"
// End:
