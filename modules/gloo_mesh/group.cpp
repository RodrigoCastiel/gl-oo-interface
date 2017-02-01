#include "group.h"
#include <cassert>

namespace gloo
{

template <>
bool MeshGroup<Interleave>::Load(const std::vector<GLfloat*> & bufferList, const GLuint* indices)
{
  assert(bufferList.size() == mNumAttributes);

  // Create temporary buffers to transfer geometry and elements to GPU.
  std::vector<GLfloat> vertexBuffer;
  vertexBuffer.reserve(mNumVertices * mVertexSize);

  // Copy geometry to temporary buffers in memory.
  for (int i = 0; i < mNumVertices; i++)
  {
    // For each attribute,
    for (int j = 0; j < mNumAttributes; j++)
    {
      const GLuint size   = mVertexAttributeList[j];
      const float* buffer = bufferList[j];

      if ((size > 0) && (buffer != nullptr))
      {
        // Add elements (coordinate, normals, ...).
        for (int k = 0; k < size; k++) 
        {
          vertexBuffer.push_back(buffer[size*i + k]);
        }
      }
    }
  }

  // Reserve vertex buffer and initialize element array (indices array).
  if (indices)  // Element array provided.
  {
    MeshGroup<Interleave>::AllocateBuffers(vertexBuffer.data(), indices);
  }
  else  // Element array wasn't provided -- build it up.
  {
    std::vector<GLuint> elementsBuffer(mNumElements);

    for (int i = 0; i < mNumElements; i++)
      elementsBuffer[i] = i;

    MeshGroup<Interleave>::AllocateBuffers(vertexBuffer.data(), elementsBuffer.data());
  }

  return true;
}

template <>
void MeshGroup<Interleave>::BuildVAO(const std::vector<std::pair<GLint, bool>> & attribList)
{
  // Specify VAO.
  int offset = 0;
  GLfloat stride = sizeof(GLfloat) * mVertexSize;
  for (int j = 0; j < mNumAttributes; j++)
  {
    const int size    = mVertexAttributeList[j];
    const GLint loc   = attribList[j].first;
    const bool active = attribList[j].second;

    if ((size > 0) && active)
    {
      // Specify internal storage architecture of Vertex Buffer.
      glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, 
        (void*)(sizeof(GLfloat) * offset));
    }

    offset += size;
  }
}

template <>
bool MeshGroup<Interleave>::Update(const std::vector<GLfloat*> & bufferList)
{
  assert(bufferList.size() == mNumAttributes);

  glBindBuffer(GL_ARRAY_BUFFER, mVbo);

  // Upload subdata of geometry to GPU.
  int attrib_offset = 0;
  for (int j = 0; j < mNumAttributes; j++)
  {
    const unsigned size = mVertexAttributeList[j];
    const float* buffer = bufferList[j];

    if ((size > 0) && (buffer != nullptr))
    {
      for (int i = 0; i < mNumVertices; i++)
      {
        glBufferSubData(GL_ARRAY_BUFFER,
                        (mVertexSize*i + attrib_offset)*sizeof(GLfloat), // Offset.
                        size*sizeof(GLfloat), &buffer[size*i]);          // Size.
      }
    }

    attrib_offset += size;
  }

  return true;
}

// ============================================================================================= //

template <>
bool MeshGroup<Batch>::Load(const std::vector<GLfloat*> & bufferList, const GLuint* indices)
{
  assert(bufferList.size() == mNumAttributes);

  // Reserve vertex buffer and initialize element array (indices array).
  if (indices)  // Element array provided.
  {
    MeshGroup<Batch>::AllocateBuffers(nullptr, indices);
  }
  else  // Element array wasn't provided -- build it up.
  {
    std::vector<GLuint> elementsBuffer(mNumElements);

    for (int i = 0; i < mNumElements; i++)
      elementsBuffer[i] = i;

    MeshGroup<Batch>::AllocateBuffers(nullptr, elementsBuffer.data());
  }

  MeshGroup<Batch>::Update(bufferList);

  return true;
}

template <>
bool MeshGroup<Batch>::Update(const std::vector<GLfloat*> & bufferList)
{
  assert(bufferList.size() == mNumAttributes);

  glBindBuffer(GL_ARRAY_BUFFER, mVbo);

  // Upload subdata of geometry to GPU.
  int offset = 0;
  for (int j = 0; j < mNumAttributes; j++)
  {
    const unsigned size = mVertexAttributeList[j];
    const float* buffer = bufferList[j];

    if ((size > 0) && (buffer != nullptr))
    {
      glBufferSubData(GL_ARRAY_BUFFER, offset, size*mNumVertices*sizeof(GLfloat), buffer);
    }

    offset += size*mNumVertices * sizeof(GLfloat);
  }

  return true;
}

template <>
void MeshGroup<Batch>::BuildVAO(const std::vector<std::pair<GLint, bool>> & attribList)
{
  // Specify VAO.
  int offset = 0;
  for (int j = 0; j < mNumAttributes; j++)
  {
    const int size    = mVertexAttributeList[j];
    const GLint loc   = attribList[j].first;
    const bool active = attribList[j].second;

    if ((size > 0) && active)
    {
      // Specify internal storage architecture of Vertex Buffer.
      glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, 0, 
        (void*)(sizeof(GLfloat) * offset*mNumVertices));
    }

    offset += size;
  }
}

// ============================================================================================= //


}  // namespace gloo.