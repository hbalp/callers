
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# add_executable(test_std_set
#   test_std_set.cpp)

add_executable(test_std_map
  test_std_map.cpp)

# WARNING: pour compiler le code de cet exemple, 
# on ne devrait pas en principe avoir besoin de l'option -std=c++11
# Cependant si cette option n'est pas activée, alors certaines définitions 
# template ne sont pas visitées, comme basic_string() définie dans
# include/c++/4.8/bits/basic_string.tcc:227
# Conséquence: le graphe d'appel de fonction généré est incomplet
# set_target_properties(test_std_map
#   PROPERTIES
#   COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11"
# )
#
# ERROR1: mais si j'active l'option c++11,
# alors le plugin callers++ plante lorsqu'il essaie d'ouvrir le fichier json en écriture
# avec la fonction out.open() dans CallersData::JsonFileWriter::JsonFileWriter(std::string jsonFileName)
#
# ERROR2: même constat si j'essaie sans option c++11, mais
# en ajoutant explicitement dans le code à compiler les deux lignes suivantes:
#
#    template class std::basic_string<char>;
#    template class std::map<std::string, int>;
#

add_executable(test_std_map1
  test_std_map1.cpp)

add_executable(test_std_map2
  test_std_map2.cpp)

add_executable(test_std_map3
  test_std_map3.cpp)

set_target_properties(test_std_map2 test_std_map3
  PROPERTIES
  COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11"
)
