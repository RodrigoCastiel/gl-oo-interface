// + ======================================== +
// |         gl-oo-interface library          |
// |            Module: GLOO Mesh.            |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

#pragma once

#include <string>
#include <vector>

namespace gloo
{

namespace atomic 
{
  const std::string COMMENT = "#";  // Comment line marker.
  const std::string V       = "v";  // Vertex.
  const std::string VT      = "vt"; // Vertex texture.
  const std::string VN      = "vn"; // Vertex normal.
  const std::string F       = "f";  // Face.
  const std::string G       = "g";  // Group.
  const std::string EMPTY = "";
  const std::string SPACE = " ";
}  // namespace atomic

class ObjParser
{
public:
  struct Attribute
  { 
    std::vector<float> mData;
  };

  struct Face
  {
    std::vector<std::vector<int>> mIndices;
    std::vector<int> mVertexFormat;
  };

  // Splits the input string into a list of strings according to a separator string.
  void SplitByString(const std::string & input, const std::string & separator, 
                            std::vector<std::string> & stringList, bool removeEmptyComponents);

  // Filters repeated spaces.
  void PreprocessLine(const std::string & rawLine, std::string & outputLine);

  // Reads the line components and converts them into attribute data.
  // Returns false if the line is invalid. 
  // Otherwise, it returns true and stores the data into attrib.
  bool ParseAttribute(const std::vector<std::string> & components, Attribute & attrib, 
                      bool verbose = false);

  // Reads the line component and converts them into face data (list of vertices, normal and uv).
  // Returns false if the line is invalid. 
  // Otherwise, it returns true and stores the data into face.
  bool ParseFace(const std::vector<std::string> & components, Face & face,
                 bool verbose = false);

  // static void SetOutputText(bool output) { mOutputText = output; }
};

}  // namespace gloo.
