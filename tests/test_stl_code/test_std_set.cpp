//#include <assert.h>
#include <set>
#include <iostream>
//#include <fstream>

int test_set_of_integers() {
  std::set<int> s; // équivaut à std::set<int, std::less<int> >
  s.insert(2); // s contient 2
  s.insert(5); // s contient 2 5
  s.insert(2); // le doublon n'est pas inséré
  s.insert(1); // s contient 1 2 5
  std::set<int>::const_iterator
    sit (s.begin()),
    send(s.end());
  for(;sit!=send;++sit) std::cout << *sit << ' ';
  std::cout << std::endl;
  return 0;
}

int test_set_of_strings() {

  std::set<std::string> s; // équivaut à std::set<std::string, std::less<std::string> >

  s.insert("z"); // s contient 2
  s.insert("a"); // s contient 2 5
  s.insert("a"); // le doublon n'est pas inséré
  s.insert("h"); // s contient 1 2 5

  std::set<std::string>::const_iterator
    sit (s.begin()),
    send(s.end());

  for(;sit!=send;++sit) std::cout << *sit << ' ';
  std::cout << std::endl;

  // look for "h"
  std::set<std::string>::iterator sh = s.find("h");
  if (sh != s.end())
    {
      // do something with *it
      std::cout << "found: " << *sh << std::endl;
    }
  return 0;
}

class Thing
{
public:
  Thing(std::string name) : name(name) {}
  std::string name;
  std::string other_stuff;
};

std::ostream &operator<<(std::ostream &output, const Thing &thing)
{
  output << thing.name << ' ';
  return output;
}

bool operator< (const Thing& thing1, const Thing& thing2)
{
  return thing1.name < thing2.name;
}

//int std::set<Thing>::find(const char& name);

int test_set_of_things() {

  std::set <Thing> s;
  //std::set <Thing, id_compare> s;

  s.insert(Thing("q"));
  s.insert(Thing("w"));
  s.insert(Thing("e"));
  s.insert(Thing("r"));
  s.insert(Thing("t"));
  s.insert(Thing("y"));

  s.insert(Thing("a"));
  s.insert(Thing("z"));
  s.insert(Thing("e"));
  s.insert(Thing("r"));
  s.insert(Thing("t"));
  s.insert(Thing("y"));

  std::set<Thing>::const_iterator
    sit (s.begin()),
    send(s.end());

  for(;sit!=send;++sit) std::cout << *sit << ' ';
  std::cout << std::endl;

  // look for "h"
  Thing w("w");
  std::set<Thing>::iterator sw = s.find(w);
  if (sw != s.end())
    {
      // do something with *it
      std::cout << "found: " << *sw << std::endl;
    }

  return 0;
}

int main()
{
  test_set_of_integers();
  test_set_of_strings();
  test_set_of_things();
}
