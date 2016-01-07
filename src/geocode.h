#ifndef GEOCODE
#define GEOCODE


#include "read_input.h"


struct Coord
{
  Coord(double lat_, double lon_) : lat(lat_), lon(lon_) {}
  
  double lat;
  double lon;
  
  bool operator==(const Coord& rhs) const { return lat == rhs.lat && lon == rhs.lon; }
};


struct Geometry
{
  Geometry(const Node& node);
  Geometry(const Way& way, const Parsing_State& data);
  Geometry(const Relation& relation, const Parsing_State& data);
  
  std::vector< std::vector< Coord > > coords;
  
  Coord bbox_center();
};


struct Way_Reference
{
  Way_Reference(const Way& way, const Coord& point, const Parsing_State& data);
  Way_Reference(const Relation& relation, const Coord& point, const Parsing_State& data);
  
  Id_Type way_ref;
  unsigned int index;
  double pos;
};


#endif
