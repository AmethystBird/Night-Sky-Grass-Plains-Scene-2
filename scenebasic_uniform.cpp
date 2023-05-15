#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/glslprogram.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using glm::vec3;

//lab 3
#include <sstream>
#include <GLFW/glfw3.h>

//lab 4
#include "helper/texture.h"

//fire
#include "helper/particleutils.h"

void SceneBasic_Uniform::BufferInitiation()
{
    glGenBuffers(2, positionBuffer);
    glGenBuffers(2, velocityBuffer);
    glGenBuffers(2, age);

    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);

    vector<GLfloat> tempData(nParticles);
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        tempData[i] = rate * (i - nParticles);
    }
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(2, particleArray);

    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    glGenTransformFeedbacks(2, feedback);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, positionBuffer[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velocityBuffer[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, positionBuffer[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velocityBuffer[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}

float SceneBasic_Uniform::RandomFloat()
{
    return randomiser.nextFloat();
}

//nParticles OG: 4,000
SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), timePrev(0.0f), rotationSpeed(glm::pi<float>() / 8.0f), plane(50.f, 50.f, 1, 1), skyBox(100.f), particleLifetime(6.0f), nParticles(100), emitterPosition(0.5, 0, 0), emitterDirection(-1, 2, 0), drawBuffer(1), deltaTime(0.f) {
    //loading of models
    tree = ObjMesh::load("media/tree/source/JASMIM+MANGA.obj", true, false);
    rock = ObjMesh::load("media/rock/rock.obj", true, false);
    //rock = ObjMesh::load("media/stylized__rock/obj/rock.obj", true, false);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //glEnable(GL_DEPTH_TEST); //maybe put this back later

    //Enabling of alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE4);
    Texture::loadTexture("media/fire.png");

    glActiveTexture(GL_TEXTURE5);
    ParticleUtils::createRandomTex1D(nParticles * 3);

    BufferInitiation();

    progFire.use();

    progFire.setUniform("particleTexture", 4);
    progFire.setUniform("particleLifetime", particleLifetime);
    progFire.setUniform("particleSize", 2.0f); //OG 0.05f, 0.38f
    progFire.setUniform("randomTexture", 5);
    progFire.setUniform("acceleration", vec3(0.f, 0.5f, 0.f));
    progFire.setUniform("emitter", emitterPosition);
    progFire.setUniform("emitterBasis", ParticleUtils::makeArbitraryBasis(emitterDirection));

    progSkyBox.use();
    progSkyBox.setUniform("skyBoxLightAmbient", vec3(0.03125f, 0.f, 0.25f));
    progSkyBox.setUniform("skyBoxMaterialAmbient", vec3(0.0625f, 0.f, 0.25f));

    progSkyBox.setUniform("fog.MaxDistance", 16.f);
    progSkyBox.setUniform("fog.MinDistance", 0.f);
    progSkyBox.setUniform("fog.Colour", 0.0f, 0.25f, 0.5f);

    GLuint skyBoxTexture = Texture::loadHdrCubeMap("media/texture/cube/pisa-hdr/pisa");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

    prog.use();

    /*--------------------------------
    Setting of MVP
    --------------------------------*/
    model = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);

    //projector matrix
    vec3 projPos = vec3(2.0f, 5.0f, 5.0f);
    vec3 projAt = vec3(-2.0f, -4.0f, 0.0f);
    vec3 projUp = vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 projView = glm::lookAt(projPos, projAt, projUp);
    glm::mat4 projProj = glm::perspective(glm::radians(30.0f), 1.0f, 0.2f, 1000.0f);
    glm::mat4 bias = glm::translate(glm::mat4(1.0f), vec3(0.5f));
    bias = glm::scale(bias, vec3(0.5f));
    prog.setUniform("ProjectorMatrix", bias * projProj * projView);

    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

    timePrev = 0.f;
    angle = 0.f;

    /*--------------------------------
    Setting of uniforms
    --------------------------------*/
    //Vertex lighting positions
    prog.setUniform("VertexLightPosition", view * glm::vec4(5.0f, 5.0f, 2.0f, 1.0f));
    prog.setUniform("Kd", 0.25f, 0.0f, 0.0f);
    prog.setUniform("Ld", 0.25f, 0.f, 0.f);

    //Fragment lighting positions
    float x, z;
    for (int i = 0; i < 3; i++)
    {
        std::stringstream name;
        name << "lights[" << i << "].position";
        x = 2.0f * cosf((glm::two_pi<float>() / 3) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 3) * i);
        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z + 1.0f, 0.0f));
    }

    /*prog.setUniform("lights[0].intensity", vec3(0.25f, 0.25f, 0.25f));
    prog.setUniform("lights[1].intensity", vec3(0.125f, 0.0f, 1.0f));
    prog.setUniform("lights[2].intensity", vec3(0.125f, 0.0f, 1.0f));*/

    prog.setUniform("lights[0].intensity", vec3(100.0f, 100.0f, 100.0f));
    prog.setUniform("lights[1].intensity", vec3(0.25f, 0.0f, 1.0f));
    prog.setUniform("lights[2].intensity", vec3(0.0f, 0.25f, 1.0f));

    prog.setUniform("material.roughness", 0.5f);
    prog.setUniform("material.metalicness", true);
    prog.setUniform("material.colour", 1.0f, 1.0f, 1.0f);

    //Fragment lighting colours & intensity
    /*prog.setUniform("lights[0].lightAmbient", vec3(0.0f, 0.0f, 0.0f));
    prog.setUniform("lights[0].lightDiffuse", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("lights[0].lightSpecular", vec3(0.5f, 0.5f, 0.5f));

    prog.setUniform("lights[1].lightAmbient", vec3(0.25f, 0.0625f, 0.0625f));
    prog.setUniform("lights[1].lightDiffuse", vec3(0.0f, 0.0f, 0.0f));
    prog.setUniform("lights[1].lightSpecular", vec3(0.0f, 0.0f, 0.0f));

    prog.setUniform("lights[2].lightAmbient", vec3(0.25f, 0.25f, 2.0f));
    prog.setUniform("lights[2].lightDiffuse", vec3(0.0f, 0.0f, 0.f));
    prog.setUniform("lights[2].lightSpecular", vec3(0.0f, 0.0f, 0.0f));

    prog.setUniform("material.materialAmbient", 0.1f, 0.1f, 0.1f);
    prog.setUniform("material.materialDiffuse", 0.3f, 0.3f, 0.3f); //0.9
    prog.setUniform("material.materialSpecular", 0.1f, 0.1f, 0.1f);
    prog.setUniform("material.shinyness", 45.0f); //180*/

    //Fog colours, intensity & depth
    prog.setUniform("fog.MaxDistance", 16.f);
    prog.setUniform("fog.MinDistance", 0.f);
    prog.setUniform("fog.Colour", 0.0f, 0.25f, 0.5f);

    /*--------------------------------
    Texture loading & binding
    --------------------------------*/
    GLuint baseTexture = Texture::loadTexture("media/tree/textures/_Vegetation_Bark_Maple_1.jpeg");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseTexture);

    GLuint overlayTexture = Texture::loadTexture("media/tree/textures/_40.png");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, overlayTexture);

    /*GLuint skyBoxTexture = Texture::loadHdrCubeMap("media/texture/cube/pisa-hdr/pisa");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);*/

    GLuint mapleLeaves = Texture::loadTexture("media/mapleLeaves/mapleLeaves.png");
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mapleLeaves);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

