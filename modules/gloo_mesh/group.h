// + ======================================== +
// |         gl-oo-interface library          |
// |            Module: GLOO Mesh.            |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

// ============================================================================================= //
// Group
// ============================================================================================= //
// 3D-surface meshes are made up by groups, which are parts that share the same
// materials or textures.
// Group has the following features:

// [StorageFormat]
//
// Each group has a single vertex buffer in GPU, which can follow two kinds of storage:
// 1. Interleaved (Tighly Packed):  (P N T) (P N T) ... (P N T)
// 2. Batched (Sub-Buffered):       (P P ... P) (N N ... N) (T T ... T)
// Where P is the vertex positions array, N is the vertex normals array and so on.
//
// The storage format is provided by the template parameter <StorageFormat F>. 

// [Vertex Attribute Data]
//
// Vertex attribute list (or data) basically defines which properties a vertex contains.
// A vertex attribute list must be specified to a group so all data can be successfully passed
// to the shader during rendering or updating methods.
//
// For example, the list {3, 3, 2} defines three attributes: the first contains three floats,
// the second contains 3 floats and the last one has 2 floats (typically 3d coordinates, 
// normal vector and uv texture coordinates).
// Vertex attribute data is specified by calling SetVertexAttribList().

// [Rendering Pass]
// A single mesh group can be rendered in different ways and in multiple passes.
// In this way, MeshGroup provides a method to create a rendering pass by specifying
// which attributes will be enabled or not and what are their locations in the shader.
//
// The location in shaders must be coeherent to attributes you've written on your shader.   

// [Loading/updating data]
// 
// In order to load data to GPU buffers, you can either pass as parameter the raw buffer
// containing all arranged data or pass a list of separate buffers containing per attribute
// data.
// You can do it using the two overloaded versions of Load().
// If you don't want to provide elements, just pass nullptr to 'indices' parameter and Load()
// will automatically build the element array for you (following the default order).
//
// You can either load/update the geometry from a single buffer containing all vertex data or load from a
// list of buffers, each one corresponding to an attribute. 
// When updating, you can optionally pass nullptr for attributes you don't want to update.

// [USAGE]
/*
    // Create.
    MeshGroup<Batch>* group = new MeshGroup<Batch>(numVertices, numElements, GL_TRIANGLES);
 
    // Specify vertex attribute data.
    // 3 attributes: the first has 3 floats, the second has 3 floats and the third has 2 floats.
    group->SetVertexAttribList({3, 3, 2});

    // Create a rendering pass.
    group->AddRenderingPass({{posAttribLoc, true}, kNoAttrib, kNoAttrib});
    group->AddRenderingPass({{posAttribLoc, true}, {normalAttribLoc, true}, {uvAttribLoc, true}});  // Phong.

    // Load data into GPU (from separate buffers).
    group->Load({positions.data(), normals.data(), uv.data()}, indices.data());
    ...
    // Render!
    group->Render();
    ...
    // Don't forget to delete it.
    delete group;
*/

// IMPORTANT:
// If you want to use a group in different shader programs, you should 
// make sure that all of them have the same attribute location (typically,
// loc=0 for position, loc=1 for normal and so on).
//
// ============================================================================================= //

#pragma once

#include "gloo/gl_header.h"
#include <vector>
#include <initializer_list>
#include <cassert>

namespace gloo
{

// StorageFormat specifies how geometry data is stored into the GPU buffer.
enum StorageFormat
{
  Interleave,  // (Tighly Packed):  (P N T) (P N T) ... (P N T)
  Batch,       // (Sub-Buffered):   (P P ... P) (N N ... N) (T T ... T)
};

const std::pair<GLint, bool> kNoAttrib = {-1, false};

template <StorageFormat F>
class MeshGroup
{
public:
  /* Most used methods */
  // TODO: add options for draw mode and buffer data usage.
  MeshGroup(int numVertices, int numElements, GLenum drawMode = GL_TRIANGLE_STRIP,
            GLenum dataUsage = GL_STATIC_DRAW);

  ~MeshGroup();

