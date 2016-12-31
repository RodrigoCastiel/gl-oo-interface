// + ======================================== +
// |         gl-oo-interface library          |
// |         Module: GLOO Rendering.          |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +

// PhongRenderer is an interface for rendering objects using phong shading model or any variant.
// It provides general functionalities to any phong shader program, such as the following attri-
// butes and uniforms:
//
// Attributes: vertex position, normal and texture coordinates.
// Uniform: matrices, light sources, texture and material.

// Usage:
// TODO.
//

#pragma once 

#include <string>

#include "light.h"
#include "renderer.h"

#include "gloo/material.h"
#include "gloo/group.h"
#include "gloo/camera.h"

namespace gloo 
{

const int kMaxNumberLights = 8;

class PhongRenderer : public Renderer
{
public:
  PhongRenderer(const std::string & vertexShaderPath,
                const std::string & fragmentShaderPath)
  : Renderer()
  , mVertexShaderPath(vertexShaderPath)
  , mFragmentShaderPath(fragmentShaderPath)
  { }

  PhongRenderer()
  : Renderer()
  , mVertexShaderPath(  "../../shaders/phong/vertex_shader.glsl")
  , mFragmentShaderPath("../../shaders/phong/fragment_shader.glsl")
  { }

  ~PhongRenderer();

  // Load initializes all uniform/attribute locations for fast access.
  // Please call it after allocating a new PhongRenderer instance.
  bool Load();

  // If you're lazy to manually set the camera and then render the mesh, just call this method.
  // Please notice that by setting the camera before every object rendering, you will be updating
  // both M and V matrices without really needing.
  template <StorageFormat F>
  void Render(const MeshGroup<F>* mesh, const Transform & model, Camera* camera, int pass=0) const;

  // Call this method if you want to render a single object without setting the camera.
  // It will automatically set both model and normal matrices.
  template <StorageFormat F>
  void Render(const MeshGroup<F>* mesh, const Transform & model, int pass=0) const;

  // Call bind before using PhongRenderer. Internally, it calls glUseProgram().
  virtual void Bind(int renderingPass = 0);

  inline unsigned GetNumRenderingPasses() const { return 1; }
  inline const ShaderProgram* GetShaderProgram(int renderingPass = 0) const { return mPhongShader; }

  GLint GetAttribLocation( const std::string & name, int renderingPass = 0) const;
  GLint GetUniformLocation(const std::string & name, int renderingPass = 0) const;

  // Extra methods (fast access).
  GLint GetPositionAttribLoc() const { return mPositionAttribLoc; }
  GLint GetTextureAttribLoc()  const { return mTextureAttribLoc;  }
  GLint GetNormalAttribLoc()   const { return mNormalAttribLoc;   }
  GLint GetTangentAttribLoc()  const { return mTangentAttribLoc;  }

  GLint GetViewUniformLoc()   const { return mViewMatrixLoc; }
  GLint GetProjUniformLoc()   const { return mProjMatrixLoc; }
  GLint GetModelUniformLoc()  const { return mModelMatrixLoc;  }
  GLint GetNormalUniformLoc() const { return mNormalMatrixLoc; }

  GLint GetLightAmbientComponentLoc() const { return mLaLoc; }
  LightUniformPack GetLightSourceUniformLoc(int slot) const { return mLightUniformArray[slot]; }
  MaterialUniformPack GetMaterialUniformLoc() const { return mMaterialUniform; }

  // Geometric transformation methods.
  void SetCamera(const Camera* camera) const;
  void SetModelNormalMatrix(const Transform & model) const;

  // === Lighting configuration methods ===
  
  // Activate light source on shader.
  void EnableLightSource(int slot)  const;
  // Deactivate light source on shader.
  void DisableLightSource(int slot) const;

  // Change number of light sources on shader (for rendering optimization).
  void SetNumLightSources(int numLightSources) const;

  // Set light source/light properties.
  void SetLightAmbientComponent(const glm::vec3 & La) const;
  void SetLightSource(const LightSource & lightSource, int slot) const;
  void SetLightSourceInCameraCoordinates(const LightSource & lightSource, const Camera * camera, int slot) const;

  // Material configuration methods.
  void SetMaterial(const Material & material) const;

  // Texture configuration methods.

  // Use the following methods to link a texture unit to a sampler on shader.
  // samplerName is your sampler uniform name on the shader.
  // slot is the number of the texture unit.
  void SetTextureUnit(const char * samplerName, GLuint slot) const;
  void SetTextureUnit(const std::string & samplerName, GLuint slot) const;

private:
  // Shader Program.
  ShaderProgram* mPhongShader { nullptr };

  // Fast-access attribute/uniform locations.
  GLint mPositionAttribLoc { -1 };
  GLint mTextureAttribLoc  { -1 };
  GLint mNormalAttribLoc   { -1 };
  GLint mTangentAttribLoc  { -1 };

  GLint mViewMatrixLoc   { -1 };
  GLint mProjMatrixLoc   { -1 };
  GLint mModelMatrixLoc  { -1 };
  GLint mNormalMatrixLoc { -1 };

  // Lighting.  
  GLint mLaLoc { -1 };
  GLint mNumLightUniform { -1 };
  GLint mLightSwitchUniformArray[kMaxNumberLights];
  LightUniformPack mLightUniformArray[kMaxNumberLights];

  // Material.
  MaterialUniformPack mMaterialUniform;  // Set of material uniforms.

  // Texture.


  // Constant data (passed to constructor).
  const std::string mVertexShaderPath;
  const std::string mFragmentShaderPath;
};

// ----- Rendering methods ------------------------------------------------------------------------

template <StorageFormat F>
void PhongRenderer::Render(const MeshGroup<F>* mesh, const Transform & model, Camera* camera, 
                           int pass) const
{
  PhongRenderer::SetModelNormalMatrix(model);
  camera->SetUniformViewMatrix(mViewMatrixLoc);
  mesh->Render(pass);
}

template <StorageFormat F>
void PhongRenderer::Render(const MeshGroup<F>* mesh, const Transform & model, int pass) const
{
  PhongRenderer::SetModelNormalMatrix(model);
  mesh->Render(pass);
}

// ----- Inline methods ---------------------------------------------------------------------------

inline
void PhongRenderer::SetTextureUnit(const char * samplerName, GLuint slot) const
{
  glUniform1i(mPhongShader->GetUniformLocation(samplerName), slot);
}

inline
void PhongRenderer::SetTextureUnit(const std::string & samplerName, GLuint slot) const
{
  PhongRenderer::SetTextureUnit(samplerName.c_str(), slot);
}

inline
void PhongRenderer::SetCamera(const Camera* camera) const
{
  camera->SetUniformProjMatrix(mProjMatrixLoc);
  camera->SetUniformViewMatrix(mViewMatrixLoc);
}

inline
void PhongRenderer::SetModelNormalMatrix(const Transform & model) const
{
  model.SetUniform(mModelMatrixLoc);
  model.SetInverseTransposeUniform(mNormalMatrixLoc);
}


inline
void PhongRenderer::SetNumLightSources(int numLightSources) const
{
  // Make sure that (0 <= numLightSources <= kMaxNumberLights).
  numLightSources = std::max(0, std::min(kMaxNumberLights, numLightSources));
  glUniform1i(mNumLightUniform, numLightSources);
}

inline
void PhongRenderer::EnableLightSource(int slot)  const
{
  glUniform1i(mLightSwitchUniformArray[slot], 1);
}

inline
void PhongRenderer::DisableLightSource(int slot) const
{
  glUniform1i(mLightSwitchUniformArray[slot], 0);
}

}  // namespace gloo.