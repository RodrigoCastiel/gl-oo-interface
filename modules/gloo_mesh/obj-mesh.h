// + ======================================== +
// |         gl-oo-interface library          |
// |            Module: GLOO Mesh.            |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

// ObjMesh is a high-level structure to store mesh data and its topology.

#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <initializer_list>

#include "group.h"

namespace gloo
{

// --------------------------------------------------------------------------------------

class Face
{
public:
  Face() { }

  Face(std::initializer_list<std::vector<int>> vertexList) 
  : mVertexList(vertexList)
  , mNormal({.0f, .0f, .0f})
  { }

  std::vector<std::vector<int>> & VertexList() { return mVertexList; }
  glm::vec3 GetNormal() const { return mNormal; }
  void SetNormal(const glm::vec3 & normal) { mNormal = normal; }

  const std::vector<int> & operator[](int index) const { return mVertexList[index]; }
  std::vector<int> & operator[](int index) { return mVertexList[index]; }

private:
  std::vector<std::vector<int>> mVertexList;
  glm::vec3 mNormal;
};

// --------------------------------------------------------------------------------------

struct ObjGroup
{
  std::string mName;
  std::vector<Face> mFaces;
  // TODO: mtl index.
};

// --------------------------------------------------------------------------------------

class ObjMesh
{
public:
  void Clear();

  void AddVertex(const glm::vec3 & attrib);
  void AddNormal(const glm::vec3 & attrib);
  void AddUV(const glm::vec2 & attrib);

  void AddFace(const Face & face);
  void AddGroup(const std::string & name);

  void LogData() const;

  std::vector<glm::vec3> & GetVertices() { return mVertices; }
  std::vector<glm::vec3> & GetNormals()  { return mNormals;  }
  std::vector<glm::vec2> & GetUVs()      { return mUVs;      }

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
  MeshGroup<Batch>* ExportToMeshGroup(int groupIndex, bool smoothLighting=true) const;

  // Geometry processing methods.
  
  // Triangulates all quads on the surface mesh. Other types of polygons are ignored.
  void TriangulateQuads();

  // Calculates the normal vector for each vertex on the surface.
  // The mesh must be triangulated.
  void ComputeVertexNormals();

  // Calculates the normal vector for each face. It considers the faces to be triangular.
  void ComputeFaceNormals();

private:
  // General data.
  std::vector<glm::vec3> mVertices;
  std::vector<glm::vec3> mNormals;
  std::vector<glm::vec2> mUVs;

  // List of groups.
  std::vector<ObjGroup> mGroups;

  // TODO: mtl.
};

// ----------------------------------------------------------------------------

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
void ObjMesh::AddVertex(const glm::vec3 & attrib)
{ 
  mVertices.push_back(attrib); 
}

inline 
void ObjMesh::AddNormal(const glm::vec3 & attrib)
{ 
  mNormals.push_back(attrib);
}

inline
void ObjMesh::AddUV(const glm::vec2 & attrib)
{ 
  mUVs.push_back(attrib); 
}

}  // namespace gloo.