  // Specifies which data/properties the vertices contain.
  void SetVertexAttribList(std::initializer_list<GLuint> vertexAttribList);
  void SetVertexAttribList(const std::vector<GLuint> & vertexAttribList);

  // Adds a different way of rendering the object - each one might use different 
  // attributes of the vertex. The active attribute list specifies which attributes 
  // are enabled and their corresponding shader locations.
  int AddRenderingPass(const std::vector<std::pair<GLint, bool>> & attribList);

  // Should be called on display function (it calls glDrawElements).
  void Render(unsigned renderingPass = 0) const;

  // TODO: document.
  bool Load(const GLfloat* buffer, const GLuint* indices);
  bool Load(const std::vector<GLfloat*> & bufferList, const GLuint* indices);

  // TODO: document.
  bool Update(const GLfloat* buffer);
  bool Update(const std::vector<GLfloat*> & bufferList);

  // Generate buffers on GPU (VAO, VBO, EAB).
  void AllocateBuffers(const GLfloat* vertices, const GLuint* elements);

  // Destroys buffers on GPU (VAO, VBO, EAB).
  void ClearBuffers();

  // Getters.
  std::vector<GLuint> GetVertexAttribList() const { return mVertexAttributeList; }
  GLuint GetNumVertices() const { return mNumVertices; }
  GLuint GetNumElements() const { return mNumElements; }
  GLuint GetVertexSize() const  { return mVertexSize;  }
  GLenum GetDataUsage() const { return mDataUsage; }
  GLenum GetDrawMode()  const { return mDrawMode;  }

  // Setters.
  void SetDrawMode(GLenum drawMode) { mDrawMode = drawMode; }

private:
  // Specifies vertex attribute object (how attributes are spatially stored into VBO and
  // mapped to attribute locations on shader).
  void BuildVAO(const std::vector<std::pair<GLint, bool>> & attribList);

  /* Attributes */

  // OpenGL buffer IDs.
  GLuint mEab { 0 };  // Element array buffer.
  GLuint mVbo { 0 };  // Vertex buffer object.
  std::vector<GLuint> mVaoList;  // Verter array object list.

  // Mesh attributes.
  GLenum mDrawMode;     // How mesh is rendered (drawing mode).
  GLenum mDataUsage;    // Tells if buffers are dynamic/static or for reading/writing.

  GLuint mNumVertices;  // Number of vertices in this group.
  GLuint mNumElements;  // Number of elements (indices of vertex).

  GLuint mVertexSize    { 0 };  // Number of floating points stored per vertex.
  GLuint mNumAttributes { 0 };  // Number of attributes.

  // Vertex attributes descriptor -> specifies which attributes a vertex contain and also
  // their dimensionality and order. This is constant within the lifetime of a MeshGroup.
  std::vector<GLuint> mVertexAttributeList;
};

// ============================================================================================ //
// Implementation of template functions.

/* Constructor */
template <StorageFormat F>
MeshGroup<F>::MeshGroup(int numVertices, int numElements, GLenum drawMode, GLenum dataUsage)
: mNumVertices(numVertices)
, mNumElements(numElements)
, mDataUsage(dataUsage)
, mDrawMode(drawMode)
{

}

/* Destructor */
template <StorageFormat F>
MeshGroup<F>::~MeshGroup() 
{
  MeshGroup<F>::ClearBuffers();
}

/* Rendering method */
template <StorageFormat F>
void MeshGroup<F>::Render(unsigned renderingPass) const
{
  assert((renderingPass >= 0) && (renderingPass < mVaoList.size()));

  const int option = renderingPass;
  
  glBindVertexArray(mVaoList[option]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEab);

  glDrawElements(
    mDrawMode,         // mode (GL_LINES, GL_TRIANGLES, ...)
    mNumElements,      // number of vertices.
    GL_UNSIGNED_INT,   // type.
    (void*)0           // element array buffer offset.
   );
}

template <StorageFormat F>
void MeshGroup<F>::SetVertexAttribList(std::initializer_list<GLuint> vertexAttribList)
{
  mVertexAttributeList = std::vector<GLuint>(vertexAttribList);

  mVertexSize = 0;
  mNumAttributes = 0;
  for (GLuint attribSize : mVertexAttributeList) 
  {
    mVertexSize += attribSize;
    mNumAttributes++;
  }

  // Generate geometry buffers.
  glGenBuffers(1, &mVbo);       // Vertex buffer object.
  glGenBuffers(1, &mEab);       // Element array buffer.
}

template <StorageFormat F>
void MeshGroup<F>::SetVertexAttribList(const std::vector<GLuint> & vertexAttribList)
{
  mVertexAttributeList = vertexAttribList;

  mVertexSize = 0;
  mNumAttributes = 0;
  for (GLuint attribSize : mVertexAttributeList) 
  {
    mVertexSize += attribSize;
    mNumAttributes++;
  }

  // Generate geometry buffers.
  glGenBuffers(1, &mVbo);       // Vertex buffer object.
  glGenBuffers(1, &mEab);       // Element array buffer.
}

template <StorageFormat F>
int MeshGroup<F>::AddRenderingPass(const std::vector<std::pair<GLint, bool>> & attribList)
{
  assert(attribList.size() == mNumAttributes);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEab);
  
