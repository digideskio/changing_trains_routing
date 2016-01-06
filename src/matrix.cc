#include "cgi-helper.h"


#include <fstream>
#include <iostream>
#include <map>
#include <string>


unsigned int get_station_id(const std::string station_name)
{
  if (station_name == "")
    return 0;
  
  unsigned int line_count = 0;
  std::ifstream station_list("../data/station_list.txt");
  do
  {
    ++line_count;
    std::string entry;
    getline(station_list, entry);
    if (entry == station_name)
      return line_count;
  }
  while (station_list.good());
  return 0;
}


int main(int argc, char* args[])
{
  std::cout<<"Content-type: application/json\n\n";
  
  std::map< std::string, std::string > fields = decode_cgi_to_plain(cgi_get_to_text());
  std::map< std::string, std::string >::const_iterator name_it = fields.find("name");
  if (name_it == fields.end())
  {
    std::cout<<"{"
      "\"error\":\"No name found in request.\""
    "}\n";
    return 0;
  }
  
  unsigned int station_id = get_station_id(name_it->second);
  
  if (station_id == 0)
  {
    std::cout<<"{"
      "\"name\":\""<<name_it->second<<"\","
      "\"error\":\"No station has this name.\""
    "}\n";
    return 0;
  }
  
  std::cout<<"{"
    "\"name\":\""<<name_it->second<<"\","
    "\"station_id\":\""<<station_id<<"\""
  "}\n";
  return 0;
}
