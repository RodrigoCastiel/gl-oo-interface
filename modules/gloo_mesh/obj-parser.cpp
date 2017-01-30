#include "obj-parser.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

namespace gloo
{

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
    if ( (rawLine[i] != ' ')                         // If not space,
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
  attrib.mData.clear();

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

    attrib.mData.push_back(coord);
  }

  if ((components[0] == atomic::VT) && (attrib.mData.size()-1 > 3 || attrib.mData.size()-1 < 2)) 
  {
    if (verbose)
      std::cerr << "ERROR VT must be 2D or 3D." << std::endl;
    return false;
  }
  else if (attrib.mData.size()-1 > 4 || attrib.mData.size() < 3)
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
  face.mIndices.clear();

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

    face.mIndices.push_back({-1, -1, -1});

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
        face.mIndices[i-1][j] = coord;
      }
    }
  }

  return true;
}


}  // namespace gloo.
