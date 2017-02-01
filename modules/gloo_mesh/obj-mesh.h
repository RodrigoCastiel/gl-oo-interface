// + ======================================== +
// |         gl-oo-interface library          |
// |            Module: GLOO Mesh.            |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

// ObjMesh is a high-level structure to store mesh data and its topology.

#pragma once

#include <vector>
#include <string>

#include "group.h"

namespace gloo
{

struct Vec2f
{
  float u, v;
};

struct Vec3f
{
  float x, y, z;
};

// using Attribute = Vec2f;
using Face = std::vector<std::vector<int>>;

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

  void AddVertex(const Vec3f & attrib);
  void AddNormal(const Vec3f & attrib);
  void AddUV(const Vec2f & attrib);

  void AddFace(const Face & face);
  void AddGroup(const std::string & name);

  void LogData() const;

  std::vector<Vec3f> & GetVertices() { return mVertices; }
  std::vector<Vec3f> & GetNormals()  { return mNormals;  }
  std::vector<Vec2f> & GetUVs()      { return mUVs;      }

  // Getter methods.
  inline size_t GetNumVertices() const { return mVertices.size(); }
  inline size_t GetNumNormals()  const { return mNormals.size();  }
  inline size_t GetNumUVs()      const { return mUVs.size();      }

  inline size_t GetNumGroups() const { return mGroups.size(); }
  inline size_t GetNumFacesOnGroup(int index) const { return mGroups[index].mFaces.size(); }
  size_t GetNumFaces() const;

  // Builds a renderable mesh group from the topology data of ObjMesh.
  // If an invalid groupIndex is passed, returns nullptr.
  //
  // It assumes that ObjMesh is triangulated.
  // The returned MeshGroup will contain all the geometry for rendering, 
  // but no texture or material info.
  //
  // Automatically sets the internal list of vertex attributes to {3, 3, 2} -> {pos, normal, uv}.
  // You must add manually a rendering pass.
  //
  // Possible source of segfault: providing texture coordinates or normals 
  // but not providing its indices.
  MeshGroup<Batch>* ExportToMeshGroup(int groupIndex) const;


  // Geometry processing methods.
  void TessellateQuads();

private:
  // General data.
  std::vector<Vec3f> mVertices;
  std::vector<Vec3f> mNormals;
  std::vector<Vec2f> mUVs;

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
void ObjMesh::AddVertex(const Vec3f & attrib)
{ 
  mVertices.push_back(attrib); 
}

inline 
void ObjMesh::AddNormal(const Vec3f & attrib)
{ 
  mNormals.push_back(attrib); 
}

inline
void ObjMesh::AddUV(const Vec2f & attrib)
{ 
  mUVs.push_back(attrib); 
}

}  // namespace gloo.