// + ======================================== +
// |         gl-oo-interface library          |
// |         Module: GLOO Rendering.          |
// |        Author: Rodrigo Castiel, 2016.    |
// + ======================================== +
// ------------------------------------------------------------------------------------------------
// PhongRenderer is an interface for rendering objects using phong shading model or any variant.
// It provides general functionalities of any phong shader program.
// As default, it loads the following shaders:
//
//  Vertex:   "../../shaders/phong/vertex_shader.glsl"
//  Fragment: "../../shaders/phong/fragment_shader.glsl"
//
// Optionally, you can write custom shaders based on the above shaders to extend the features.
// As example, in the folder 'shaders' you can find the normal_mapping_phong shaders.
// Perhaps, you could also add more texture samplers or anything to improve your rendering.
// PhongRenderer contains several methods for querying attribute/uniform locations in an 
// optimized way (it stores in the client memory all IDs). 
// 
// This is the list of mandatory uniforms/attributes that the Phong Shader must have:
// Reference of Attributes:
//  vec3 v_position;  // Vertex position (object coordinates).
//  vec3 v_normal;    // Vertex normal   (object coordinates).
//  vec2 v_uv;        // Texture coordinates.
//
// Reference of Uniforms:
// (Vertex)
//  mat4 M;  // Model matrix.
//  mat4 V;  // View  matrix.
//  mat4 P;  // Projection matrix.
//  mat4 N;  // Normal matrix N = (VM)^-t.
//
// (Fragment)
//  int lighting = 0;                   // Light switch (toggle on/off).
//  int num_lights = 1;                 // Number of light sources.
//  int light_switch[max_num_lights];   // Array of light source states (on/off).
//  vec3 La = vec3(0.1);                // Ambient light component.
//  LightSource light[max_num_lights];  // Array of light sources.
//  sampler2D color_map;  // Color texture sampler.
//  Material material;    // Material properties (Ka, Kd, Ks).
//
// Basic Usage:
//
// 1. Construct:
//  Default: PhongRenderer* mPhongRenderer = new PhongRenderer();
//  Custom:  PhongRenderer* mPhongRenderer = new PhongRenderer(vtxShaderPath, fragShaderPath); 
// 
// 2. Load and check for errors:
//  bool success = mPhongRenderer->Load();
//
// 3. Get default shader attribute/uniform locations (please read the method declarations):
//  GLint attribLoc  = mPhongRenderer->Get<Name>AttribLoc();
//  GLint uniformLoc = mPhongRenderer->Get<Name>UniformLoc();
//  ...
//
// 4. Get custom shader attribute/uniform locations (please read the method declarations):
//  GLint customAttribLoc  = mPhongRenderer->GetAttribLocation( "<name>");
//  GLint customUniformLoc = mPhongRenderer->GetUniformLocation("<name>");
//  ...
//
// 5. Before use (i.e. rendering calls):
//  mPhongRenderer->Bind();
//
// 6. Lighting management:
//  (a) EnableLighting() or DisableLighting() to toggle on/off the lighting.
//  (b) EnableLightSource() or DisableLightSource() to toggle on/off a specific light source.
//  (c) SetNumLightSources() for setting the loop size when rendering on shader.
//  (d) SetLightSource() for setting the light source properties in world coordinates.
//  (e) SetLightSourceInCameraCoordinates() to set the light source properties in camera reference.
//
// 7. Material and texture management:
//  (a) SetMaterial() to update the current material properties.
//  (b) SetTextureUnit(name, slot) 
//     It binds the sampler name to a texture slot. It does not set the texture.
//     In order to set a texture, you should link the sampler name to a slot:
//      GLuint slot = 0;
//      mPhongRenderer->SetTextureUnit(color_map, slot);
//     And then bind your Texture* to this slot:
//      myTexture->Bind(slot);
//
// ------------------------------------------------------------------------------------------------

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
  // Load a custom phong renderer if you need it. 
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
  
  void EnableLighting() const;
  void DisableLighting() const;

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

  // === Material configuration methods ===
  void SetMaterial(const Material & material) const;

  // === Texture configuration methods ===
  void EnableColorMap() const;
  void DisableColorMap() const;

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
  GLint mLightingLoc { -1 };
  GLint mLaLoc { -1 };
  GLint mNumLightUniform { -1 };
  GLint mLightSwitchUniformArray[kMaxNumberLights];
  LightUniformPack mLightUniformArray[kMaxNumberLights];

  // Material and Texture.
  MaterialUniformPack mMaterialUniform;  // Set of material uniforms.
  GLint mUseColorMapLoc { -1 };

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

inline
void PhongRenderer::EnableLighting()  const
{
  glUniform1i(mLightingLoc, 1);
}

inline
void PhongRenderer::DisableLighting() const
{
  glUniform1i(mLightingLoc, 0);
}

inline
void PhongRenderer::EnableColorMap() const
{
  glUniform1i(mUseColorMapLoc, 1);
}

inline
void PhongRenderer::DisableColorMap() const
{
  glUniform1i(mUseColorMapLoc, 0);
}

}  // namespace gloo.
