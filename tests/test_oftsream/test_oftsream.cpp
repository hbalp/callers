
#include <fstream>
#include <iostream>
#include <map>
#include <string>

// template class std::basic_string<char>;
// template class std::map<std::string, int>;

int main()
{
  std::string jsonFileName = "my_new_file.gen.json";
  //std::string jsonFileName = "/home/hbalp/work/src/llvm/trunk/tools/clang/tools/extra/callers/tests/test_stl_code/test_std_map.cpp.file.callers.gen.json";
  std::ofstream out;

  std::cout << "Open file \"" << jsonFileName << "\" in write mode." << std::endl;
  out.open(jsonFileName.c_str());
  if(out.fail())
    {
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      std::cerr << "Failed to open file \"" << jsonFileName << "\" in write mode." << std::endl;
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
    }
  else
    {
      std::cerr << "JSON output file \"" << jsonFileName << "\" is now opened in write mode." << std::endl;
    }

  std::cout << "Close file \"" << jsonFileName << "\"." << std::endl;
  out.close();
  if(out.fail())
    {
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      std::cerr << "Failed to close file \"" << jsonFileName << "\"." << std::endl;
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
    }
  else
    {
      std::cerr << "JSON output file \"" << jsonFileName << "\" is now closed." << std::endl;
    }
  
  return 0;
}
