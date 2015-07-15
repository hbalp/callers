#include <climits>
#include <iostream>
#include <boost/regex.hpp>

using namespace boost;
using namespace std;

int main()
{
  regex re("ba(\\w+)s");
  string s("monkeys like bananas");
  smatch m;
  
  if(regex_search(s, m, re)) {
    cout << "whole match: " << m[0] << endl
	 << "first group: " << m[1] << endl;
  }
}

// Local Variables:
// compile-command: "g++ -Wall -Wextra -pedantic -std=c++98 -O2 test_boost_regex.cpp -lboost_regex -o test_boost_regex && ./test_boost_regex"
// End:
