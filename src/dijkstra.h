#ifndef DIJSKTRA_H
#define DIJSKTRA_H


#include "geocode.h"

#include <limits>
#include <vector>


struct Routing_Node;

struct Routing_Edge
{
  Routing_Edge() : valuation(0), start(0), end(0) {}
  
  std::vector< Coord > trace;
  double valuation;
  const Routing_Node* start;
  const Routing_Node* end;
};


struct Routing_Node
{
  Routing_Node(Id_Type id_, bool is_routable_, double penalty_)
      : id(id_), is_routable(is_routable_), penalty(penalty_) {}
  
  Id_Type id;
  bool is_routable;
  double penalty;
  std::vector< const Routing_Edge* > edges;
  
  bool operator<(const Routing_Node& rhs) const { return id < rhs.id; }
};


struct Routing_Profile
{
  virtual bool is_routable(const Way& way) const = 0;
  virtual bool is_routable(const Node& node) const = 0;
  
  virtual double valuation_factor(const Way& way) const = 0;
  virtual double node_penalty(const Node& node) const = 0;
  
  virtual ~Routing_Profile() {}
};


class Routing_Data
{
public:
  Routing_Data(const Parsing_State& data, const Routing_Profile& profile);
  
  void print_statistics() const;
  
  std::pair< const Routing_Edge*, unsigned int > resolve_way_pos(unsigned int way_id, unsigned int index) const;
  
private:
  std::vector< Routing_Node > nodes;
  std::vector< Routing_Edge > edges;
  
  std::vector< std::pair< Id_Type, std::vector< std::pair< unsigned int, unsigned int > > > > way_dictionary;
  
  Routing_Edge edge_from_way(const Way& way, unsigned int start, unsigned int end,
			     const Parsing_State& data, const Routing_Profile& profile) const;
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
  const static double max_route_length;
  
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
