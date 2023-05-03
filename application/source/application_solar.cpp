#include "application_solar.hpp"
#include "window_handler.hpp"
#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include <glbinding/gl/gl.h>
using namespace gl; // use gl definitions from glbinding 

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <array>
#include "SceneGraph.hpp"
#include "Node.hpp"
#include "PointLightNode.hpp"
#include "GeometryNode.hpp"
#include "Timer.hpp"
#include "CameraNode.hpp"
using glm::fvec3;
using glm::fmat4;
using glm::scale;
using glm::rotate;
using glm::translate;
using std::array;
using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
    :Application{resource_path}
    ,planet_object{}
    ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 20.0f})}
    ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
    , _timer{}
{
    initializeSceneGraph();
    initializeCamera(m_view_transform, m_view_projection);
    initializeGeometry();
    initializeShaderPrograms();
    SceneGraph::getInstance().printGraph(); // When all initialization are done, print SceneGraph to console
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

///////////////////////////// intialisation functions /////////////////////////
// Initialize scenegraph's hierarchy object
void ApplicationSolar::initializeSceneGraph() {
    auto distanceBetweenPlanetInX = 3.0f; // distance between each planet in X axis
    model planetModel = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

    // Initialize sceneGraph obj & Attach root node to it
    auto root = make_shared<Node>("Root");
    SceneGraph::getInstance().setRoot(root);

    // Add sun node as a child of root nood
    auto sun = make_shared<PointLightNode>("PointLight");
    auto sunGeo = make_shared<GeometryNode>("Sun Geometry", planetModel);
    root->addChild(sun);
    sun->addChild(sunGeo);
    sunGeo->setLocalTransform(scale(sunGeo->getLocalTransform(), { 3.0f, 3.0f, 3.0f })); // make sun bigger size

    // Add earth node and its moon under root nood
    auto earth = make_shared<Node>("Earth Holder");
    auto earthGeo = make_shared<GeometryNode>("Earth Geometry", planetModel);
    root->addChild(earth);
    earth->addChild(earthGeo);
    earthGeo->setLocalTransform(translate(earthGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set earth position
    auto moon = make_shared<Node>("Moon Holder");
    auto moonGeo = make_shared<GeometryNode>("Moon Geometry", planetModel);
    earth->addChild(moon);
    moon->addChild(moonGeo);
    moonGeo->setLocalTransform(scale(moonGeo->getLocalTransform(), { 0.5f,0.5f,0.5f })); // make moon smaller
    moonGeo->setLocalTransform(translate(moonGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set moon position

    // Add remaining 7 planets as children of root nood
    array<string, 7> planets = { "Mercury", "Venus", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune" };
    for (const auto& each : planets) {
        auto planet = make_shared<Node>(each + " Holder");
        auto planetGeo = make_shared<GeometryNode>(each + " Geometry", planetModel);
        root->addChild(planet);
        planet->addChild(planetGeo);

        // set position and gap between each planet
        distanceBetweenPlanetInX += 3.0f;
        planetGeo->setLocalTransform(translate(planetGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f }));
    }
}

// Setup camera node
void ApplicationSolar::initializeCamera(fmat4 camInitialTransform, fmat4 camInitialProjection) {
    auto camera = make_shared<CameraNode>("Camera");
    camera->setEnabled(true);
    camera->setLocalTransform(camInitialTransform);
    camera->setProjectionMatrix(camInitialProjection);
    
    SceneGraph::getInstance().setCamera(camera); // make camera node accessible through SceneGraph object
    SceneGraph::getInstance().getRoot()->addChild(camera); // add camera node to root node
}

// load models
void ApplicationSolar::initializeGeometry() {
    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

    // generate vertex array object
    glGenVertexArrays(1, &planet_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(planet_object.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &planet_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

    // generate generic buffer
    glGenBuffers(1, &planet_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

    // store type of primitive to draw
    planet_object.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object 
    planet_object.num_elements = GLsizei(planet_model.indices.size());
}

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
    // store shader program objects in container
    m_shaders.emplace("planet", shader_program{ {{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                             {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}} });
    // request uniform locations for shader program
    m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
    m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
    m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
    m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
}

void ApplicationSolar::uploadView() {
    // vertices are transformed in camera space, so camera transform must be inverted
    glm::fmat4 view_matrix = glm::inverse(SceneGraph::getInstance().getCamera()->getWorldTransform());
    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(SceneGraph::getInstance().getCamera()->getProjectionMatrix()));
}

// update uniform locations (triggered before render())
void ApplicationSolar::uploadUniforms() {
    // bind shader to which to upload unforms
    glUseProgram(m_shaders.at("planet").handle);
    // upload uniform values to new locations
    uploadView();
    uploadProjection();
}

///////////////////////////// intialisation functions /////////////////////////
void ApplicationSolar::render() const {
    // bind shader to upload uniforms
    glUseProgram(m_shaders.at("planet").handle);

    auto renderPlanet = [this](shared_ptr<Node> node) {
        // Render only GeometryNode
        auto geoNode = dynamic_pointer_cast<GeometryNode>(node);
        if (!geoNode) { return; }

        // Rotate GeometryNode's parent, because rightnow all parent node is in the same position as sun
        if (geoNode->getName() == "Moon Geometry") { // Except moon holder need to rotate around earth geometry !!
            auto parent = geoNode->getParent();
            auto earthGeo = parent->getParent()->getChild("Earth Geometry");
            parent->setLocalTransform(rotate(earthGeo->getLocalTransform(), float(glfwGetTime()) * 10, fvec3{ 0.0f, 1.0f, 0.0f }));
        }
        else if (geoNode->getName() != "Sun Geometry") {
            auto parent = geoNode->getParent();
            parent->setLocalTransform(rotate(parent->getLocalTransform(), static_cast<float>(_timer.getElapsedTime()) * 2, fvec3{ 0.0f, 1.0f, 0.0f }));
        }

        // Then the rotation of holder node will affect position of geometry node aswell
        auto modelTransform = geoNode->getWorldTransform();
        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelTransform));

        // extra matrix for normal transformation to keep them orthogonal to surface
        glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(SceneGraph::getInstance().getCamera()->getWorldTransform()) * modelTransform);
        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));

        // bind the VAO to draw
        glBindVertexArray(planet_object.vertex_AO);

        // draw bound vertex array using bound shader
        glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
    };

    // Traverse each geometry node from root node in SceneGraph
    auto root = SceneGraph::getInstance().getRoot();
    renderPlanet(root);
    root->traverse(renderPlanet);
}

///////////////////////////// callback functions for window events ////////////
// handle key input
void ApplicationSolar::keyCallback(int key, int action, int mods) {
    auto camera = SceneGraph::getInstance().getCamera();
    auto& cameraTransform = camera->getLocalTransform();
    
    if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        camera->setLocalTransform(translate(cameraTransform, fvec3{ 0.0f, 0.0f, -0.2f })); 
        uploadView();
    } else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        camera->setLocalTransform(translate(cameraTransform, fvec3{ 0.0f, 0.0f, 0.2f }));
        uploadView();
    } else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        camera->setLocalTransform(translate(cameraTransform, fvec3{ -0.2f, 0.0f, 0.0f })); // move camera position to left
        uploadView();
    } else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        camera->setLocalTransform(translate(cameraTransform, fvec3{ 0.2f, 0.0f, 0.0f })); // move camera position to right
        uploadView();
    }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  SceneGraph::getInstance().getCamera()->setProjectionMatrix(utils::calculate_projection_matrix(float(width) / float(height)));
  // upload new projection matrix
  uploadProjection();
}

///////////////////////////// exe entry point /////////////////////////////
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}