#include <gloo/light.h>
#include <gloo/group.h>
#include <gloo/camera.h>
#include <gloo/texture.h>
#include <gloo/material.h>
#include <gloo/model_base.h>
#include <gloo/useful_meshes.h>
#include <gloo/shader_program.h>
#include <gloo/debug_renderer.h>
#include <gloo/phong_renderer.h>

using namespace gloo;

class MyModel : public ModelBase
{
public:
  MyModel();
  virtual ~MyModel();

  virtual bool Init();

  // -- Essential Callbacks --
  virtual void Idle();
  virtual void Display();
  virtual void Reshape(int w, int h);

  // -- Mouse Callbacks --
  virtual void  ActiveMouseMotion(const MouseEvent & mouseEvent);
  virtual void PassiveMouseMotion(const MouseEvent & mouseEvent);
  virtual void  MouseButtonChange(const MouseEvent & mouseEvent);

  // -- Keyboard Callbacks --
  virtual void KeyboardChange(unsigned char key, int x, int y);
  virtual void SpecialKeyboardChange(unsigned char key, int x, int y);

private:
  int mRendererNum { 0 };

  Camera* mCamera { nullptr };
  MeshGroup<Batch>* mMeshGroup { nullptr };
  Texture2d* mTexture   { nullptr };
  Texture2d* mNormalMap { nullptr };
  MeshGroup<Batch>* mImportedMeshGroup { nullptr };

  Polygon* mPolygon;
  AxisMesh* mAxis;
  GridMesh* mGrid;
  TexturedSphere*  mDome;
  BoundingBoxMesh* mBoundingBox;
  WireframeSphere* mWireframeSphere;

  LightSource* mLightSource;

  DebugRenderer* mDebugRenderer;
  PhongRenderer* mPhongRenderer;
};