//Compilation of shaders
void SceneBasic_Uniform::compile()
{
    try {
        progFire.compileShader("shader/fire.vert");
        progFire.compileShader("shader/fire.frag");

        GLuint programHandle = progFire.getHandle();
        const char* outputNames[] = { "position", "velocity", "age" };
        glTransformFeedbackVaryings(programHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);

        progFire.link();
        progFire.use();

        progSkyBox.compileShader("shader/skyBox.vert");
        progSkyBox.compileShader("shader/skyBox.frag");

        progSkyBox.link();
        progSkyBox.use();

        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::SetMatrices(GLSLProgram& prog)
{
    //Setting of matrices & related fog information
    glm::mat4 mv = view * model;

    prog.setUniform("modelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("projection", projection);
    prog.setUniform("ModelMatrix", model);

    //prog.setUniform("Fog.MinDistance", 1.f);
    //prog.setUniform("Fog.Color", vec3(0.5f, 0.5f, 0.5f));
}

float SceneBasic_Uniform::DynamicFog()
{
    //Dynamic fog levels
    if (fogIncreasing == true)
    {
        if (fogIntensity < 192.f)
        {
            fogIntensity = fogIntensity + fogChangeSpeed;
        }
        else
        {
            fogIncreasing = false;
            fogIntensity = fogIntensity - fogChangeSpeed;
        }
    }
    else if (fogIncreasing == false)
    {
        if (fogIntensity > 16.f)
        {
            fogIntensity = fogIntensity - fogChangeSpeed;
        }
        else
        {
            fogIncreasing = true;
            fogIntensity = fogIntensity + fogChangeSpeed;
        }
    }
    return fogIntensity;
}

//Called by scenerunner.h
void SceneBasic_Uniform::update(float t)
{
    deltaTime = t - timePrev;

    if (timePrev == 0.f)
    {
        deltaTime = 0.f;
    }

    timePrev = t;
    angle += rotationSpeed * deltaTime;

    if (angle > glm::two_pi<float>())
    {
        angle -= glm::two_pi<float>();
    }
}

//Called by scenerunner.h
void SceneBasic_Uniform::render()
{
    /*glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);*/

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();
    model = glm::mat4(1.f);

    SetMatrices(prog);

    //update light position; work on this to get light into position of torch flame
    //glm::vec4 lightPosition = glm::vec4(10.f * cos(angle), 10.f, 10.f * sin(angle), 1.f); //if camera not moving

    //Dynamic light position
    glm::vec4 lightPosition = glm::vec4(10.f * cos(angle * 2), 10.f, 10.f * sin(angle * 2), 1.f); //if camera moving
    prog.setUniform("lights[0].position", view * lightPosition);

    DynamicFog();

    prog.setUniform("fog.MaxDistance", fogIntensity);

    //Rendering of objects
    //skyBox.render();
    plane.render();

    //Scaling & repositioning for tree object render
    glm::vec3 scale = glm::vec3(0.025f, 0.025f, 0.025f);
    model = glm::scale(model, scale);

    view = glm::translate(view, glm::vec3(0.f, 3.f, -2.f));

    SetMatrices(prog);

    tree->render();

    //Scaling & repositioning for rock object render
    scale = glm::vec3(72.f, 72.f, 72.f);
    model = glm::scale(model, scale);

    view = glm::translate(view, glm::vec3(3.f, -2.7f, 2.f));

    SetMatrices(prog);
    rock->render();

    //Rotating camera position
    //vec3 cameraPosition = vec3(7.f * cos(angle), 2.f, 7.f * sin(angle));
    //view = glm::lookAt(cameraPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    SetMatrices(prog);

    //Skybox
    progSkyBox.use();
    SetMatrices(progSkyBox);
    skyBox.render();
    progSkyBox.setUniform("fog.MaxDistance", fogIntensity);

    progFire.use();

    //progFire.setUniform("time", timePrev);
    progFire.setUniform("deltaTime", deltaTime);

    progFire.setUniform("pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuffer]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(particleArray[1 - drawBuffer]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    progFire.setUniform("pass", 2);

    //SetMatrices(progFire);
    //view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.f, 1.5f, 0.f), vec3(0.f, 1.f, 0.f));
    //model = glm::mat4(1.f);
    view = glm::translate(view, glm::vec3(-2.5f, 0.f, -2.5f));
    SetMatrices(progFire);
    glDepthMask(GL_FALSE);

    //SetMatrices(progFire);

    glBindVertexArray(particleArray[drawBuffer]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles); //added *6
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    drawBuffer = 1 - drawBuffer;

    //Rotating camera position
    vec3 cameraPosition = vec3(7.f * cos(angle), 2.f, 7.f * sin(angle));
    view = glm::lookAt(cameraPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    prog.use();
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}