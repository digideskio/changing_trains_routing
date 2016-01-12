#include "geocode.h"


#include <cmath>


Geometry::Geometry(const Node& node)
{
  coords.push_back(std::vector< Coord >());
  coords.back().push_back(Coord(node.lat, node.lon));
}


Geometry::Geometry(const Way& way, const Parsing_State& data)
{
  coords.push_back(std::vector< Coord >());
  for (std::vector< Id_Type >::const_iterator nds_it = way.nds.begin(); nds_it != way.nds.end(); ++nds_it)
  {
    std::vector< Node >::const_iterator n_it =
        std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
    if (n_it != data.nodes.end() && n_it->id == *nds_it)
      coords.back().push_back(Coord(n_it->lat, n_it->lon));
  }
}


Geometry::Geometry(const Relation& relation, const Parsing_State& data)
{
  for (std::vector< Relation::Member >::const_iterator members_it = relation.members.begin();
       members_it != relation.members.end(); ++members_it)
  {
    if (members_it->type == Node_t)
    {
      coords.push_back(std::vector< Coord >());
      std::vector< Node >::const_iterator n_it =
          std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(members_it->ref));
      if (n_it != data.nodes.end() && n_it->id == members_it->ref)
        coords.back().push_back(Coord(n_it->lat, n_it->lon));
    }
    else if (members_it->type == Way_t)
    {
      coords.push_back(std::vector< Coord >());
      std::vector< Way >::const_iterator w_it =
          std::lower_bound(data.ways.begin(), data.ways.end(), Way(members_it->ref));
      if (w_it != data.ways.end() && w_it->id == members_it->ref)
      {
        for (std::vector< Id_Type >::const_iterator nds_it = w_it->nds.begin(); nds_it != w_it->nds.end(); ++nds_it)
        {
          std::vector< Node >::const_iterator n_it =
              std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
          if (n_it != data.nodes.end() && n_it->id == *nds_it)
            coords.back().push_back(Coord(n_it->lat, n_it->lon));
        }
      }
    }
  }
}


Coord Geometry::bbox_center()
{
  double max_lat = -90.0;
  double min_lat = 90.0;
  double max_lon = -180.0;
  double min_lon = 180.0;
  
  for (std::vector< std::vector< Coord > >::const_iterator b_it = coords.begin(); b_it != coords.end(); ++b_it)
  {
    for (std::vector< Coord >::const_iterator it = b_it->begin(); it != b_it->end(); ++it)
    {
      max_lat = std::max(max_lat, it->lat);
      min_lat = std::min(min_lat, it->lat);
      max_lon = std::max(max_lon, it->lon);
      min_lon = std::min(min_lon, it->lon);
    }
  }
  
  return Coord((min_lat + max_lat) / 2, (min_lon + max_lon) / 2);
}


std::pair< double, double > distance_and_offset(const Coord& start, const Coord& end, const Coord& point)
{
  static double PI = acos(0)*2;
  
  double lon_scale = cos(point.lat * PI / 180.0);
  double length = sqrt((end.lat - start.lat) * (end.lat - start.lat)
      + (end.lon - start.lon) * lon_scale * (end.lon - start.lon) * lon_scale);
  
  if (length == 0)
    return std::make_pair(sqrt((point.lat - start.lat) * (point.lat - start.lat)
        + ((point.lon - start.lon) * lon_scale * (point.lon - start.lon) * lon_scale)), 0);
  
  double offset = ((end.lat - start.lat) * (point.lat - start.lat)
      + (point.lon - start.lon) * lon_scale) / length;
  if (offset <= 0)
    return std::make_pair(sqrt((point.lat - start.lat) * (point.lat - start.lat)
        + ((point.lon - start.lon) * lon_scale * (point.lon - start.lon) * lon_scale)), 0);
  else if (offset >= length)
    return std::make_pair(sqrt((point.lat - end.lat) * (point.lat - end.lat)
        + ((point.lon - end.lon) * lon_scale * (point.lon - end.lon) * lon_scale)), length);
    
  return std::make_pair(fabs((start.lon - end.lon) * lon_scale * (point.lat - start.lat)
        + ((end.lat - start.lat) * (point.lon - start.lon) * lon_scale)) / length, offset);
}


Way_Reference::Way_Reference(const Way& way, const Coord& point, const Parsing_State& data)
{
  double min_distance = 180.0;
  
  std::vector< Id_Type >::const_iterator nds_it = way.nds.begin();
  if (nds_it == way.nds.end())
  {
    index = 0;
    pos = 0;
    return;
  }
  
  std::vector< Node >::const_iterator n_it =
      std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
  Coord last_coord(0, 0);
  if (n_it != data.nodes.end() && n_it->id == *nds_it)
    last_coord = Coord(n_it->lat, n_it->lon);

  for (++nds_it; nds_it != way.nds.end(); ++nds_it)
  {
    std::vector< Node >::const_iterator n_it =
        std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
    if (n_it != data.nodes.end() && n_it->id == *nds_it)
    {
      Coord coord(n_it->lat, n_it->lon);
      
      std::pair< double, double > distance_offset = distance_and_offset(last_coord, coord, point);
      if (distance_offset.first <= min_distance)
      {
	min_distance = distance_offset.first;
	way_ref = way.id;
	index = std::distance(way.nds.begin(), nds_it)-1;
	pos = distance_offset.second;
      }
      
      last_coord = coord;      
    }
  }
}


Way_Reference::Way_Reference(const Relation& relation, const Coord& point, const Parsing_State& data)
{
  double min_distance = 180.0;
  
  for (std::vector< Relation::Member >::const_iterator members_it = relation.members.begin();
       members_it != relation.members.end(); ++members_it)
  {
    if (members_it->type == Way_t)
    {
      std::vector< Way >::const_iterator w_it =
          std::lower_bound(data.ways.begin(), data.ways.end(), Way(members_it->ref));
      if (w_it != data.ways.end() && w_it->id == members_it->ref)
      {
        std::vector< Id_Type >::const_iterator nds_it = w_it->nds.begin();
        if (nds_it == w_it->nds.end())
	  continue;
  
        std::vector< Node >::const_iterator n_it =
            std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
        Coord last_coord(0, 0);
        if (n_it != data.nodes.end() && n_it->id == *nds_it)
          last_coord = Coord(n_it->lat, n_it->lon);
  
        for (++nds_it; nds_it != w_it->nds.end(); ++nds_it)
        {
          std::vector< Node >::const_iterator n_it =
              std::lower_bound(data.nodes.begin(), data.nodes.end(), Node(*nds_it));
          if (n_it != data.nodes.end() && n_it->id == *nds_it)
          {
            Coord coord(n_it->lat, n_it->lon);
      
            std::pair< double, double > distance_offset = distance_and_offset(last_coord, coord, point);
            if (distance_offset.first <= min_distance)
            {
	      min_distance = distance_offset.first;
	      way_ref = w_it->id;
	      index = std::distance(w_it->nds.begin(), nds_it)-1;
	      pos = distance_offset.second;
            }
      
            last_coord = coord;      
          }
        }
      }
    }
  }
}
