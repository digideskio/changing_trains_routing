#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>


std::string fuzz_name(std::string input)
{
  std::string name;
  std::string::size_type pos = 0;
  while (pos < input.size())
  {
    if (input[pos] & 0x80)
      name += input[pos];
    else if (isalpha(input[pos]))
      name += tolower(input[pos]);
    else if (!name.empty() && (name.size() == 1 || name.substr(name.size()-2) != ".*"))
      name += ".*";
    ++pos;
  }
  return name;
}


int main(int argc, char* args[])
{
  if (argc < 2)
    return 0;

  unsigned int fuzzing_level = 0;
  std::string input = args[1];

  if (argc >= 3)
  {
    fuzzing_level = atoi(args[1]);
    input = args[2];
  }

  std::string name;
  std::string area;

  if (fuzzing_level == 1)
  {
    if (input[input.size()-1] == ')')
    {
      std::string::size_type pos = input.find(" (");
      if (pos != std::string::npos)
        input = input.substr(0, pos);
    }

    std::string::size_type pos = input.find("-");
    if (pos != std::string::npos)
    {
      area = input.substr(0, pos);
      input = input.substr(pos+1);
    }

    if (area == "")
    {
      pos = input.find(") ");
      if (pos != std::string::npos)
      {
        area = input.substr(0, pos+1);
        input = input.substr(pos+2);
      }
    }

    if (area == "")
    {
      pos = input.find(" ");
      if (pos != std::string::npos)
      {
        area = input.substr(0, pos);
        input = input.substr(pos+1);
      }
    }
  }

  name = fuzz_name(input);
  area = fuzz_name(area);
  
  std::string::size_type pos = name.find(".*hbf");
  if (pos != std::string::npos)
    name = name.substr(0, pos) + ".*(hbf|hauptbahnhof)" + name.substr(pos + 5);

  pos = name.find(".*pbf");
  if (pos != std::string::npos)
    name = name.substr(0, pos) + ".*" + name.substr(pos + 5);

  pos = name.find(".*hp");
  if (pos != std::string::npos)
    name = name.substr(0, pos) + ".*" + name.substr(pos + 4);

  if (name.size() >= 2 && name.substr(name.size()-2) == ".*")
    name = name.substr(0, name.size()-2);

  std::string buffer;
  std::getline(std::cin, buffer);
  while (std::cin.good())
  {
    std::string::size_type pos = buffer.find("{{name}}");
    while (pos != std::string::npos)
    {
      buffer = buffer.substr(0, pos) + name + buffer.substr(pos + 8);
      pos = buffer.find("{{name}}", pos + 8 - name.size());
    }

    if (fuzzing_level == 1)
    {
      if (area == "")
      {
        std::string::size_type pos = buffer.find("{{area}}");
        if (pos != std::string::npos)
          buffer = "";

        pos = buffer.find("(area.a)");
        while (pos != std::string::npos)
        {
          buffer = buffer.substr(0, pos) + buffer.substr(pos + 8);
          pos = buffer.find("(area.a)", pos + 8);
        }
      }
      else
      {
        std::string::size_type pos = buffer.find("{{area}}");
        while (pos != std::string::npos)
        {
          buffer = buffer.substr(0, pos) + area + buffer.substr(pos + 8);
          pos = buffer.find("{{area}}", pos + 8 - area.size());
        }
      }
    }
    
    std::cout<<buffer<<'\n';
    std::getline(std::cin, buffer);
  }

  return 0;
}
