#include <string>
#include <vector>


typedef unsigned long long Id_Type;


enum Object_Type { Node_t, Way_t, Relation_t };

  
struct Node
{
  Node(Id_Type id_ = 0) : id(id_), lat(100.0), lon(200.0) {}
  
  Id_Type id;
  double lat;
  double lon;
  std::vector< std::pair< std::string, std::string > > tags;
  
  bool operator<(const Node& rhs) const { return id < rhs.id; }
};


struct Way
{
  Way(Id_Type id_ = 0) : id(id_) {}
  
  Id_Type id;
  std::vector< Id_Type > nds;
  std::vector< std::pair< std::string, std::string > > tags;
  
  bool operator<(const Way& rhs) const { return id < rhs.id; }
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
  
  bool operator<(const Relation& rhs) const { return id < rhs.id; }
};


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


const Parsing_State& read_osm();
