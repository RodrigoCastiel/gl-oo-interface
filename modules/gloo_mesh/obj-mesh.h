// + ======================================== +
// |         gl-oo-interface library          |
// |            Module: GLOO Mesh.            |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

// ObjMesh is a high-level structure to store mesh data and its topology.

#include <vector>
#include <string>

using Attribute = std::vector<float>;
using Face = std::vector<std::vector<int>>;

namespace gloo
{

struct ObjGroup
{
  std::string mName;
  std::vector<Face> mFaces;
  // TODO: mtl index.
};

class ObjMesh
{
public:

  void Clear();

  void AddVertex(const Attribute & attrib);
  void AddNormal(const Attribute & attrib);
  void AddUV(const Attribute & attrib);

  void AddFace(const Face & face);
  void AddGroup(const std::string & name);

  void LogData() const;

private:
  // General data.
  std::vector<Attribute> mVertices;
  std::vector<Attribute> mNormals;
  std::vector<Attribute> mUVs;

  // The following integers specify the size of each attribute.
  // This information is used when converting into a renderable mesh.
  int mVertexSize { 4 };
  int mNormalSize { 4 };
  int mUVSize { 3 };

  // List of groups.
  std::vector<ObjGroup> mGroups;

  // TODO: mtl.
};

inline
void ObjMesh::AddFace(const Face & face)
{
  // If no group was created previously -> create a default one.
  if (mGroups.size() == 0)  
  {
    ObjMesh::AddGroup("default");
  }

  mGroups.back().mFaces.push_back(face);
}

inline
void ObjMesh::AddGroup(const std::string & name)
{
  mGroups.push_back({name, {}});
}

inline
void ObjMesh::AddVertex(const Attribute & attrib)
{ 
  mVertices.push_back(attrib); 
}

inline 
void ObjMesh::AddNormal(const Attribute & attrib)
{ 
  mNormals.push_back(attrib); 
}

inline
void ObjMesh::AddUV(const Attribute & attrib)
{ 
  mUVs.push_back(attrib); 
}

}  // namespace gloo.