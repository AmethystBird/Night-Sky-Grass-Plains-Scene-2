#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/cube.h"
#include "plane.h"
#include "skybox.h"
#include "helper/objmesh.h"

using namespace std;

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;

    //Functions
    void compile();
    void SetMatrices();

    //Objects
    SkyBox skyBox;
    Plane plane;
    unique_ptr<ObjMesh> tree;
    unique_ptr<ObjMesh> rock;
    //Teapot teapot;
    //Cube cube;

    //Dynamic general attributes
    float timePrev;
    float angle;
    float rotationSpeed;

    //Dynamic fog attributes
    float fogIntensity = 16.f;
    float fogChangeSpeed = 1.f / 4.f;
    bool fogIncreasing = true;

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H