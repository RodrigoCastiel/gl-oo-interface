#include "obj-parser.h"

#include <vector>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace gloo
{

void PrintStringList(const std::vector<std::string> & stringList)
{
  std::cout << "{";
  for (size_t i = 0; i < stringList.size(); i++)
  {
    std::cout << "\"" << stringList[i] << "\"";

    if (i < stringList.size()-1)
      std::cout << ", ";
  }
  std::cout << "}";
}

void ObjParser::SplitByString(const std::string & input, const std::string & separator, 
                              std::vector<std::string> & stringList, bool removeEmptyComponents)
{
  size_t start  = 0;
  size_t length = separator.size(); 

  while (start < input.size())
  {
    size_t pos = input.find(separator, start);
    stringList.push_back(input.substr(start, pos-start));
    if (pos == std::string::npos)
    {
      break;
    }
    start = pos + length;
  }

  if (removeEmptyComponents)
    stringList.erase( std::remove( stringList.begin(), stringList.end(), ""), stringList.end() ); 
}

void ObjParser::PreprocessLine(const std::string & rawLine, std::string & filteredLine)
{
  filteredLine = std::string();
  filteredLine.reserve(rawLine.size());

  // Remove repeated spaces.
  for (size_t i = 0; i < rawLine.size(); i++)
  {
    if ( (!isspace(rawLine[i]))                      // If not space,
      || ((i > 0) && (rawLine[i] != rawLine[i-1]))   // or a not repeated space
      || (i == 0))                                   // or space at the first position.
    {
      filteredLine.push_back(rawLine[i]);
    }
  }
}

bool ObjParser::ParseAttribute(const std::vector<std::string> & components, Attribute & attrib,
                               bool verbose)
{
  attrib.clear();

  for (int i = 1; i < components.size(); i++)
  {
    // Check if the component is a valid number.
    char* p;
    float coord = strtof(components[i].c_str(), &p);
    if (*p || components[i].size() == 0)
    {
      if (verbose)
        std::cerr << "ERROR Could not read numerical data: '" << components[i] << "' " << std::endl;
      return false;
    }

    attrib.push_back(coord);
  }

  if (components.front() == atomic::VT) 
  {
    if (attrib.size() > 3 || attrib.size() < 2)
    {
      if (verbose)
        std::cerr << "ERROR VT must be 2D or 3D." << std::endl;
      return false;
    }
    else
    {
      return true;
    }
  }
  else if (attrib.size() > 4 || attrib.size() < 3)
  {
    if (verbose)
      std::cerr << "ERROR V, VN must be 3D or 4D." << std::endl;
    return false;
  }
  else
  {
    return true;
  }
}

bool ObjParser::ParseFace(const std::vector<std::string> & components, Face & face,
                          bool verbose)
{
  face.clear();

  for (int i = 1; i < components.size(); i++)
  {
    // Read the i-th vertex data into components.
    std::vector<std::string> subcomponents;
    ObjParser::SplitByString(components[i], "/", subcomponents, false);

    // Check the number of subcomponents.
    if (subcomponents.size() < 1 || subcomponents.size() > 3)
    {
      if (verbose)
        std::cerr << "ERROR Wrong vertex indices format. " << std::endl;
      return false;
    }

    face.push_back({-1, -1, -1});

    // Convert subcomponents into indices.
    for (int j = 0; j < subcomponents.size(); j++)
    {
      char* p;
      int coord = strtol(subcomponents[j].c_str(), &p, 10);
      if (*p)
      {
        if (verbose)
          std::cerr << "ERROR Could not read index: '" << subcomponents[j] << "' " << std::endl;
        return false;
      }

      // Index is being provided.
      if (subcomponents[j].size() > 0)
      {
        face[i-1][j] = coord;
      }
    }
  }

  return true;
}

bool ObjParser::LoadObj(const std::string & filename, ObjMesh & objMesh, bool verbose)
{
  std::ifstream objFile(filename);
  std::string line;
  int lineNumber = 1;
  bool hasBuiltGroup = false;

  if (!objFile.is_open())
  {
    if (verbose)
      std::cerr << "ERROR Could not open .obj file at '" << filename << "'. " << std::endl;
    return false;
  }

  while (std::getline(objFile, line))
  {
    // Filter line and split into components (also, remove empty components).
    std::string filteredLine;
    std::vector<std::string> components;

    ObjParser::PreprocessLine(line, filteredLine);
    ObjParser::SplitByString(filteredLine, " ", components, true);
    // std::cout << lineNumber << ": '" << filteredLine << "'\n";

    if (components.empty()) 
      continue;

    // Check first component type.
    if (components.front() == atomic::G)  // New group.
    {
      std::string groupName;

      for (int i = 1; i < components.size(); i++)
      {
        groupName += components[i];
      }

      objMesh.AddGroup(groupName);
    }
    else
    {
      if (components.front() == atomic::V)  // New vertex.
      {
        Attribute vertex;
        if (!ObjParser::ParseAttribute(components, vertex, verbose)) 
        {
          std::cout << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
          std::cout << lineNumber << ": '" << line << "'\n";
          return false;
        }

        objMesh.AddVertex(vertex);
      }
      else if (components.front() == atomic::VT)  // New texture uv.
      {
        Attribute uv;
        if (!ObjParser::ParseAttribute(components, uv, verbose)) 
        {
          std::cout << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
          std::cout << lineNumber << ": '" << line << "'\n";
          return false;
        }

        objMesh.AddUV(uv);
      }
      else if (components.front() == atomic::VN)  // New normal vector.
      {
        Attribute normal;
        if (!ObjParser::ParseAttribute(components, normal, verbose)) 
        {
          std::cout << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
          std::cout << lineNumber << ": '" << line << "'\n";
          return false;
        }

        objMesh.AddNormal(normal);
      }
      else if (components.front() == atomic::F)  // New face.
      {
        Face face;
        if (!ObjParser::ParseFace(components, face, verbose))
        {
          std::cout << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
          std::cout << lineNumber << ": '" << line << "'\n";
          return false;
        }

        objMesh.AddFace(face);
      }
    
      else  // Ignore comments and other kinds of line.
      {
        // std::cout << "Ignore line " << lineNumber << std::endl;
      }
    }
    
    lineNumber++;
  }

  return true;
}

}  // namespace gloo.

int main()
{
  gloo::ObjParser parser;
  gloo::ObjMesh objMesh;

  parser.LoadObj("B-747.obj", objMesh, true);

  objMesh.LogData();

  // std::string line = "       f  0/2/0   3/0/0  5/4/0  ";
  // std::string filtered;
  // std::string separator = " ";
  // std::vector<std::string> list;
  // gloo::ObjParser::Attribute attrib;
  // gloo::ObjParser::Face face;

  // parser.PreprocessLine(line, filtered);
  // parser.SplitByString(filtered, separator, list, true);

  // std::cout << "Filtered line = '" << filtered << "'" << std::endl;
  // gloo::PrintStringList(list);

  // parser.ParseAttribute(list, attrib, true);
  // parser.ParseFace(list, face, true);

  // PrintStringList(list);

  return 0;
}