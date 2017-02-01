#include "my_model.h"

#include <gloo/transform.h>
#include <gloo/mouse_event.h>
#include <gloo/obj-parser.h>
#include <gloo/obj-mesh.h>

#include <cstdio>
#include <iostream>


GLfloat squareVertices[] = {-1.5f, 0.0f,  1.5f,
                             1.5f, 0.0f,  1.5f,
                            -1.5f, 0.0f, -1.5f,
                             1.5f, 0.0f, -1.5f};

GLfloat squareColors[] = {1.0f, 0.0f, 0.0f, 
                          0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 1.0f,
                          0.4f, 0.4f, 0.4f};

GLfloat squareUV[] = { 0.0f, 0.0f,
                       1.0f, 0.0f,
                       0.0f, 1.0f,
                       1.0f, 1.0f };

GLfloat squareTangents[] = { -1.0f, 0.0f, 0.0f,
                             -1.0f, 0.0f, 0.0f,
                             1.0f, 0.0f, 0.0f,
                             1.0f, 0.0f, 0.0f };

GLfloat squareNormals[] = {0.0f, 1.0f, 0.0f, 
                           0.0f, 1.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           0.0f, 1.0f, 0.0f};

GLuint indices[] = {0, 2, 1, 3};
GLuint program; 

MyModel::MyModel()
{

}

MyModel::~MyModel()
{
  delete mCamera;
  delete mMeshGroup;

  delete mDebugRenderer;
  delete mPhongRenderer;
  delete mImportedMeshGroup;

  delete mAxis;
  delete mGrid;
  delete mPolygon;
  delete mBoundingBox;
  delete mWireframeSphere;
  delete mTexture;
  delete mDome;
  delete mNormalMap;

  delete mLightSource;
}

bool MyModel::Init()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  mDebugRenderer = new DebugRenderer();
  mPhongRenderer = new PhongRenderer("../../shaders/phong/vertex_shader.glsl",
                                     "../../shaders/phong/fragment_shader.glsl");

  if (!mDebugRenderer->Load())
  {
    std::cout << "Couldn't initialize 'MyModel::DebugRenderer*' ..." << std::endl;
    delete mDebugRenderer;
    return false;
  }

  if (!mPhongRenderer->Load())
  {
    std::cout << "Couldn't initialize 'MyModel::PhongRenderer*' ..." << std::endl;
    delete mPhongRenderer;
    return false;
  }

  mCamera = new Camera();
  mCamera->SetPosition(0, 0, 3.0f);

  GLint posAttribLoc  = mDebugRenderer->GetPositionAttribLoc();
  GLint colAttribLoc  = mDebugRenderer->GetColorAttribLoc();

  mAxis = new AxisMesh(posAttribLoc, colAttribLoc);
  mGrid = new GridMesh(posAttribLoc, colAttribLoc, 9, 9, 0.15f, {0.4f, 0.4f, 0.4f});
  mBoundingBox = new BoundingBoxMesh(posAttribLoc, colAttribLoc);
  mPolygon = new Polygon(posAttribLoc, colAttribLoc, 1.0f, 10);
  mWireframeSphere = new WireframeSphere(posAttribLoc, colAttribLoc, {0.0f, 1.0f, 0.0f}, 16);


  GLint posAttribLocPhong = mPhongRenderer->GetPositionAttribLoc();
  GLint normalAttribLocPhong =  mPhongRenderer->GetNormalAttribLoc();
  GLint textureAttribLocPhong = mPhongRenderer->GetTextureAttribLoc();
  GLint tangentAttribLocPhong = mPhongRenderer->GetTangentAttribLoc();

  mDome = new TexturedSphere(posAttribLocPhong, normalAttribLocPhong, textureAttribLocPhong, tangentAttribLocPhong,
                             {glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(0.07, 0.07, 0.07)});

  mMeshGroup = new MeshGroup<Batch>(4, 4);
  mMeshGroup->SetVertexAttribList({3, 3, 2, 3});
  mMeshGroup->AddRenderingPass({{posAttribLoc, true}, {colAttribLoc, true}, gloo::kNoAttrib, gloo::kNoAttrib});
  mMeshGroup->AddRenderingPass({{posAttribLocPhong, true}, 
                                {normalAttribLocPhong, true}, 
                                {textureAttribLocPhong, true},
                                {tangentAttribLocPhong, true}});
  mMeshGroup->AddRenderingPass({{posAttribLocPhong, true}, 
                                {normalAttribLocPhong, true}, 
                                {textureAttribLocPhong, true},
                                {tangentAttribLocPhong, false}});
  mMeshGroup->Load({squareVertices, squareNormals, squareUV, squareTangents}, nullptr);

  // Load textures.
  mTexture = new Texture2d();
  mTexture->Load("textures/sky2.jpg");

  mNormalMap = new Texture2d();
  mNormalMap->Load("textures/154_norm.jpg");

  mPhongRenderer->SetTextureUnit("color_map",  0);
  mPhongRenderer->SetTextureUnit("normal_map", 1);
  
  mPhongRenderer->EnableLighting();


  gloo::ObjParser parser;
  gloo::ObjMesh objMesh;
  if (parser.LoadObj("objs/B-747.obj", objMesh, true))
  {
    std::cout << "Obj successfully loaded. " << std::endl;
    objMesh.TessellateQuads();
  }

  mImportedMeshGroup = objMesh.ExportToMeshGroup(0); 
  if (mImportedMeshGroup)
  { 
    auto vertexAttribList = mImportedMeshGroup->GetVertexAttribList();

    if (vertexAttribList.size() == 3)
    {
      mImportedMeshGroup->AddRenderingPass({{posAttribLocPhong, true}, 
                                            {normalAttribLocPhong, true}, 
                                            {textureAttribLocPhong, true}});  
    }
    else if (vertexAttribList.size() == 2)
    {
      if (vertexAttribList[1] == 2)
      {
        mImportedMeshGroup->AddRenderingPass({{posAttribLocPhong, true},  
                                             {textureAttribLocPhong, true}});  
      }
      else
      {
        mImportedMeshGroup->AddRenderingPass({{posAttribLocPhong, true},  
                                              {normalAttribLocPhong, true}});  
      }
    }  
  }
  else
  {
    std::cout << "Couldn't initialize 'MyModel::mImportedMeshGroup*' ..." << std::endl;
    return false;
  }

  return true;
}

