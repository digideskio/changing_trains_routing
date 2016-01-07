#ifndef DIJSKTRA_H
#define DIJSKTRA_H


#include "geocode.h"

#include <vector>


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
  
  std::pair< const Routing_Edge*, unsigned int > resolve_way_pos(unsigned int way_id, unsigned int index) const;
  
private:
  std::vector< Routing_Node > nodes;
  std::vector< Routing_Edge > edges;
  
  std::vector< std::pair< Id_Type, std::vector< std::pair< unsigned int, unsigned int > > > > way_dictionary;
  
  Routing_Edge edge_from_way(const Way& way, unsigned int start, unsigned int end, const Parsing_State& data) const;
};


struct Route_Ref
{
  Route_Ref(const Routing_Data& routing_data, const Way_Reference& way_ref, const std::string label,
	    const Parsing_State& data);
  
  double proportionate_valuation() const;
  
  std::string label;
  const Routing_Edge* edge;
  unsigned int index;
  double pos;
};


struct Route
{
  const static double max_route_length = 180.0;
  
  Route(const Route_Ref& start_, const Route_Ref& end_, double value_) : start(start_), end(end_), value(value_) {}
  
  Route_Ref start;
  Route_Ref end;
  std::vector< const Routing_Edge* > edges;
  double value;
};


struct Route_Tree
{
  Route_Tree(const Routing_Data& routing_data, const Route_Ref& origin, const std::vector< Route_Ref >& destinations);
  
  std::vector< Route > routes;
};


double distance(const Coord& a, const Coord& b);


#endif
