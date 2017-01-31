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

void ObjMesh::LogData() const
{
  std::cout << "#vertices = " << mVertices.size() << std::endl;
  std::cout << "#normals  = " << mNormals.size() << std::endl;
  std::cout << "#uvs      = " << mUVs.size() << std::endl;
  
  std::cout << "#groups   = " << mGroups.size() << std::endl;
  for (auto & group : mGroups)
  {
    std::cout << "\t" << group.mName << std::endl;
  }
}

}  // namespace gloo.