void MyModel::Idle()
{
  glutPostRedisplay();
}

void MyModel::Display()
{
  static float blah_angle = 0.0;
  blah_angle += 0.01;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mCamera->SetOnRendering();
  mPhongRenderer->Bind();
  mPhongRenderer->SetCamera(mCamera);

  gloo::Transform M;
  M.LoadIdentity();

  mPhongRenderer->SetNumLightSources(3);
  mPhongRenderer->EnableLightSource(0);
  mPhongRenderer->EnableLightSource(1);
  mPhongRenderer->EnableLightSource(2);

  LightSource lightSource1 = { glm::vec3(3*std::cos(blah_angle), 3.0f, 3*std::sin(blah_angle)),  // Pos.
                               glm::vec3(0,   -1,  0),     // Dir.
                               glm::vec3(0.7, 0.7, 0.7),     // Ld.
                               glm::vec3(0.2, 0.2, 0.2),   // Ls.
                               2.0f};  // Alpha.
  LightSource lightSource2 = { glm::vec3(3*std::cos(blah_angle + 3.1415*0.66), 3.0f, 3*std::sin(blah_angle + 3.1415*0.66)),  // Pos.
                               glm::vec3(0,   -1,  0),    // Dir.
                               glm::vec3(0.7, 0.7, 0.7),    // Ld.
                               glm::vec3(0.2, 0.2, 0.2),  // Ls.
                              2.0f};  // Alpha.

  LightSource lightSource3 = { glm::vec3(3*std::cos(blah_angle + 3.1415*1.33), 3.0f, 3*std::sin(blah_angle + 3.1415*1.33)),  // Pos.
                               glm::vec3(0,   -1,  0),  // Dir.
                               glm::vec3(0.7, 0.7, 0.7),  // Ld.
                               glm::vec3(.5, .5, .5),  // Ls.
                               5.0f};  // Alpha.

  mPhongRenderer->SetLightSourceInCameraCoordinates(lightSource1, mCamera, 0);
  mPhongRenderer->SetLightSourceInCameraCoordinates(lightSource2, mCamera, 1);
  mPhongRenderer->SetLightSourceInCameraCoordinates(lightSource3, mCamera, 2);


  // Render rectangle with normal mapping.
  mPhongRenderer->SetMaterial({ glm::vec3(0, 0, 0), 
                                glm::vec3(.4, .4, .4),
                                glm::vec3(.05, .05, .05)});

  mTexture->Bind(GL_TEXTURE0);
  mNormalMap->Bind(GL_TEXTURE1);
  M.LoadIdentity();
  // M.Rotate(-0.79*cos(blah_angle), 1, 0, 1);
  // mPhongRenderer->Render(mMeshGroup, M, 1);
  
  // Render dome.
  mPhongRenderer->DisableLighting();
  M.LoadIdentity();
  M.Scale(100.0f, 100.0f, 100.0f);
  // M.Rotate(blah_angle/10.0f, 0, 1, 0);
  mPhongRenderer->SetMaterial(mDome->GetMaterial());
  mPhongRenderer->Render(mDome->GetMeshGroup(), M, 0);
  mPhongRenderer->EnableLighting();

  // Render imported obj.
  M.LoadIdentity();
  M.Scale(0.10f, 0.10f, 0.10f);
  mPhongRenderer->DisableColorMap();
  mPhongRenderer->SetMaterial({ glm::vec3(0, 0, 0), 
                                glm::vec3(.9, .9, .9),
                                glm::vec3(.1, .1, .1)});
  mPhongRenderer->Render(mImportedMeshGroup, M, 0);
  mPhongRenderer->EnableColorMap();


  if (mRendererNum == 1) 
  {
    M.LoadIdentity();
    M.Scale(3.0f, 3.0f, 3.0f);
    mDebugRenderer->Bind();
    mDebugRenderer->Render(mGrid->GetMeshGroup(), M, mCamera);
    mDebugRenderer->Render(mAxis->GetMeshGroup(), M, mCamera);
    // mDebugRenderer->Render(mWireframeSphere->GetMeshGroup(), M, mCamera);
  }

  glutSwapBuffers();
}

