#include "obj-parser.h"

#include <vector>
#include <cctype>
#include <cstdio>
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

bool ObjParser::ParseVec2f(const char * line, glm::vec2 & attrib, 
                           bool verbose)
{
  if (sscanf(line, "%f %f", &attrib[0], &attrib[1]) != 2)
  {
    if (verbose)
      std::cerr << "ERROR The attribute must have 2 values. " << std::endl;
    return false;
  }
  else
  {
    return true;
  }
}

bool ObjParser::ParseVec3f(const char * line, glm::vec3 & attrib, 
                           bool verbose)
{
  
  if (sscanf(line, "%f %f %f", &attrib[0], &attrib[1], &attrib[2]) != 3)
  {
    if (verbose)
      std::cerr << "ERROR The attribute must have 3 values. " << std::endl;
    return false;
  }
  else
  {
    return true;
  }
}

bool ObjParser::ParseFace(const std::string & lineData, Face & face, bool verbose)
{
  // Check if there is any data.
  if (lineData.size() == 0) {
    if (verbose)
      std::cerr << "ERROR Empty line." << std::endl;
    return false;
  }

  int i0 = 0;
  int vtxNumber = 0;
  for (int i = i0; i < lineData.size(); )
  {
    // Search for the next space index. "
    i++;
    for (; i < lineData.size() && !isspace(lineData[i]); ++i); 

    // Read the i-th vertex data into components.
    std::string vertexInfo = lineData.substr(i0, i-i0);
    std::vector<std::string> subcomponents;
    ObjParser::SplitByString(vertexInfo, "/", subcomponents, false);

    // Check the number of subcomponents.
    if (subcomponents.size() < 1 || subcomponents.size() > 3)
    {
      if (verbose)
        std::cerr << "ERROR Wrong vertex indices format. " << std::endl;
      return false;
    }

    face.VertexList().push_back({-1, -1, -1});

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

      // Index is provided.
      if (subcomponents[j].size() > 0)
      {
        face[vtxNumber][j] = coord-1;
      }
    }

    i0 = i+1;
    vtxNumber++;
  }

  if (vtxNumber < 3) 
  {
     if (verbose)
      std::cerr << "ERROR A face (polygon) must contain at least 3 vertices." << std::endl;
    return false;
  }

  return true;
}

bool ObjParser::LoadObj(const std::string & filename, ObjMesh & objMesh, bool verbose)
{
  std::ifstream objFile(filename);
  std::string line;
  int lineNumber = 1;

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
    ObjParser::PreprocessLine(line, filteredLine);

    // Find first white space.
    int ws_pos = 0;
    for (int i = 0; i < filteredLine.size() && !isspace(filteredLine[i]); i++) 
      ws_pos++;

    if (filteredLine.size() < 2 || !isspace(filteredLine[ws_pos]))
      continue;

    // Split line into two pieces: the ID and its data.
    std::string lineID   = filteredLine.substr(0, ws_pos-0);
    std::string lineData = filteredLine.substr(ws_pos+1);

    // Check first component type.
    if (lineID == atomic::G)  // New group.
    {
      objMesh.AddGroup(lineData);
    }
    else
    {
      if (lineID == atomic::V)  // New vertex.
      {
        glm::vec3 vertex;
        if (!ObjParser::ParseVec3f(lineData.c_str(), vertex, verbose)) 
        {
          if (verbose)
          {
            std::cerr << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
            std::cerr << lineNumber << ": '" << line << "'\n";  
          }
          return false;
        }

        objMesh.AddVertex(vertex);
      }
      else if (lineID == atomic::VT)  // New texture uv.
      {
        glm::vec2 uv;
        if (!ObjParser::ParseVec2f(lineData.c_str(), uv, verbose)) 
        {
          if (verbose)
          {
            std::cerr << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
            std::cerr << lineNumber << ": '" << line << "'\n";  
          }
          return false;
        }

        objMesh.AddUV(uv);
      }
      else if (lineID == atomic::VN)  // New normal vector.
      {
        glm::vec3 normal;
        if (!ObjParser::ParseVec3f(lineData.c_str(), normal, verbose)) 
        {
          if (verbose)
          {
            std::cerr << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
            std::cerr << lineNumber << ": '" << line << "'\n";  
          }
          return false;
        }

        objMesh.AddNormal(normal);
      }
      else if (lineID == atomic::F)  // New face.
      {
        Face face;
        if (!ObjParser::ParseFace(lineData, face, verbose))
        {
          if (verbose)
          {
            std::cerr << "\t^ at line " << lineNumber << " ('" << filename << "')" << std::endl;
            std::cerr << lineNumber << ": '" << line << "'\n";  
          }
          return false;
        }

        objMesh.AddFace(face);
      }
      // Ignore comments and other kinds of line.
    }
    
    lineNumber++;
  }

  return true;
}

}  // namespace gloo.

// int main()
// {
//   gloo::ObjParser parser;
//   gloo::ObjMesh objMesh;

//   parser.LoadObj("weirdo.obj", objMesh, true);

//   objMesh.LogData();

//   // std::string line = "       f  0/2/0   3/0/0  5/4/0  ";
//   // std::string filtered;
//   // std::string separator = " ";
//   // std::vector<std::string> list;
//   // gloo::ObjParser::Attribute attrib;
//   // gloo::ObjParser::Face face;

//   // parser.PreprocessLine(line, filtered);
//   // parser.SplitByString(filtered, separator, list, true);

//   // std::cout << "Filtered line = '" << filtered << "'" << std::endl;
//   // gloo::PrintStringList(list);

//   // parser.ParseAttribute(list, attrib, true);
//   // parser.ParseFace(list, face, true);

//   // PrintStringList(list);

//   return 0;
// }