  MeshGroup<F>::BuildVAO(attribList);

  for (int i = 0; i < mNumAttributes; i++)
  {
    const GLuint attribLoc = attribList[i].first;
    const bool active = attribList[i].second;

    // If the attribute is active...
    if (active)
    {
      glEnableVertexAttribArray(attribLoc);
    }
    else if (attribLoc != -1)
    {
      glDisableVertexAttribArray(attribLoc);
    }
  }

  mVaoList.push_back(vao);

  return mVaoList.size()-1;
}

/* Generate buffers */
template <StorageFormat F>
void MeshGroup<F>::AllocateBuffers(const GLfloat* vertices, const GLuint* elements)
{  
  // Allocate buffer for elements (EAB).
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEab);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumElements * sizeof(GLuint),
               elements, GL_STATIC_DRAW);

  // Allocate buffer for vertices (VBO).
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBufferData(GL_ARRAY_BUFFER, mVertexSize * mNumVertices * sizeof(GLfloat),
               vertices, mDataUsage);
}

/* Delete buffers */
template <StorageFormat F>
void MeshGroup<F>::ClearBuffers()
{
  glDeleteBuffers(1, &mVbo);
  glDeleteBuffers(1, &mEab);
  glDeleteVertexArrays(mVaoList.size(), mVaoList.data());
}

template <StorageFormat F>
bool MeshGroup<F>::Load(const GLfloat* buffer, const GLuint* indices)
{
  // Reserve vertex buffer and initialize element array (indices array).
  if (indices)  // Element array provided.
  {
    MeshGroup<F>::AllocateBuffers(buffer, indices);
  }
  else  // Element array wasn't provided -- build it up.
  {
    std::vector<GLuint> elementsBuffer(mNumElements);

    for (int i = 0; i < mNumElements; i++)
      elementsBuffer[i] = i;

    MeshGroup<F>::AllocateBuffers(buffer, elementsBuffer.data());
  }

  return true;
}

template <StorageFormat F>
bool MeshGroup<F>::Update(const GLfloat* buffer)
{
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, mVertexSize * mNumVertices * sizeof(GLfloat), buffer);
  return true;
}

// ============================================================================================= //
// Specializations for different StorageFormats.

// ================ Batched Storage =================== //

template <>
bool MeshGroup<Batch>::Load(const std::vector<GLfloat*> & bufferList, const GLuint* indices);

template <>
bool MeshGroup<Batch>::Update(const std::vector<GLfloat*> & bufferList);


template <>
void MeshGroup<Batch>::BuildVAO(const std::vector<std::pair<GLint, bool>> & attribList);

// ================ Interleaved Storage ==================== //

template <>
bool MeshGroup<Interleave>::Load(const std::vector<GLfloat*> & bufferList, const GLuint* indices);

template <>
bool MeshGroup<Interleave>::Update(const std::vector<GLfloat*> & bufferList);


template <>
void MeshGroup<Interleave>::BuildVAO(const std::vector<std::pair<GLint, bool>> & attribList);


}  // namespace gloo.