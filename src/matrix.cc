#include "cgi-helper.h"
#include "dijkstra.h"
#include "read_input.h"


#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
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


std::vector< Route_Ref > build_destinations(const Parsing_State& data, const Routing_Data& routing_data)
{
  std::vector< Route_Ref > result;
  
  for (std::vector< Way >::const_iterator it = data.ways.begin(); it != data.ways.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      Geometry geom(*it, data);
      Coord center = geom.bbox_center();
      std::string label = "-";
      unsigned int label_level = 0;
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
      {
	if (it2->first == "local_ref")
	{
	  label = it2->second;
	  label_level = 3;
	}
	else if (label_level < 2 && it2->first == "ref")
	{
	  label = it2->second;
	  label_level = 2;
	}
	else if (label_level < 1 && it2->first == "name")
	{
	  label = it2->second;
	  label_level = 1;
	}
      }
      Way_Reference ref(*it, center, data);
      result.push_back(Route_Ref(routing_data, ref, label, data));
    }
  }
  
  for (std::vector< Relation >::const_iterator it = data.relations.begin(); it != data.relations.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      Geometry geom(*it, data);
      Coord center = geom.bbox_center();
      std::string label = "-";
      unsigned int label_level = 0;
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
      {
	if (it2->first == "local_ref")
	{
	  label = it2->second;
	  label_level = 3;
	}
	else if (label_level < 2 && it2->first == "ref")
	{
	  label = it2->second;
	  label_level = 2;
	}
	else if (label_level < 1 && it2->first == "name")
	{
	  label = it2->second;
	  label_level = 1;
	}
      }
      Way_Reference ref(*it, center, data);
      result.push_back(Route_Ref(routing_data, ref, label, data));
    }
  }
  
  return result;
}


Coord position_of_ref(const Route_Ref& ref)
{
  if (!ref.edge)
    return Coord(0, 0);
  
  Coord from = ref.edge->trace[ref.index];
  Coord to = ref.edge->trace[ref.index + 1];
  double edge_distance = distance(from, to);
  
  if (edge_distance == 0)
    return Coord(0, 0);
  
  Coord result(0, 0);
  result.lat = from.lat + (to.lat - from.lat) * ref.pos / edge_distance;
  result.lon = from.lon + (to.lon - from.lon) * ref.pos / edge_distance;
  return result;
}


std::string to_json(const Coord& coord)
{
  std::ostringstream result;
  result<<"{\"lat\":"<<std::fixed<<std::setprecision(7)<<coord.lat
      <<",\"lon\":"<<std::fixed<<std::setprecision(7)<<coord.lon<<"}";
  return result.str();
}


std::string full_trace(const Route& route)
{
  std::string result = to_json(position_of_ref(route.start));
  
  if (route.edges.size() > 1)
  {
    Coord last_coord(0, 0);
    if (route.edges[0]->trace.back() == route.edges[1]->trace.front()
        || route.edges[0]->trace.back() == route.edges[1]->trace.back())
    {
      for (unsigned int i = route.start.index + 1; i < route.edges[0]->trace.size(); ++i)
	result += "," + to_json(route.edges[0]->trace[i]);
      last_coord = route.edges[0]->trace.back();
    }
    else
    {
      for (int i = route.start.index; i >= 0; --i)
	result += "," + to_json(route.edges[0]->trace[i]);
      last_coord = route.edges[0]->trace.front();
    }
    
    for (unsigned int j = 1; j < route.edges.size()-1; ++j)
    {
      if (route.edges[j]->trace.front() == last_coord)
      {
        for (unsigned int i = 1; i < route.edges[j]->trace.size(); ++i)
	  result += "," + to_json(route.edges[j]->trace[i]);
        last_coord = route.edges[j]->trace.back();
      }
      else
      {
        for (int i = route.edges[j]->trace.size()-2; i >= 0; --i)
	  result += "," + to_json(route.edges[j]->trace[i]);
        last_coord = route.edges[j]->trace.front();
      }
    }
    
    unsigned int j = route.edges.size()-1;    
    if (route.edges[j]->trace.front() == last_coord)
    {
      for (unsigned int i = 1; i < route.end.index; ++i)
	result += "," + to_json(route.edges[j]->trace[i]);
    }
    else
    {
      for (unsigned int i = route.edges[j]->trace.size()-2; i > route.end.index; --i)
	result += "," + to_json(route.edges[j]->trace[i]);
    }
  }
  else if (!route.edges.empty())
  {
    const Routing_Edge& edge = *route.edges[0];
    if (route.start.index < route.end.index)
    {
      for (unsigned int i = route.start.index + 1; i <= route.end.index; ++i)
	result += "," + to_json(edge.trace[i]);
    }
    else if (route.start.index > route.end.index)
    {
      for (unsigned int i = route.end.index; i > route.start.index; --i)
	result += "," + to_json(edge.trace[i]);
    }
  }
  
  result += "," + to_json(position_of_ref(route.end));
  return result;
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
  
  std::ostringstream filename;
  filename<<"../station_"<<station_id<<"/data.osm";
  
  const Parsing_State& state = read_osm(filename.str());
  Routing_Data routing_data(state);
  
  std::vector< Route_Ref > destinations = build_destinations(state, routing_data);
  
  std::cout<<"{"
    "\"name\":\""<<name_it->second<<"\","
    "\"station_id\":\""<<station_id<<"\","
    "\"gates\":[";
  
  for (std::vector< Route_Ref >::const_iterator it = destinations.begin(); it != destinations.end(); ++it)
  {
    if (it != destinations.begin())
      std::cout<<",";
    Coord dest_pos = position_of_ref(*it);
    std::cout<<"{"
      "\"ref\":\""<<it->label<<"\","
      "\"lat\":"<<std::fixed<<std::setprecision(7)<<dest_pos.lat<<","
      "\"lon\":"<<std::fixed<<std::setprecision(7)<<dest_pos.lon<<","
      "\"connections\":[";
      
    Route_Tree tree(routing_data, *it, destinations);
    for (std::vector< Route >::const_iterator r_it = tree.routes.begin(); r_it != tree.routes.end(); ++r_it)
    {
      if (r_it != tree.routes.begin())
	std::cout<<",";
      std::cout<<"{"
        "\"to\":\""<<r_it->end.label<<"\","
	"\"cost\":\""<<std::fixed<<std::setprecision(7)<<r_it->value<<"\","
	"\"trace\":["<<full_trace(*r_it)<<"]}";
    }

    std::cout<<"]}";
  }
  std::cout<<"]"
  "}\n";
  return 0;
}
