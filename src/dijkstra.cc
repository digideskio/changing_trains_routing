#include "read_input.h"
#include "geocode.h"


#include <cmath>
#include <iostream>
#include <map>


struct Routing_Node;

struct Routing_Edge
{
  Routing_Edge() : start(0), end(0), valuation(0) {}
  
  std::vector< Coord > trace;
  double valuation;
  const Routing_Node* start;
  const Routing_Node* end;
};


struct Routing_Node
{
  Routing_Node(Id_Type id_) : id(id_) {}
  
  Id_Type id;
  std::vector< const Routing_Edge* > edges;
  
  bool operator<(const Routing_Node& rhs) const { return id < rhs.id; }
};


class Routing_Data
{
public:
  Routing_Data(const Parsing_State& data);
  
  void print_statistics() const;
  
private:
  std::vector< Routing_Node > nodes;
  std::vector< Routing_Edge > edges;
  
  Routing_Edge edge_from_way(const Way& way, unsigned int start, unsigned int end, const Parsing_State& data) const;
};


double distance(const Coord& a, const Coord& b)
{
  static double PI = acos(0)*2;
  
  double lon_scale = 1.0/cos((a.lat + b.lat) / 2.0 * PI / 180.0);
  return sqrt((b.lat - a.lat) * (b.lat - a.lat)
      + (b.lon - a.lon) * lon_scale * (b.lon - a.lon) * lon_scale);
}


Routing_Edge Routing_Data::edge_from_way(const Way& way, unsigned int start, unsigned int end,
					 const Parsing_State& data) const
{
  Routing_Edge edge;
  std::vector< Routing_Node >::const_iterator n_it
      = std::lower_bound(nodes.begin(), nodes.end(), Routing_Node(way.nds[start]));
  if (n_it != nodes.end())
    edge.start = &(*n_it);
  n_it = std::lower_bound(nodes.begin(), nodes.end(), Routing_Node(way.nds[end]));
  if (n_it != nodes.end())
    edge.end = &(*n_it);

  Coord last_coord(0, 0);
  for (unsigned int i = start; i <= end; ++i)
  {
    std::vector< Node >::const_iterator n_it =
        std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(way.nds[i]));
    if (n_it != data.nodes.end() && n_it->id == way.nds[i])
    {
      Coord coord(n_it->lat, n_it->lon);
      edge.trace.push_back(coord);
      if (i > start)
	edge.valuation += distance(last_coord, coord);
      last_coord = coord;
    }
  }
  
  return edge;
}


Routing_Data::Routing_Data(const Parsing_State& data)
{
  std::map< Id_Type, unsigned int > node_count;
  
  for (std::vector< Way >::const_iterator it = data.ways.begin(); it != data.ways.end(); ++it)
  {
    if (!it->nds.empty())
    {
      node_count[it->nds[0]] += 2;
      for (unsigned int i = 1; i < it->nds.size()-1; ++i)
        ++node_count[it->nds[i]];
      node_count[it->nds[it->nds.size()-1]] += 2;
    }
  }
  
  for (std::map< Id_Type, unsigned int >::const_iterator it = node_count.begin(); it != node_count.end(); ++it)
  {
    if (it->second > 1)
      nodes.push_back(Routing_Node(it->first));
  }
  
  for (std::vector< Way >::const_iterator it = data.ways.begin(); it != data.ways.end(); ++it)
  {
    unsigned int start = 0;
    for (unsigned int i = 1; i < it->nds.size(); ++i)
    {
      if (node_count[it->nds[i]] > 1)
      {
	edges.push_back(edge_from_way(*it, start, i, data));
	start = i;
      }
    }
    if (start < it->nds.size() - 1)
      edges.push_back(edge_from_way(*it, start, it->nds.size() - 1, data));
  }
  
  for (std::vector< Routing_Edge >::const_iterator it = edges.begin(); it != edges.end(); ++it)
  {
    std::vector< Routing_Node >::iterator n_it
	= std::lower_bound(nodes.begin(), nodes.end(), *it->start);
    if (n_it != nodes.end())
      n_it->edges.push_back(&(*it));
    n_it = std::lower_bound(nodes.begin(), nodes.end(), *it->end);
    if (n_it != nodes.end())
      n_it->edges.push_back(&(*it));
  }
}


void Routing_Data::print_statistics() const
{
  double total_valuation = 0;
  for (std::vector< Routing_Edge >::const_iterator it = edges.begin(); it != edges.end(); ++it)
    total_valuation += it->valuation;
  
  std::cout<<nodes.size()<<' '<<edges.size()<<' '<<total_valuation * 111111.1<<'\n';
}


int main(int argc, char* args[])
{
  const Parsing_State& state = read_osm();
  
  Routing_Data routing_data(state);
  routing_data.print_statistics();
  
  for (std::vector< Way >::const_iterator it = state.ways.begin(); it != state.ways.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      std::cout<<it->id<<'\n';
      Geometry geom(*it, state);
      Coord center = geom.bbox_center();
      std::cout<<"  "<<center.lat<<' '<<center.lon<<'\n';
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
	std::cout<<"  "<<it2->first<<" = "<<it2->second<<'\n';
      Way_Reference ref(*it, center, state);
      std::cout<<"  "<<ref.way_ref<<' '<<ref.index<<' '<<ref.pos * 111111.1<<'\n';
    }
  }
  
  for (std::vector< Relation >::const_iterator it = state.relations.begin(); it != state.relations.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      std::cout<<it->id<<'\n';
      Geometry geom(*it, state);
      Coord center = geom.bbox_center();
      std::cout<<"  "<<center.lat<<' '<<center.lon<<'\n';
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
	std::cout<<"  "<<it2->first<<" = "<<it2->second<<'\n';
      Way_Reference ref(*it, center, state);
      std::cout<<"  "<<ref.way_ref<<' '<<ref.index<<' '<<ref.pos * 111111.1<<'\n';
    }
  }
  
  return 0;
}