void MyModel::Reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  mCamera->SetOnReshape(0, 0, w, h);
}

void MyModel::ActiveMouseMotion(const MouseEvent & mouseEvent)
{
  switch (mouseEvent.mMouseState.mModifier)
  {
    case MouseState::kCTRL:

    break;

    case MouseState::kSHIFT:

    break;

    case MouseState::kALT:

    break;
  
    default:
      if (mouseEvent.mMouseState.mLftButton == MouseState::kDown)
      {
        // std::cout << "Left button dragged.\n";
        mCamera->Translate(0.0f, 0.0f, -mouseEvent.mMouseState.mVelY * 1e-2);
      }
      if (mouseEvent.mMouseState.mRgtButton == MouseState::kDown)
      {
        mCamera->Rotate(mouseEvent.mMouseState.mVelY/100.0f, 
                        mouseEvent.mMouseState.mVelX/100.0f,
                        0.0f);
        // std::cout << "Right button dragged.\n";
      }
      if (mouseEvent.mMouseState.mMidButton == MouseState::kDown)
      {
        // std::cout << "dy = " << mouseEvent.mMouseState.mVelY * 1e-2 << std::endl;
        mCamera->Translate(0.0f, 0.0f, -mouseEvent.mMouseState.mVelY * 1e-2);
        // std::cout << "Middle button dragged.\n";
      }
  }  // end switch.

}

void MyModel::PassiveMouseMotion(const MouseEvent & mouseEvent)
{
  // std::cout << "Coordinates: " << mouseState.mPosX << ", " << mouseState.mPosY << std::endl;
}

void MyModel::MouseButtonChange(const MouseEvent & mouseEvent)
{
  if (mouseEvent.mMouseState.mModifier == MouseState::kCTRL)
  {
    std::cout << "CTRL.\n";
  }
  if (mouseEvent.mMouseState.mModifier == MouseState::kSHIFT)
  {
    std::cout << "SHIFT.\n";
  }
  if (mouseEvent.mMouseState.mModifier == MouseState::kALT)
  {
    std::cout << "ALT.\n";
  }

  if (mouseEvent.mButton == MouseState::kLeft && mouseEvent.mButtonState == MouseState::kUp)
  {
    // std::cout << "Left button pressed.\n";
  }
  if (mouseEvent.mButton == MouseState::kRight && mouseEvent.mButtonState == MouseState::kUp)
  {
    // std::cout << "Right button pressed.\n";
  }
  if (mouseEvent.mButton == MouseState::kMiddle && mouseEvent.mButtonState == MouseState::kUp)
  {
    // std::cout << "Middle button pressed.\n";
  }
}

void MyModel::KeyboardChange(unsigned char key, int x, int y)
{
  static bool fullScreen = false;

  switch (key)
  {
    case 27: // ESC key
      exit(0);
    break;

    case 'u':
    case 'U':
      mRendererNum = (mRendererNum+1) % 2;
    break;

    case 'F':
    case 'f':
      if (fullScreen)
      {
        glutReshapeWindow(800, 600);
        glutPositionWindow(0, 0);
      }
      else
      {
        glutFullScreen();
      }
      fullScreen = !fullScreen;
    break;
  }
}

void MyModel::SpecialKeyboardChange(unsigned char key, int x, int y)
{

}