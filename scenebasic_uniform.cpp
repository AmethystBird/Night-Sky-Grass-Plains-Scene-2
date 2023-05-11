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
    glGenBuffers(1, &initialVelocity);
    glGenBuffers(1, &startTime);

    int size = nParticles * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initialVelocity);
    glBufferData(GL_ARRAY_BUFFER, size * 3, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

    glm::mat3 emitterBasis = ParticleUtils::makeArbitraryBasis(emitterDirection);
    vec3 v(0.0f);
    float velocity, theta, phi;
    vector<GLfloat> data(nParticles * 3);

    for (uint32_t i = 0; i < nParticles; i++)
    {
        theta = glm::mix(0.0f, glm::pi<float>() / 20.f, RandomFloat());
        phi = glm::mix(0.0f, glm::two_pi<float>(), RandomFloat());

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        velocity = glm::mix(1.25f, 1.5f, RandomFloat());
        v = glm::normalize(emitterBasis * v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.z;
    }

    glBindBuffer(GL_ARRAY_BUFFER, initialVelocity);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        data[i] = rate * i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initialVelocity);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

float SceneBasic_Uniform::RandomFloat()
{
    return randomiser.nextFloat();
}

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), timePrev(0.0f), rotationSpeed(glm::pi<float>() / 8.0f), plane(50.f, 50.f, 1, 1), skyBox(100.f) {
    //loading of models
    tree = ObjMesh::load("media/tree/source/JASMIM+MANGA.obj", true, false);
    rock = ObjMesh::load("media/rock/rock.obj", true, false);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);

    //Enabling of alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    BufferInitiation();

    progFire.use();

    prog.setUniform("particleTexture", 0);
    prog.setUniform("particleLifetime", particleLifetime);
    prog.setUniform("particleSize", 0.05f);
    prog.setUniform("gravity", vec3(0.0f, -0.2f, 0.0f));
    prog.setUniform("emitterPosition", emitterPosition);

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
        name << "lights[" << i << "].lightPosition";
        x = 2.0f * cosf((glm::two_pi<float>() / 3) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 3) * i);
        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z + 1.0f, 0.0f));
    }

    //Fragment lighting colours & intensity
    prog.setUniform("lights[0].lightAmbient", vec3(0.0f, 0.0f, 0.0f));
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
    prog.setUniform("material.shinyness", 45.0f); //180

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

    GLuint skyBoxTexture = Texture::loadHdrCubeMap("media/texture/cube/pisa-hdr/pisa");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

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
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();

        progFire.compileShader("shader/fire.vert");
        progFire.compileShader("shader/fire.frag");
        progFire.link();
        progFire.use();
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
    prog.setUniform("ProjectionMatrix", projection);
    prog.setUniform("ModelMatrix", model);

    //prog.setUniform("Fog.MinDistance", 1.f);
    //prog.setUniform("Fog.Color", vec3(0.5f, 0.5f, 0.5f));
}

//Called by scenerunner.h
void SceneBasic_Uniform::update(float t)
{
    float deltaTime = t - timePrev;

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
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    /*
    //Draw skybox
    prog.use();
    model = glm::mat4(1.f);

    SetMatrices(prog);

    //update light position; work on this to get light into position of torch flame
    //glm::vec4 lightPosition = glm::vec4(10.f * cos(angle), 10.f, 10.f * sin(angle), 1.f); //if camera not moving

    //Dynamic light position
    glm::vec4 lightPosition = glm::vec4(10.f * cos(angle * 2), 10.f, 10.f * sin(angle * 2), 1.f); //if camera moving
    prog.setUniform("lights[0].lightPosition", view * lightPosition);

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
    prog.setUniform("fog.MaxDistance", fogIntensity);

    //Rendering of objects
    skyBox.render();
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
    vec3 cameraPosition = vec3(7.f * cos(angle), 2.f, 7.f * sin(angle));
    view = glm::lookAt(cameraPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    SetMatrices(prog);
    */
    progFire.use();
    SetMatrices(progFire);
    prog.setUniform("time", timePrev);
    glBindVertexArray(particles);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}