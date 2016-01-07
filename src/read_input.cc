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
#include <iostream>

#include <cstdlib>
#include <cstring>

#include "expat_justparse_interface.h"
#include "read_input.h"


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
  
  std::sort(g_state->nodes.begin(), g_state->nodes.end());
  std::sort(g_state->ways.begin(), g_state->ways.end());
  std::sort(g_state->relations.begin(), g_state->relations.end());

  return global_state();
}


const Parsing_State& read_osm(const std::string& filename)
{
  FILE* input = fopen(filename.c_str(), "r");
  g_state = &global_state();
  
  // read the XML input
  parse(input, start, end);
  
  std::sort(g_state->nodes.begin(), g_state->nodes.end());
  std::sort(g_state->ways.begin(), g_state->ways.end());
  std::sort(g_state->relations.begin(), g_state->relations.end());

  return global_state();
}
