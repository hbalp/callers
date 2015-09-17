
WARNING: pour compiler ou analyser le code de cet exemple, 
on ne devrait pas en principe avoir besoin de l'option -std=c++11

Cependant si cette option n'est pas activée, alors certaines définitions 
template ne sont pas visitées, comme basic_string() définie dans
include/c++/4.8/bits/basic_string.tcc:227
Conséquence: le graphe d'appel de fonction généré est incomplet

Cf le premier test: source test_std_map_launch.sh

Mais si j'active l'option c++11,
alors le plugin callers++ plante lorsqu'il essaie d'ouvrir le fichier json en écriture
avec la fonction std::fstream::open() dans CallersData::JsonFileWriter::JsonFileWriter(std::string jsonFileName)

Cf le test: source test_std_map_launch_c++11.sh

Même constat si j'essaie sans option c++11, mais
en ajoutant explicitement dans le code à compiler les deux lignes suivantes:

   template class std::basic_string<char>;
   template class std::map<std::string, int>;

Cf le test: source test_std_map1_launch.sh

