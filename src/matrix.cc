#include "cgi-helper.h"
#include "dijkstra.h"
#include "read_input.h"


#include <algorithm>
#include <cstdlib>
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
  
  double max_lat = -90.0;
  double min_lat = 90.0;
  double max_lon = -180.0;
  double min_lon = 180.0;
  for (std::vector< Node >::const_iterator it = data.nodes.begin(); it != data.nodes.end(); ++it)
  {
    max_lat = std::max(max_lat, it->lat);
    min_lat = std::min(min_lat, it->lat);
    max_lon = std::max(max_lon, it->lon);
    min_lon = std::min(min_lon, it->lon);
  }
  
  Id_Type north_node = 0;
  Id_Type south_node = 0;
  Id_Type west_node = 0;
  Id_Type east_node = 0;
  for (std::vector< Node >::const_iterator it = data.nodes.begin(); it != data.nodes.end(); ++it)
  {
    if (it->lat == max_lat)
      north_node = it->id;
    if (it->lat == min_lat)
      south_node = it->id;
    if (it->lon == max_lon)
      east_node = it->id;
    if (it->lon == min_lon)
      west_node = it->id;
  }
  
  for (std::vector< Way >::const_iterator it = data.ways.begin(); it != data.ways.end(); ++it)
  {
    for (std::vector< Id_Type >::const_iterator nds_it = it->nds.begin(); nds_it != it->nds.end(); ++nds_it)
    { 
      if (*nds_it == north_node)
      {
        Way_Reference ref(*it, std::distance(it->nds.begin(), nds_it), data);
        result.push_back(Route_Ref(routing_data, ref, "northern perimeter", data));	
	north_node = 0;
      }
      if (*nds_it == south_node)
      {
        Way_Reference ref(*it, std::distance(it->nds.begin(), nds_it), data);
        result.push_back(Route_Ref(routing_data, ref, "southern perimeter", data));	
	south_node = 0;
      }
      if (*nds_it == east_node)
      {
        Way_Reference ref(*it, std::distance(it->nds.begin(), nds_it), data);
        result.push_back(Route_Ref(routing_data, ref, "eastern perimeter", data));	
	east_node = 0;
      }
      if (*nds_it == west_node)
      {
        Way_Reference ref(*it, std::distance(it->nds.begin(), nds_it), data);
        result.push_back(Route_Ref(routing_data, ref, "western perimeter", data));	
	west_node = 0;
      }
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
  if (route.value == Route::max_route_length)
    return "";
  
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


bool node_is_elevator(Id_Type id, const Parsing_State& data)
{
  std::vector< Node >::const_iterator n_it =
      std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(id));
  if (n_it != data.nodes.end() && n_it->id == id)
    return (has_kv(*n_it, "highway", "elevator"));
  else
    return false;
}


struct Distance_Profile : public Routing_Profile
{
  virtual bool is_routable(const Way& way) const { return true; }
  virtual bool is_routable(const Node& node) const { return true; }
  
  virtual double valuation_factor(const Way& way) const { return 40000000. / 360.; }
  virtual double node_penalty(const Node& node) const { return 0.; }
};


struct Travel_Time_Profile : public Routing_Profile
{
  Travel_Time_Profile(double footway_, double platform_, double stairs_)
      : footway(footway_), platform(platform_), stairs(stairs_) {}
  
  virtual bool is_routable(const Way& way) const;
  virtual bool is_routable(const Node& node) const;
  
  virtual double valuation_factor(const Way& way) const;
  virtual double node_penalty(const Node& node) const;
  
  double footway;
  double platform;
  double stairs;
  
  std::vector< Id_Type > blocked_nodes;
};


bool Travel_Time_Profile::is_routable(const Way& way) const
{
  if (has_kv(way, "highway", "steps"))
    return (stairs > 0);
  else if (has_k(way, "highway"))
    return (footway > 0);
  return (platform > 0);
}


double Travel_Time_Profile::valuation_factor(const Way& way) const
{
  if (has_kv(way, "highway", "steps"))
    return 40000000. / 360. / stairs;
  else if (has_k(way, "highway"))
    return 40000000. / 360. / footway;
  return 40000000. / 360. / platform;
}


bool Travel_Time_Profile::is_routable(const Node& node) const
{
  std::vector< Id_Type >::const_iterator it = std::lower_bound(blocked_nodes.begin(), blocked_nodes.end(), node.id);
  return !(it != blocked_nodes.end() && *it == node.id);
}


double Travel_Time_Profile::node_penalty(const Node& node) const
{
  if (has_kv(node, "highway", "elevator"))
    return 2.;
  return 0.;
}


std::vector< Node > find_osm_elevators(const std::vector< Node >& nodes)
{
  std::vector< Node > result;
  
  for (std::vector< Node >::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    if (has_kv(*it, "highway", "elevator"))
      result.push_back(*it);
  }
  
  return result;
}


struct Expected_Elevator
{
  enum State { not_monitored, unknown, active, inactive };
  
  Expected_Elevator() : node_ref(0), lat(100.0), lon(200.0), state(not_monitored) {}
  
  Id_Type node_ref;
  double lat;
  double lon;
  std::string ref;
  State state;
};


std::vector< Expected_Elevator > construct_expected_elevators(
    unsigned int station_id, const std::vector< Node >& osm_elevators,
    const std::map< std::string, std::string >& elevator_states)
{
  std::vector< Expected_Elevator > result;
  
  std::ostringstream filename;
  filename<<"../station_"<<station_id<<"/elevators.tsv";
  std::ifstream elevator_file(filename.str().c_str());
    
  std::string buffer;
  std::getline(elevator_file, buffer);
  while (elevator_file.good())
  {    
    Expected_Elevator elevator;
    
    std::string::size_type start_pos = 0;
    std::string::size_type pos = buffer.find('\t');
    if (pos == std::string::npos)
      pos = buffer.size();
    
    if (start_pos < buffer.size())
    {
      elevator.ref = buffer.substr(start_pos, pos - start_pos);
      start_pos = (pos == buffer.size() ? pos : pos + 1);
      pos = (start_pos == buffer.size() ? start_pos : buffer.find('\t', start_pos));
      if (pos == std::string::npos)
	pos = buffer.size();
    }
    
    if (start_pos < buffer.size())
    {
      if (pos > start_pos + 1 && (isdigit(buffer[start_pos]) || (buffer[start_pos] == '-')))
        elevator.lon = atof(buffer.substr(start_pos, pos - start_pos).c_str());
      start_pos = (pos == buffer.size() ? pos : pos + 1);
      pos = (start_pos == buffer.size() ? start_pos : buffer.find('\t', start_pos));
      if (pos == std::string::npos)
	pos = buffer.size();
    }
    
    if (start_pos < buffer.size())
    {
      if (pos > start_pos + 1 && (isdigit(buffer[start_pos]) || (buffer[start_pos] == '-')))
        elevator.lat = atof(buffer.substr(start_pos, pos - start_pos).c_str());
    }
    
    result.push_back(elevator);
    
    std::getline(elevator_file, buffer);
  }
  
  for (std::vector< Node >::const_iterator n_it = osm_elevators.begin(); n_it != osm_elevators.end(); ++n_it)
  {
    Coord node_pos(n_it->lat, n_it->lon);
    double min_distance = .0002;
    Expected_Elevator* closest = 0;
    
    for (std::vector< Expected_Elevator >::iterator e_it = result.begin();
	 e_it != result.end(); ++e_it)
    {
      if (e_it->lat != 100.0 && e_it->lon != 200.0)
      {
	double distance_ = distance(node_pos, Coord(e_it->lat, e_it->lon));
	if (distance_ < min_distance)
	{
	  min_distance = distance_;
	  closest = &*e_it;
	}
      }
    }
    
    if (closest && !closest->node_ref)
      closest->node_ref = n_it->id;
    else if (closest)
    {
      std::vector< Node >::const_iterator n_it =
          std::lower_bound(osm_elevators.begin(), osm_elevators.end(), Node(closest->node_ref));
      if (min_distance < distance(Coord(n_it->lat, n_it->lon), Coord(closest->lat, closest->lon)))
	closest->node_ref = n_it->id;
    }
  }
  
  for (std::vector< Expected_Elevator >::iterator e_it = result.begin();
      e_it != result.end(); ++e_it)
  {
    std::map< std::string, std::string >::const_iterator s_it = elevator_states.find(e_it->ref);
    if (s_it != elevator_states.end())
    {
      if (s_it->second == "ACTIVE")
	e_it->state = Expected_Elevator::active;
      else if (s_it->second == "INACTIVE")
	e_it->state = Expected_Elevator::inactive;
      else
	e_it->state = Expected_Elevator::unknown;
    }
  }
  
  return result;
}


std::map< std::string, std::string > load_elevator_states()
{
  std::map< std::string, std::string > result;
  
  std::ifstream in("../data/elevator_state.json");
  std::string json;
  std::getline(in, json);
  
  std::string::size_type pos = 0;
  std::string ref;
  std::string state;
  while (pos < json.size() && json[pos] != ']')
  {
    if (json[pos] == '}')
    {
      if (ref != "" && state != "")
	result[ref] = state;
      
      ref = "";
      state = "";
      ++pos;
    }
    else if (json[pos] == '"')
    {
      if (pos + 18 < json.size() && json.substr(pos, 18) == "\"equipmentnumber\":")
      {
	std::string::size_type end_pos = json.find(',', pos + 18);
	if (end_pos == std::string::npos)
	  end_pos = json.size();
	ref = json.substr(pos + 18, end_pos - pos - 18);
	pos = end_pos;
      }
      else if (pos + 8 < json.size() && json.substr(pos, 8) == "\"state\":")
      {
	std::string::size_type end_pos = json.find(',', pos + 8);
	if (end_pos == std::string::npos)
	  end_pos = json.size();
	state = json.substr(pos + 9, end_pos - pos - 10);
	pos = end_pos;
      }
      else
	++pos;
    }
    else
      ++pos;
  }
  
  return result;
}


bool operator<(const std::pair< Id_Type, bool >& lhs, const std::pair< Id_Type, bool >& rhs)
{
  return lhs.first < rhs.first;
}


int main(int argc, char* args[])
{
  std::map< std::string, std::string > fields = decode_cgi_to_plain(cgi_get_to_text());
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = args[i];
    if (arg.substr(0,2) == "--")
    {
      std::string::size_type pos = arg.find('=');
      if (pos != std::string::npos && pos < arg.size()-1)
	fields[arg.substr(2,pos-2)] = arg.substr(pos+1);
    }
  }
  
  std::map< std::string, std::string >::const_iterator output_it = fields.find("output");
  
  if (output_it == fields.end() || output_it->second == "json")
    std::cout<<"Content-type: application/json\n\n";
  
  unsigned int station_id = 0;
  std::string station_name;
 
  std::map< std::string, std::string >::const_iterator id_it = fields.find("id");
  if (id_it != fields.end())
    station_id = atol(id_it->second.c_str());
  
  if (station_id > 0)
  {
    std::ostringstream filename;
    filename<<"../station_"<<station_id<<"/name.txt";
    std::ifstream in(filename.str().c_str());
    
    std::getline(in, station_name);
  }
  else
  {
    std::map< std::string, std::string >::const_iterator name_it = fields.find("name");
    if (name_it == fields.end())
    {
      std::cout<<"{"
	"\"error\":\"No name found in request.\""
      "}\n";
      return 0;
    }
  
    station_name = name_it->second;
    station_id = get_station_id(station_name);
  }
  
  if (output_it != fields.end() && output_it->second == "id")
  {
    std::cout<<station_id<<'\n';
    return 0;
  }
  
  if (station_id == 0)
  {
    std::cout<<"{"
      "\"name\":\""<<station_name<<"\","
      "\"error\":\"No station has this name.\""
    "}\n";
    return 0;
  }
  
  std::ostringstream filename;
  filename<<"../station_"<<station_id<<"/data.osm";
  
  Routing_Profile* profile = 0;
  std::map< std::string, std::string >::const_iterator profile_it = fields.find("profile");
  if (profile_it == fields.end() || profile_it->second == "distance")
    profile = new Distance_Profile();
  else if (profile_it->second == "sport")
    profile = new Travel_Time_Profile(90, 60, 30);
  else if (profile_it->second == "luggage")
    profile = new Travel_Time_Profile(60, 40, 10);
  else if (profile_it->second == "wheelchair")
    profile = new Travel_Time_Profile(60, 40, 0);
  else
    profile = new Distance_Profile();
  
  const Parsing_State& state = read_osm(filename.str());
  
  std::vector< Node > osm_elevators = find_osm_elevators(state.nodes);
  std::map< std::string, std::string > elevator_states = load_elevator_states();
  std::vector< Expected_Elevator > expected_elevators
      = construct_expected_elevators(station_id, osm_elevators, elevator_states);
    
  Travel_Time_Profile* travel_time_profile = dynamic_cast< Travel_Time_Profile* >(profile);
  if (travel_time_profile)
  {
    for (std::vector< Expected_Elevator >::const_iterator it = expected_elevators.begin();
	 it != expected_elevators.end(); ++it)
    {
      if (it->node_ref && (it->state == Expected_Elevator::unknown || it->state == Expected_Elevator::inactive))
	travel_time_profile->blocked_nodes.push_back(it->node_ref);
    }
    std::sort(travel_time_profile->blocked_nodes.begin(), travel_time_profile->blocked_nodes.end());
  }
      
  Routing_Data routing_data(state, *profile);
  delete profile;
  
  std::vector< Route_Ref > destinations = build_destinations(state, routing_data);
  
  if (output_it == fields.end() || output_it->second == "json")
  {
    std::cout<<"{"
      "\"name\":\""<<station_name<<"\","
      "\"station_id\":\""<<station_id<<"\","
      "\"timestamp\":\""<<state.timestamp<<"\","
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
    std::cout<<"],"
      "\"elevators\":[";
    
    std::vector< Id_Type > expected_elevators_ids;
    for (std::vector< Expected_Elevator >::const_iterator ee_it = expected_elevators.begin();
	ee_it != expected_elevators.end(); ++ee_it)
      expected_elevators_ids.push_back(ee_it->node_ref);
    std::sort(expected_elevators_ids.begin(), expected_elevators_ids.end());
    
    bool comma = false;
    for (std::vector< Node >::const_iterator it = osm_elevators.begin(); it != osm_elevators.end(); ++it)
    {
      std::vector< Id_Type >::const_iterator ee_it
          = std::lower_bound(expected_elevators_ids.begin(), expected_elevators_ids.end(), it->id);
      if (ee_it == expected_elevators_ids.end() || *ee_it != it->id)
      {
        if (comma)
	  std::cout<<",";
        else
	  comma = true;
        std::cout<<to_json(Coord(it->lat, it->lon));
      }
    }
    
    std::cout<<"],"
      "\"expected_elevators\":[";
    
    std::vector< std::string > elevator_state_strings;
    elevator_state_strings.push_back("not monitored");
    elevator_state_strings.push_back("unknown");
    elevator_state_strings.push_back("active");
    elevator_state_strings.push_back("inactive");
        
    comma = false;
    for (std::vector< Expected_Elevator >::const_iterator it = expected_elevators.begin();
	 it != expected_elevators.end(); ++it)
    {
      if (comma)
	std::cout<<",";
      else
	comma = true;
      if (it->node_ref)
      {
        std::vector< Node >::const_iterator n_it =
          std::lower_bound(state.nodes.begin(), state.nodes.end(), Node(it->node_ref));
        std::cout<<"{"
	  "\"ref\":"<<it->ref<<","
	  "\"lat\":"<<n_it->lat<<","
	  "\"lon\":"<<n_it->lon<<","
	  "\"state\":\""<<elevator_state_strings[it->state]<<"\""
	"}";
      }
      else if (it->lat != 100.0 && it->lon != 200.0)
        std::cout<<"{"
	  "\"ref\":"<<it->ref<<","
	  "\"lat\":"<<it->lat<<","
	  "\"lon\":"<<it->lon<<","
	  "\"status\":\"no match\""
	"}";
      else
        std::cout<<"{"
	  "\"ref\":"<<it->ref<<","
	  "\"status\":\"without coordinates\""
	"}";
    }
    
    std::cout<<"]"
    "}\n";
  }
  else if (output_it->second == "stats")
  {
    unsigned int found_ways = 0;
    unsigned int ways_using_elevators = 0;
    
    for (std::vector< Route_Ref >::const_iterator it = destinations.begin(); it != destinations.end(); ++it)
    {
      Route_Tree tree(routing_data, *it, destinations);
      for (std::vector< Route >::const_iterator r_it = tree.routes.begin(); r_it != tree.routes.end(); ++r_it)
      {
	if (r_it->value != Route::max_route_length)
	  ++found_ways;

	bool uses_elevator = false;
	for (std::vector< const Routing_Edge* >::const_iterator edge_it = r_it->edges.begin();
	     edge_it != r_it->edges.end(); ++edge_it)
        {
	  if (node_is_elevator((*edge_it)->start->id, state))
	    uses_elevator = true;
	}
	if (uses_elevator)
	  ++ways_using_elevators;
      }
    }
    
    std::vector< std::pair< Id_Type, bool > > expected_elevators_ids;
    for (std::vector< Expected_Elevator >::const_iterator ee_it = expected_elevators.begin();
	ee_it != expected_elevators.end(); ++ee_it)
      expected_elevators_ids.push_back(std::make_pair(ee_it->node_ref, false));
    std::sort(expected_elevators_ids.begin(), expected_elevators_ids.end());
       
    for (std::vector< Route_Ref >::const_iterator it = destinations.begin(); it != destinations.end(); ++it)
    {
      Route_Tree tree(routing_data, *it, destinations);
      for (std::vector< Route >::const_iterator r_it = tree.routes.begin(); r_it != tree.routes.end(); ++r_it)
      {
	for (std::vector< const Routing_Edge* >::const_iterator e_it = r_it->edges.begin();
	    e_it != r_it->edges.end(); ++e_it)
	{
	  std::vector< std::pair< Id_Type, bool > >::iterator ee_it
	      = std::lower_bound(expected_elevators_ids.begin(), expected_elevators_ids.end(),
				 std::make_pair((*e_it)->end->id, false));
	  if (ee_it != expected_elevators_ids.end() && ee_it->first == (*e_it)->end->id)
	    ee_it->second = true;
	}
      }
    }
    
    unsigned int matched_elevators_count = 0;
    unsigned int elevators_with_coords_count = 0;
    for (std::vector< Expected_Elevator >::const_iterator it = expected_elevators.begin();
	 it != expected_elevators.end(); ++it)
    {
      if (it->node_ref)
      {
	++matched_elevators_count;
	++elevators_with_coords_count;
      }
      else if (it->lat != 100.0 && it->lon != 200.0)
	++elevators_with_coords_count;
    }
    
    unsigned int num_defunct_elevators = 0;
    for (std::vector< Expected_Elevator >::const_iterator it = expected_elevators.begin();
	 it != expected_elevators.end(); ++it)
    {
      if (it->state == Expected_Elevator::unknown || it->state == Expected_Elevator::inactive)
	++num_defunct_elevators;
    }
    
    unsigned int used_elevators_count = 0;
    for (std::vector< std::pair< Id_Type, bool > >::const_iterator it = expected_elevators_ids.begin();
	it != expected_elevators_ids.end(); ++it)
    {
      if (it->second)
	++used_elevators_count;
    }
    
    std::cout<<destinations.size()<<'\t'<<found_ways<<'\t'
        <<(destinations.size() * destinations.size() - found_ways)<<'\t'
        <<expected_elevators.size()<<'\t'<<elevators_with_coords_count<<'\t'
        <<matched_elevators_count<<'\t'<<used_elevators_count<<'\t'
        <<num_defunct_elevators<<'\t'
        <<ways_using_elevators<<'\t'<<station_name<<'\n';
  }
  
  return 0;
}
