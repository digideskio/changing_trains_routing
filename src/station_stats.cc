
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


struct Station_Stats
{
  std::string name;
  unsigned int num_with_coords;
  unsigned int num_found;
  unsigned int num_routable;
  unsigned int num_defunct;
};


unsigned int count_stations()
{
  unsigned int line_count = 0;
  std::ifstream station_list("../data/station_list.txt");
  do
  {
    ++line_count;
    std::string entry;
    std::getline(station_list, entry);
  }
  while (station_list.good());
  return line_count - 1;
}


Station_Stats read_stations_stats(unsigned int id)
{
  std::ostringstream filename;
  filename<<"../station_"<<id<<"/stats.tsv";
  
  Station_Stats result;
  std::ifstream station_stats(filename.str().c_str());
  
  unsigned int ignored;
  station_stats>>ignored;
  station_stats>>ignored;
  station_stats>>ignored;
  station_stats>>ignored;
  station_stats>>result.num_with_coords;
  station_stats>>result.num_found;
  station_stats>>result.num_routable;
  station_stats>>result.num_defunct;
  station_stats>>ignored;
  std::getline(station_stats, result.name);
  
  unsigned int i = 0;
  while (i < result.name.size() && isspace(result.name[i]))
    ++i;
  result.name = result.name.substr(i);
  
  return result;
}


int main(int argc, char* args[])
{
  unsigned int num_stations = count_stations();

  std::cout<<"Content-type: application/json\n\n"
  "{"
    "\"stations\":[";
  
  bool comma = false;
  for (unsigned int i = 1; i <= num_stations; ++i)
  {
    if (comma)
      std::cout<<",";
    else
      comma = true;
    
    Station_Stats station = read_stations_stats(i);
    std::cout<<"{"
      "\"name\":\""<<station.name<<"\","
      "\"stats\":["
      <<station.num_with_coords<<","
      <<station.num_found<<","
      <<station.num_routable<<","
      <<station.num_defunct<<"]"
      "}";
  }
    
  std::cout<<"]"
  "}\n";

  return 0;
}