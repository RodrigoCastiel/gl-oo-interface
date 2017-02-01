#include "obj-mesh.h"

#include <iostream>

namespace gloo
{

void ObjMesh::Clear()
{
  mVertices.clear();
  mNormals.clear();
  mUVs.clear();

  for (auto & group : mGroups )
  {
    group.mFaces.clear();
  }
}

size_t ObjMesh::GetNumFaces() const
{
  size_t count = 0;
  for (auto & group : mGroups)
  {
    count += group.mFaces.size();
  }
  return count;
}

void ObjMesh::LogData() const
{
  std::cout << "#vertices = " << mVertices.size() << std::endl;
  std::cout << "#normals  = " << mNormals.size() << std::endl;
  std::cout << "#uvs      = " << mUVs.size() << std::endl;
  std::cout << "#faces    = " << ObjMesh::GetNumFaces() << std::endl;

  std::cout << "#groups   = " << mGroups.size() << std::endl;
  for (auto & group : mGroups)
  {
    std::cout << "\t" << group.mName << std::endl;
  }
}

MeshGroup<Batch>* ObjMesh::ExportToMeshGroup(int groupIndex) const
{  
  if ((groupIndex < 0) && (groupIndex > mGroups.size())) 
  {
    // Error: invalid index.
    std::cerr << "ERROR Invalid groupIndex. " << std::endl;
    return nullptr;
  }
  const ObjGroup & objGroup = mGroups[groupIndex];

  // Check attribute list.
  bool hasNormals = (mNormals.size() > 0);
  bool hasUVs     = (mUVs.size()   > 0);

  // Count number of vertices.
  size_t numVertices = 3 * ObjMesh::GetNumFacesOnGroup(groupIndex);
  size_t numElements = numVertices;

  // Copy all data referenced on face indices into an auxiliary buffer.
  std::vector<float> groupPositions;
  std::vector<float> groupNormals;
  std::vector<float> groupUVs;

  groupPositions.reserve(3*numVertices);
  if (hasNormals)
    groupNormals.reserve(3*numVertices);

  if (hasUVs)
    groupUVs.reserve(2*numVertices);

  for (int i = 0; i < objGroup.mFaces.size(); i++)
  {
    for (int vtx = 0; vtx < 3; vtx++)
    {
      // Retrieve v, vt and vn.
      int pos_index = objGroup.mFaces[i][vtx][0];
      int  uv_index = objGroup.mFaces[i][vtx][1];
      int nor_index = objGroup.mFaces[i][vtx][2];

      groupPositions.push_back(mVertices[pos_index].x);
      groupPositions.push_back(mVertices[pos_index].y);
      groupPositions.push_back(mVertices[pos_index].z);

      if (hasNormals)
      {
        groupNormals.push_back(mNormals[nor_index].x);
        groupNormals.push_back(mNormals[nor_index].y);
        groupNormals.push_back(mNormals[nor_index].z);  
      }

      if (hasUVs)
      {
        groupUVs.push_back(mUVs[uv_index].u);
        groupUVs.push_back(mUVs[uv_index].v);
      }
    }
  }

  // Build up MeshGroup.
  std::vector<GLuint> vertexAttribList;
  std::vector<GLfloat*> bufferList;

  // Add position attrib info.
  vertexAttribList.push_back(3);
  bufferList.push_back(groupPositions.data());

  if (hasNormals)
  {
    vertexAttribList.push_back(3);
    bufferList.push_back(groupNormals.data());
  }
  if (hasUVs)
  {
    vertexAttribList.push_back(2);
    bufferList.push_back(groupUVs.data());
  }

  MeshGroup<Batch>* meshGroup = new MeshGroup<Batch>(numVertices, numElements, GL_TRIANGLES);
  meshGroup->SetVertexAttribList(vertexAttribList);

  if (!meshGroup->Load(bufferList, nullptr))
  {
    return nullptr;
  }
  else
  {
    return meshGroup;
  }
}

}  // namespace gloo.