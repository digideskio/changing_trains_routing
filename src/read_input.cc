/** Copyright 2008, 2009, 2010, 2011, 2012 Roland Olbricht
*
* This file is part of PT_Diagrams.
*
* PT_Diagrams is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PT_Diagrams is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PT_Diagrams.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctime>
#include "expat_justparse_interface.h"

//#include "read_input.h"


typedef unsigned long long Id_Type;


enum Object_Type { Node_t, Way_t, Relation_t };

  
struct Node
{
  Node() : id(0), lat(100.0), lon(200.0) {}
  
  Id_Type id;
  double lat;
  double lon;
  std::vector< std::pair< std::string, std::string > > tags;
};


struct Way
{
  Id_Type id;
  std::vector< Id_Type > nds;
  std::vector< std::pair< std::string, std::string > > tags;
};


struct Relation
{
  struct Member
  {
    Id_Type ref;
    Object_Type type;
    std::string role;
  };

  Id_Type id;
  std::vector< Member > members;
  std::vector< std::pair< std::string, std::string > > tags;
};


template< typename Object >
bool has_kv(const Object& obj, const std::string& key, const std::string& value)
{
  for (std::vector< std::pair< std::string, std::string > >::const_iterator it = obj.tags.begin();
       it != obj.tags.end(); ++it)
  {
    if (it->first == key && it->second == value)
      return true;
  }
  return false;
}


struct Parsing_State
{
  std::vector< std::pair< std::string, std::string > > tags;
  
  std::vector< Node > nodes;
  
  std::vector< Way > ways;
  std::vector< Id_Type > nds;
  
  std::vector< Relation > relations;
  std::vector< Relation::Member > members;
  
  std::vector< Node > implicit_nodes;
};

Parsing_State* g_state;


void start(const char *el, const char **attr)
{
  if (!strcmp(el, "tag"))
  {
    std::string key, value;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "k"))
	key = attr[i+1];
      else if (!strcmp(attr[i], "v"))
	value = attr[i+1];
    }
    if (key != "" && value != "")
      g_state->tags.push_back(std::make_pair(key, value));
  }
  else if (!strcmp(el, "node"))
  {
    Node node;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "id"))
	node.id = atoll(attr[i+1]);
      else if (!strcmp(attr[i], "lat"))
	node.lat = atof(attr[i+1]);
      else if (!strcmp(attr[i], "lon"))
	node.lon = atof(attr[i+1]);
    }
    if (node.id != 0 && node.lat >= -90.0 && node.lat <= 90.0 && node.lon >= -180.0 && node.lon <= 180.0)
      g_state->nodes.push_back(node);
    
    g_state->tags.clear();
  }
  else if (!strcmp(el, "way"))
  {
    Way way;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "id"))
	way.id = atoll(attr[i+1]);
    }
    if (way.id != 0)
      g_state->ways.push_back(way);
    
    g_state->tags.clear();
    g_state->nds.clear();
  }
  else if (!strcmp(el, "nd"))
  {
    Node node;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "ref"))
	node.id = atoll(attr[i+1]);
      else if (!strcmp(attr[i], "lat"))
	node.lat = atof(attr[i+1]);
      else if (!strcmp(attr[i], "lon"))
	node.lon = atof(attr[i+1]);
    }
    if (node.id != 0)
    {
      g_state->nds.push_back(node.id);
      if (node.lat >= -90.0 && node.lat <= 90.0 && node.lon >= -180.0 && node.lon <= 180.0)
	g_state->implicit_nodes.push_back(node);
    }
  }
  else if (!strcmp(el, "relation"))
  {
    Relation relation;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "id"))
	relation.id = atoll(attr[i+1]);
    }
    if (relation.id != 0)
      g_state->relations.push_back(relation);
    
    g_state->tags.clear();
    g_state->members.clear();
  }
  else if (!strcmp(el, "member"))
  {
    Relation::Member member;
    for (unsigned int i(0); attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "ref"))
	member.ref = atoll(attr[i+1]);
      else if (!strcmp(attr[i], "type"))
      {
	std::string type(attr[i+1]);
	if (type == "node")
	  member.type = Node_t;
	else if (type == "way")
	  member.type = Way_t;
	else if (type == "relation")
	  member.type = Relation_t;
      }
      else if (!strcmp(attr[i], "role"))
	member.role = attr[i+1];
    }
    if (member.ref != 0)
      g_state->members.push_back(member);
  }
}


void end(const char *el)
{
  if (!strcmp(el, "node"))
    g_state->nodes.back().tags.swap(g_state->tags);
  else if (!strcmp(el, "way"))
  {
    g_state->ways.back().tags.swap(g_state->tags);
    g_state->ways.back().nds.swap(g_state->nds);
  }
  else if (!strcmp(el, "relation"))
  {
    g_state->relations.back().tags.swap(g_state->tags);
    g_state->relations.back().members.swap(g_state->members);
  }
}


Parsing_State& global_state()
{
  static Parsing_State* singleton = 0;
  if (!singleton)
    singleton = new Parsing_State();
  return *singleton;
}


const Parsing_State& read_osm()
{
  g_state = &global_state();
  
  // read the XML input
  parse(stdin, start, end);
  
//   for (std::map< uint32, Way* >::const_iterator it(current_data.ways.begin());
//       it != current_data.ways.end(); ++it)
//   {
//     std::vector< Pending_Nd >& nds = pending_nds[it->first];
//     for (std::vector< Pending_Nd >::const_iterator it2 = nds.begin(); it2 != nds.end(); ++it2)
//     {
//       std::map< uint64, Node* >::const_iterator nit = current_data.nodes.find(it2->ref);
//       if (nit != current_data.nodes.end())
// 	it->second->nds.push_back(nit->second);
//       else if (it2->lat < 100.0 && it2->lon < 200.0)
//       {
// 	Node* node = new Node();
// 	node->lat = it2->lat;
// 	node->lon = it2->lon;
// 	current_data.nodes.insert(std::make_pair(it2->ref, node));
// 	it->second->nds.push_back(node);
//       }
//       else
//       {
// 	std::cerr<<"Error: Node "<<it2->ref<<" referenced by way "<<it->first
// 	    <<" but not contained in the source file.\n";
// 	// better throw an exception
//       }
//     }
//   }
//   
//   for (std::map< uint32, Relation* >::const_iterator it(current_data.relations.begin());
//       it != current_data.relations.end(); ++it)
//   {
//     std::vector< RelMember >& members(pending_members[it->first]);
//     for (std::vector< RelMember >::const_iterator it2(members.begin());
//         it2 != members.end(); ++it2)
//     {
//       if (it2->type == RelMember::NODE)
//       {
// 	std::map< uint64, Node* >::const_iterator nit(current_data.nodes.find(it2->ref));
// 	if (nit == current_data.nodes.end())
// 	{
// 	  std::cerr<<"Error: Node "<<it2->ref<<" referenced by relation "<<it->first
// 	  <<" but not contained in the source file.\n";
// 	  // throw an exception
// 	}
// 	else
// 	  it->second->members.push_back(std::make_pair(nit->second, it2->role));
//       }
//       else if (it2->type == RelMember::WAY)
//       {
// 	std::map< uint32, Way* >::const_iterator nit(current_data.ways.find(it2->ref));
// 	if (nit == current_data.ways.end())
// 	{
// 	  std::cerr<<"Error: Way "<<it2->ref<<" referenced by relation "<<it->first
// 	  <<" but not contained in the source file.\n";
// 	  // throw an exception
// 	}
// 	else
// 	  it->second->members.push_back(std::make_pair(nit->second, it2->role));
//       }
//       if (it2->type == RelMember::RELATION)
//       {
// 	std::map< uint32, Relation* >::const_iterator nit(current_data.relations.find(it2->ref));
// 	if (nit == current_data.relations.end())
// 	{
// 	  std::cerr<<"Error: Relation "<<it2->ref<<" referenced by relation "<<it->first
// 	  <<" but not contained in the source file.\n";
// 	  // throw an exception
// 	}
// 	else
// 	  it->second->members.push_back(std::make_pair(nit->second, it2->role));
//       }
//     }
//   }

  return global_state();
}


int main()
{
  const Parsing_State& state = read_osm();
  
  for (std::vector< Way >::const_iterator it = state.ways.begin(); it != state.ways.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      std::cout<<it->id<<'\n';
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
	std::cout<<"  "<<it2->first<<" = "<<it2->second<<'\n';
    }
  }
  
  for (std::vector< Relation >::const_iterator it = state.relations.begin(); it != state.relations.end(); ++it)
  {
    if (has_kv(*it, "railway", "platform") ||
        (has_kv(*it, "public_transport", "platform") && !has_kv(*it, "bus", "yes")))
    {
      std::cout<<it->id<<'\n';
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it2 = it->tags.begin();
          it2 != it->tags.end(); ++it2)
	std::cout<<"  "<<it2->first<<" = "<<it2->second<<'\n';
    }
  }
  
  return 0;
}
