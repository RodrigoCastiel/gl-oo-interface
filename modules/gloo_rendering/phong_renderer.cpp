#include "phong_renderer.h"

#define GLM_SWIZZLE
#include "gloo/gl_header.h"

#define LOG_OUTPUT_ON 1

namespace gloo
{

PhongRenderer::~PhongRenderer()
{
  delete mPhongShader;
}

bool PhongRenderer::Load()
{
  // Allocate shader.
  mPhongShader = new ShaderProgram();

  // Load and build it.
  mPhongShader->BuildFromFiles(mVertexShaderPath, mFragmentShaderPath);

  // Check if compilation was successful.
  gloo::CompilationStatus status = mPhongShader->GetCompilationStatus();

  if (status == gloo::CompilationStatus::kSuccess)
  {
    mPhongShader->Bind();

    // Get uniform/attribute locations.
    mPositionAttribLoc = mPhongShader->GetAttribLocation("v_position");
    mNormalAttribLoc   = mPhongShader->GetAttribLocation("v_normal");
    mTextureAttribLoc  = mPhongShader->GetAttribLocation("v_uv");
    mTangentAttribLoc  = mPhongShader->GetAttribLocation("v_tangent");

    mProjMatrixLoc   = mPhongShader->GetUniformLocation("P");
    mViewMatrixLoc   = mPhongShader->GetUniformLocation("V");
    mModelMatrixLoc  = mPhongShader->GetUniformLocation("M");
    mNormalMatrixLoc = mPhongShader->GetUniformLocation("N");

    // Pre-load material uniform pack.
    mMaterialUniform.mKaLoc = mPhongShader->GetUniformLocation("material.Ka");
    mMaterialUniform.mKdLoc = mPhongShader->GetUniformLocation("material.Kd");
    mMaterialUniform.mKsLoc = mPhongShader->GetUniformLocation("material.Ks");
    mUseColorMapLoc = mPhongShader->GetUniformLocation("use_color_map");

    // Pre-load light uniform packs.
    mLightingLoc = mPhongShader->GetUniformLocation("lighting");
    mNumLightUniform = mPhongShader->GetUniformLocation("num_lights");
    mLaLoc = mPhongShader->GetUniformLocation("La");

    for (int i = 0; i < kMaxNumberLights; i++)
    {
      std::string light_prefix = "light[" + std::to_string(i) + "].";

      mLightUniformArray[i].mPosLoc   = mPhongShader->GetUniformLocation(light_prefix + "pos");
      mLightUniformArray[i].mDirLoc   = mPhongShader->GetUniformLocation(light_prefix + "dir");
      mLightUniformArray[i].mLdLoc    = mPhongShader->GetUniformLocation(light_prefix + "Ld");
      mLightUniformArray[i].mLsLoc    = mPhongShader->GetUniformLocation(light_prefix + "Ls");
      mLightUniformArray[i].mAlphaLoc = mPhongShader->GetUniformLocation(light_prefix + "alpha");

      mLightSwitchUniformArray[i] = mPhongShader->GetUniformLocation("light_switch[" + std::to_string(i) + "]");
    }

    return true;
  }
  else 
  {
    mPhongShader->PrintCompilationLog();
    return false;
  }
}

void PhongRenderer::Bind(int renderingPass)
{
  if (mPhongShader) 
  {
    mPhongShader->Bind();
  }
}

GLint PhongRenderer::GetAttribLocation(const std::string & name, int renderingPass) const
{
  if (name == "position" || name == "v_position")
  {
    return mPositionAttribLoc;
  }
  else if (name == "texture" || name == "v_uv" || name == "uv")
  {
    return mTextureAttribLoc;
  }
  else if (name == "normal" || name == "v_normal")
  {
    return mNormalAttribLoc;
  }
  else if (name == "tangent" || name == "v_tangent")
  {
    return mTangentAttribLoc;
  }
  else  // Search it up.
  {
    return mPhongShader->GetAttribLocation(name);
  }
}

GLint PhongRenderer::GetUniformLocation(const std::string & name, int renderingPass) const
{
  if (name == "M" || name == "model")
  {
    return mModelMatrixLoc;
  }
  else if (name == "V" || name == "view")
  {
    return mViewMatrixLoc;
  }
  else if (name == "P" || name == "projection" || name == "proj")
  {
    return mProjMatrixLoc;
  }
  else if (name == "N" || name == "normal")
  {
    return mNormalMatrixLoc;
  }
  else // Search it up.
  {
    return mPhongShader->GetUniformLocation(name);
  }
}

namespace 
{
  void SetUniform3f(GLint loc, const glm::vec3 & data)
  {
    if (loc != kNoUniform)
    {
      glUniform3f(loc, data[0], data[1], data[2]);
    }
  }

  void SetUniform1f(GLint loc, GLfloat data)
  {
    if (loc != kNoUniform)
    {
      glUniform1f(loc, data);
    }
  }
}

void PhongRenderer::SetLightAmbientComponent(const glm::vec3 & La) const
{
  SetUniform3f(mLaLoc, La);
}

void PhongRenderer::SetLightSource(const LightSource & lightSource, int slot) const
{
  const LightUniformPack & lightSourceUniform = mLightUniformArray[slot];

  SetUniform3f(lightSourceUniform.mPosLoc, lightSource.mPos);  // Position.
  SetUniform3f(lightSourceUniform.mDirLoc, lightSource.mDir);  // Direction.
  SetUniform3f(lightSourceUniform.mLdLoc,  lightSource.mLd);  // Diffuse component.
  SetUniform3f(lightSourceUniform.mLsLoc,  lightSource.mLs);  // Specular component.
  SetUniform1f(lightSourceUniform.mAlphaLoc, lightSource.mAlpha);  // Shininess.
}

void PhongRenderer::SetLightSourceInCameraCoordinates(const LightSource & lightSource, 
                                                      const Camera * camera, int slot) const
{
  const LightUniformPack & lightSourceUniform = mLightUniformArray[slot];

  // Transform position/direction into camera coordinates.
  glm::vec4 p = glm::vec4(lightSource.mPos, 1.0f);
  glm::vec4 d = glm::vec4(lightSource.mDir, 1.0f);
  const glm::mat4 & V = camera->ViewTransform().GetMatrix();

  p = V * p;
  d = V * d;
  p = p / p[3];  // Normalize homogenous coordinates.

  SetUniform3f(lightSourceUniform.mPosLoc, p.xyz());  // Position.
  SetUniform3f(lightSourceUniform.mDirLoc, d.xyz());  // Direction.
  SetUniform3f(lightSourceUniform.mLdLoc,  lightSource.mLd);  // Diffuse component.
  SetUniform3f(lightSourceUniform.mLsLoc,  lightSource.mLs);  // Specular component.
  SetUniform1f(lightSourceUniform.mAlphaLoc, lightSource.mAlpha);  // Shininess.
}

void PhongRenderer::SetMaterial(const Material & material) const
{
  SetUniform3f(mMaterialUniform.mKaLoc,  material.mKa);  // Ambient component.
  SetUniform3f(mMaterialUniform.mKdLoc,  material.mKd);  // Diffuse component.
  SetUniform3f(mMaterialUniform.mKsLoc,  material.mKs);  // Specular component.
}

}  // namespace gloo.