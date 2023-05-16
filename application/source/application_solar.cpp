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
#include <map>
#include <vector>
#include "SceneGraph.hpp"
#include "Node.hpp"
#include "PointLightNode.hpp"
#include "GeometryNode.hpp"
#include "Timer.hpp"
#include "CameraNode.hpp"
using glm::fvec3;
using glm::radians;
using glm::fmat4;
using glm::scale;
using glm::rotate;
using glm::translate;
using std::map;
using std::array;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
    :Application{resource_path}
    ,planet_object{}
    ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 20.0f})}
    ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
    ,_timer{}
    ,_shaderList{{"planetShader", "simple"}, {"starShader", "vao"}, {"orbitShader", "orbit"}}
{
    // Initialize order is matter
    initializeGeometry();
    initializeShaderPrograms();
    initializeSceneGraph();
    initializeCamera(m_view_transform, m_view_projection);
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
    // Initialize sceneGraph obj & Attach root node to it
    auto root = make_shared<Node>("Root");
    SceneGraph::getInstance().setRoot(root);
    auto distanceBetweenPlanetInX = 5.0f; // distance between each planet in X axis

    // Add sun node as a child of root node
    auto sun = make_shared<PointLightNode>("PointLight");
    auto sunGeo = make_shared<GeometryNode>("Sun Geometry", "planetShader", planet_object);
    root->addChild(sun);
    sun->addChild(sunGeo);
    sunGeo->setLocalTransform(scale(sunGeo->getLocalTransform(), { 3.0f, 3.0f, 3.0f })); // make sun bigger size

    // todo-moch: refactor earth initialization and moon initialization
    // Add earth node
    auto earth = make_shared<Node>("Earth Holder");
    auto earthGeo = make_shared<GeometryNode>("Earth Geometry", "planetShader", planet_object);
    auto earthOrbit = make_shared<GeometryNode>("Earth Orbit", "orbitShader", planet_object);
    root->addChild(earth);
    //root->addChild(earthOrbit); todo-moch: enable when shader is ready
    earth->addChild(earthGeo);
    earthGeo->setLocalTransform(translate(earthGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set earth geo's position
    earthOrbit->setLocalTransform(scale(earthOrbit->getLocalTransform(), { distanceBetweenPlanetInX, distanceBetweenPlanetInX, distanceBetweenPlanetInX })); // set earth orbit's size
    
    // Add moon as child of earth geometry
    auto moon = make_shared<Node>("Moon Holder");
    auto moonGeo = make_shared<GeometryNode>("Moon Geometry", "planetShader", planet_object);
    auto moonOrbit = make_shared<GeometryNode>("Moon Orbit", "orbitShader", planet_object);
    earthGeo->addChild(moon);
    //earthGeo->addChild(moonOrbit); todo-moch: enable when shader is ready
    moon->addChild(moonGeo);
    moonGeo->setLocalTransform(scale(moonGeo->getLocalTransform(), { 0.5f,0.5f,0.5f })); // make moon smaller
    moonGeo->setLocalTransform(translate(moonGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set moon position
    moonOrbit->setLocalTransform(scale(moonOrbit->getLocalTransform(), { distanceBetweenPlanetInX, distanceBetweenPlanetInX, distanceBetweenPlanetInX })); // set moon orbit size

    // Add remaining 7 planets as children of root node
    array<string, 7> planets = { "Mercury", "Venus", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune" };
    for (const auto& each : planets) {
        auto planet = make_shared<Node>(each + " Holder");
        auto planetGeo = make_shared<GeometryNode>(each + " Geometry", "planetShader", planet_object);
        auto planetOrbit = make_shared<GeometryNode>(each + " Orbit", "orbitShader", planet_object);
        root->addChild(planet);
        //root->addChild(planetOrbit); todo-moch: enable when shader is ready
        planet->addChild(planetGeo);

        // set position and gap between each planet
        distanceBetweenPlanetInX += 5.0f;
        planetGeo->setLocalTransform(translate(planetGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f }));

        // Orbit geo node will be at the same position as sun, and its scale is big as distance between sun and planet geometry
        planetOrbit->setLocalTransform(scale(planetOrbit->getLocalTransform(), { distanceBetweenPlanetInX, distanceBetweenPlanetInX, distanceBetweenPlanetInX }));
    }

    // Add star geometry node and scale its size as big as possible
    auto starGeo = make_shared<GeometryNode>("Star", "starShader", planet_object);
    starGeo->setLocalTransform(scale(starGeo->getLocalTransform(), { 10.0f, 10.0f, 10.0f }));
    //root->addChild(starGeo); todo-moch: enable when shader is ready
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
    // Generic method to setup data for rendering each geometry
    auto initGeometry = [this](model_object& geoObject,
                                const model* modelData,
                                vector<float>& vertexData,
                                GLenum drawMode,
                                GLint numAttribute,
                                GLint* attributeSizes,
                                GLenum* attributeTypes,
                                GLsizei* attributeStrides,
                                void** attributeOffsets,
                                GLsizei numElement,
                                bool useIndices = false) {
            // Generate and bind vertex array object
            glGenVertexArrays(1, &geoObject.vertex_AO);
            glBindVertexArray(geoObject.vertex_AO);

            // Generate and bind vertex buffer object
            glGenBuffers(1, &geoObject.vertex_BO);
            glBindBuffer(GL_ARRAY_BUFFER, geoObject.vertex_BO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

            // Set up vertex attributes on gpu
            for (GLint i = 0; i < numAttribute; ++i) {
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, attributeSizes[i], attributeTypes[i], GL_FALSE, attributeStrides[i], attributeOffsets[i]);
            }

            // If indices are used, set up IBO aswell
            if (useIndices) {
                glGenBuffers(1, &geoObject.element_BO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geoObject.element_BO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * modelData->indices.size(), modelData->indices.data(), GL_STATIC_DRAW);
            }

            // Store draw mode and number of elements in geometry object
            geoObject.draw_mode = drawMode;
            geoObject.num_elements = numElement;
    };

    // 1. Initialize planet geometry from loaded model
    model planetModel = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
    GLint attributeSizes[] = { model::POSITION.components, model::NORMAL.components };
    GLenum attributeTypes[] = { model::POSITION.type, model::NORMAL.type };
    GLsizei attributeStrides[] = { planetModel.vertex_bytes, planetModel.vertex_bytes };
    void* attributeOffsets[] = { planetModel.offsets[model::POSITION], planetModel.offsets[model::NORMAL] };
    GLsizei numElement = GLsizei(planetModel.data.size());
    initGeometry(planet_object, &planetModel, planetModel.data, GL_TRIANGLES, 2, attributeSizes, attributeTypes, attributeStrides, attributeOffsets, numElement, true);

    // 2. init star model and set to node object


    // 3. init orbit model and set to node object 
}

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
    // store shader program objects in container
    for (const auto& each : _shaderList) {
        auto& filePath = m_resource_path + "shaders/" + each.second;

        m_shaders.emplace(each.first, shader_program{ {{GL_VERTEX_SHADER,filePath+".vert"}, {GL_FRAGMENT_SHADER,filePath+".frag"}} });
        m_shaders.at(each.first).u_locs["NormalMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ModelMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ViewMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ProjectionMatrix"] = -1;
    }
}

void ApplicationSolar::uploadView() {
    // vertices are transformed in camera space, so camera transform must be inverted
    fmat4 viewMatrix = glm::inverse(SceneGraph::getInstance().getCamera()->getWorldTransform());
    // upload matrix to gpu
    for (auto& const each : _shaderList) {
        glUseProgram(m_shaders.at(each.first).handle);
        glUniformMatrix4fv(m_shaders.at(each.first).u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    }
}

void ApplicationSolar::uploadProjection() {
    fmat4 projectionMatrix = SceneGraph::getInstance().getCamera()->getProjectionMatrix();
    for (auto& const each : _shaderList) {
        glUseProgram(m_shaders.at(each.first).handle);
        glUniformMatrix4fv(m_shaders.at(each.first).u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    }
}

// update uniform locations (triggered before render())
void ApplicationSolar::uploadUniforms() {
    // bind shader to which to upload unforms + upload uniform values to new locations
    uploadView();
    uploadProjection();
}

///////////////////////////// intialisation functions /////////////////////////
void ApplicationSolar::render() const {
    auto transformAndDrawGeometry = [this](shared_ptr<Node> node) {
        auto geoNode = dynamic_pointer_cast<GeometryNode>(node);
        if (!geoNode) { return; } // Render only GeometryNode

        // ------------------- Transformation section ----------------------
        if (geoNode->getShader() == "planetShader" && geoNode->getName() != "Sun Geometry") {
            // Rotate GeometryNode's parent, because rightnow all holder node is in the same position as sun
            // Then the rotation of holder will affect position of childe geometry node aswell
            auto parent = geoNode->getParent();
            parent->setLocalTransform(rotate(parent->getLocalTransform(), static_cast<float>(_timer.getElapsedTime() * 10.0f), fvec3{ 0.0f, 1.0f, 0.0f }));
        }
        // ------------------- End transformation section -------------------
        
        // ------------------- Drawing section -------------------------------
        auto shaderToUse = geoNode->getShader();
        auto geometryObject = geoNode->getGeometry();
        auto worldTransform = geoNode->getWorldTransform();

        // Bind shader to use
        glUseProgram(m_shaders.at(shaderToUse).handle);

        // Upload ModelMatrix & NormalMatrix
        glUniformMatrix4fv(m_shaders.at(shaderToUse).u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(worldTransform));
        glm::fmat4 normalMatrix = glm::inverseTranspose(glm::inverse(SceneGraph::getInstance().getCamera()->getWorldTransform()) * worldTransform);
        glUniformMatrix4fv(m_shaders.at(shaderToUse).u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix)); // extra matrix for normal transformation to keep them orthogonal to surface

        // Bind VAO and draw VBO
        glBindVertexArray(geometryObject.vertex_AO);
        if (geoNode->getShader() == "planetShader") {
            glDrawElements(geometryObject.draw_mode, geometryObject.num_elements, model::INDEX.type, NULL);
        }
        else {
            // todo-moch
        }
        // ------------------- End drawing section --------------------------
    };

    // Traverse scenegraph to render Geometry node
    auto root = SceneGraph::getInstance().getRoot();
    root->traverse(transformAndDrawGeometry);
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
    auto camera = SceneGraph::getInstance().getCamera();
    float mouseSensitivity = 0.1f;

    // Rotate camera in Y axis regarding x coordinate of mouse cursor
    camera->setLocalTransform(rotate(camera->getLocalTransform(), radians(float(pos_x * mouseSensitivity)), fvec3{ 0.0f, -1.0f, 0.0f }));
    // Rotate camera in X axis regarding y coordinate of mouse cursor
    camera->setLocalTransform(rotate(camera->getLocalTransform(), radians(float(pos_y* mouseSensitivity)), fvec3{ -1.0f, 0.0f, 0.0f }));

    uploadView();
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