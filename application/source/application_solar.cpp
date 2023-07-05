#include "application_solar.hpp"
#include "window_handler.hpp"
#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
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

auto const COLOR_COMPONENTS = 3;
auto const POSITION_COMPONENTS = 3;
auto const STAR_POSITION_RANGE = 2.0f;
auto const COLOR_MAX_VALUE = 255;
auto const TWO_PI = 2.0f * 3.14159265358979323846f;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
    : Application{resource_path}
    , _planetObject{}
    , _starObject{}
    , _orbitObject{}
    , _skyboxObject{}
    , _screenQuadObject{}
    , m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 20.0f})}
    , m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
    , _timer{}
    , _shaderList{ {"planetShader", "simple"}, {"starShader", "vao"}, {"orbitShader", "orbit"}, {"skyboxShader", "skybox"}, {"quadShader", "quad"} }
    , _isRotating{true}
    , _enableToonShading{false}
    , _enableHorizontalMirror{ false }
    , _enableVericallMirror{ false }
    , _enableBlur{ false }
    , _enableGrayscale{ false }
{
    // Initialization order is matter
    initializeGeometry();
    initializeShaderPrograms();
    initializeSceneGraph();
    initializeCamera(m_view_transform, m_view_projection);
    initializeFrameBuffer(initial_resolution.x, initial_resolution.y);
    SceneGraph::getInstance().printGraph(); // When all initialization are done, print SceneGraph to console
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &_planetObject.vertex_BO);
  glDeleteBuffers(1, &_planetObject.element_BO);
  glDeleteVertexArrays(1, &_planetObject.vertex_AO);
  glDeleteFramebuffers(1, &_fbo);
  glDeleteRenderbuffers(1, &_rbo);
  glDeleteTextures(1, &_screenTexture);
}

///////////////////////////// intialisation functions /////////////////////////
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
    GLint attributeSizes[] = { model::POSITION.components, model::NORMAL.components, model::TEXCOORD.components };
    GLenum attributeTypes[] = { model::POSITION.type, model::NORMAL.type, model::TEXCOORD.type };
    GLsizei attributeStrides[] = { planetModel.vertex_bytes, planetModel.vertex_bytes, planetModel.vertex_bytes };
    void* attributeOffsets[] = { planetModel.offsets[model::POSITION], planetModel.offsets[model::NORMAL], planetModel.offsets[model::TEXCOORD] };
    GLsizei numElement = GLsizei(planetModel.data.size());
    initGeometry(_planetObject, &planetModel, planetModel.data, GL_TRIANGLES, 3, attributeSizes, attributeTypes, attributeStrides, attributeOffsets, numElement, true);

    // 2. Initialize star primitive
    auto numberOfStars = 3000;
    vector<float> starData;
    for (int i = 0; i < numberOfStars; ++i) { // Each Star data layout consists of 3 number for xyz coordinate and 3 number for rgb color
        for (int p = 0; p < POSITION_COMPONENTS; ++p) { // each model space's coordinate will be a random float between - 1 and 1
            starData.emplace_back(STAR_POSITION_RANGE * (utils::random_float() - 0.5f));
        }
        for (int c = 0; c < COLOR_COMPONENTS; ++c) { // each color component will be random 0-254(int) and normalized to 0-1(float)
            starData.emplace_back(float(std::rand() % COLOR_MAX_VALUE) / COLOR_MAX_VALUE);
        }
    }
    GLint starAttributeSizes[] = { POSITION_COMPONENTS, COLOR_COMPONENTS };
    GLenum starAttributeTypes[] = { GL_FLOAT, GL_FLOAT };
    GLsizei starAttributeStrides[] = { sizeof(float) * (POSITION_COMPONENTS + COLOR_COMPONENTS), sizeof(float) * (POSITION_COMPONENTS + COLOR_COMPONENTS) };
    void* starAttributeOffsets[] = { 0, (void*)(sizeof(float) * POSITION_COMPONENTS) };
    GLsizei starNumElement = GLsizei(starData.size());
    initGeometry(_starObject, nullptr, starData, GL_POINTS, 2, starAttributeSizes, starAttributeTypes, starAttributeStrides, starAttributeOffsets, starNumElement);

    // 3. Initialize orbit primitive
    auto numberOfPointInTheLine = 128;
    vector<float> orbitData;
    for (int i = 0; i < numberOfPointInTheLine; ++i) { // Calculate coodinate around circle
        float theta = TWO_PI * static_cast<float>(i) / numberOfPointInTheLine; // Calculates an angle theta, which is evenly spaced around a full circle 2 * PI * radians
        orbitData.emplace_back(sin(theta)); // x coordinate = horizontal position of the point on the circle
        orbitData.emplace_back(0); // y-coordinate = orbits lie in the xz-plane
        orbitData.emplace_back(cos(theta)); //  z-coordinate depth position of the point on the circle
    }
    GLint orbitAttributeSizes[] = { POSITION_COMPONENTS };
    GLenum orbitAttributeTypes[] = { GL_FLOAT };
    GLsizei orbitAttributeStrides[] = { sizeof(float) * POSITION_COMPONENTS };
    void* orbitAttributeOffsets[] = { 0 };
    GLsizei orbitNumElement = GLsizei(orbitData.size() / POSITION_COMPONENTS);
    initGeometry(_orbitObject, nullptr, orbitData, GL_LINE_LOOP, 1, orbitAttributeSizes, orbitAttributeTypes, orbitAttributeStrides, orbitAttributeOffsets, orbitNumElement);

    // 4. Initialize skybox primitive
    vector<float> cubeData = {
        // Define 2 triangles for each cube face
        // back      
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        // left
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // right
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         // front
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // top
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        // bottom
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    GLint cubeAttributeSizes[] = { POSITION_COMPONENTS };
    GLenum cubeAttributeTypes[] = { GL_FLOAT };
    GLsizei cubeAttributeStrides[] = { sizeof(float) * POSITION_COMPONENTS };
    void* cubeAttributeOffsets[] = { 0 };
    GLsizei cubeNumElement = GLsizei(cubeData.size());
    initGeometry(_skyboxObject, nullptr, cubeData, GL_TRIANGLES, 1, cubeAttributeSizes, cubeAttributeTypes, cubeAttributeStrides, cubeAttributeOffsets, cubeNumElement);

    // 5. Initialize quad for offscreen rendering
    vector<float> screenQuadData = {
        // positions   // texCoords
        -1.0f, 1.0f,   0.0f, 1.0f, // v4
        -1.0f, -1.0f,  0.0f, 0.0f, // v1
        1.0f, -1.0f,   1.0f, 0.0f, // v2
        -1.0f,  1.0f,  0.0f, 1.0f, // v4
        1.0f, -1.0f,   1.0f, 0.0f, // v2
        1.0f,  1.0f,   1.0f, 1.0f  // v3
    };

    GLint screenQuadAttributeSizes[] = { 2, 2 }; // position components, texture components
    GLenum screenQuadAttributeTypes[] = { GL_FLOAT, GL_FLOAT };
    GLsizei screenQuadAttributeStrides[] = { sizeof(float) * 4, sizeof(float) * 4 }; // total stride for a full set of attributes (position + texture coordinates)
    void* screenQuadAttributeOffsets[] = { 0, (void*)(sizeof(float) * 2) }; // texture coordinates follow position, hence offset = sizeof(float) * 2
    GLsizei screenQuadNumElement = GLsizei(screenQuadData.size() / 4); // divide by the number of components (position + texture coordinates)
    initGeometry(_screenQuadObject, nullptr, screenQuadData, GL_TRIANGLE_STRIP, 2, screenQuadAttributeSizes, screenQuadAttributeTypes, screenQuadAttributeStrides, screenQuadAttributeOffsets, screenQuadNumElement);
}

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
    // store shader program objects in container
    for (const auto& each : _shaderList) {
        auto& filePath = m_resource_path + "shaders/" + each.second;
        
        m_shaders.emplace(each.first, shader_program{ {{GL_VERTEX_SHADER,filePath + ".vert"}, {GL_FRAGMENT_SHADER,filePath + ".frag"}} });
        
        m_shaders.at(each.first).u_locs["NormalMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ModelMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ViewMatrix"] = -1;
        m_shaders.at(each.first).u_locs["ProjectionMatrix"] = -1;
        m_shaders.at(each.first).u_locs["GeometryColor"] = -1;
        m_shaders.at(each.first).u_locs["AmbientColor"] = -1;
        m_shaders.at(each.first).u_locs["AmbientStrength"] = -1;
        m_shaders.at(each.first).u_locs["LightPosition"] = -1;
        m_shaders.at(each.first).u_locs["LightColor"] = -1;
        m_shaders.at(each.first).u_locs["CameraPosition"] = -1;
        m_shaders.at(each.first).u_locs["EnableToonShading"] = -1;
        m_shaders.at(each.first).u_locs["Texture"] = -1;
        m_shaders.at(each.first).u_locs["ScreenTexture"] = -1;
        m_shaders.at(each.first).u_locs["EnableHorizontalMirror"] = -1;
        m_shaders.at(each.first).u_locs["EnableVerticalMirror"] = -1;
        m_shaders.at(each.first).u_locs["EnableBlur"] = -1;
        m_shaders.at(each.first).u_locs["EnableGrayscale"] = -1;
    }
}

texture_object ApplicationSolar::initializeTexture(const string& textureFile) {
    // Initialize 2D texture
    texture_object textureObject;
    textureObject.target = GL_TEXTURE_2D;
    glGenTextures(1, &(textureObject.handle));
    glBindTexture(GL_TEXTURE_2D, textureObject.handle);
    
    // Configure wrapping mode and texture filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // specify wrapping mode for x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // specify wrapping mode for y-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // specify filtering method for texture magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // specify filtering method for texture minification
    
    // Uploads the pixel data to the GPU
    pixel_data pixelData = texture_loader::file(m_resource_path + "textures/" + textureFile);
    glTexImage2D(GL_TEXTURE_2D, 0, pixelData.channels, pixelData.width, pixelData.height, 0, pixelData.channels, pixelData.channel_type, pixelData.ptr());

    // Generates a set of smaller textures from the current texture, used in texture filtering
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return textureObject;
}

texture_object ApplicationSolar::initializeCubemapTexture() {
    // Initialize cubemap texture
    texture_object textureObject;
    textureObject.target = GL_TEXTURE_CUBE_MAP;
    glGenTextures(1, &(textureObject.handle));
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject.handle); // bind cubemap texture

    // Configure wrapping mode and texture filtering mode
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Upload testure to each cube face
    //vector<string> faces { "right", "left", "bottom", "top", "back", "front" }; config for earth's skybox version
    vector<string> faces { "right", "left", "bottom", "top", "front", "back" };
    for (auto i = 0; i < faces.size(); ++i) {
        pixel_data pixelData = texture_loader::file(m_resource_path + "textures/skybox/" + faces[i] + ".png");
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, pixelData.channels, pixelData.width, pixelData.height, 0, pixelData.channels, pixelData.channel_type, pixelData.ptr());
    }

    return textureObject;
}

// Initialize scenegraph's hierarchy object (todo-moch: need to refactor)
void ApplicationSolar::initializeSceneGraph() {
    // Initialize sceneGraph obj & Attach root node to it
    auto root = make_shared<Node>("Root");
    SceneGraph::getInstance().setRoot(root);
    auto distanceBetweenPlanetInX = 5.0f; // distance between each planet in X axis

    // Add sun node as a child of root node
    auto sun = make_shared<PointLightNode>("PointLight", fvec3{ 1.0f, 1.0f, 1.0f }, 1.0f);
    auto sunGeo = make_shared<GeometryNode>("Sun Geometry", "planetShader", _planetObject, fvec3{ 1.0f, 1.0f, 1.0f }, initializeTexture("Sun.png"));
    root->addChild(sun);
    sun->addChild(sunGeo);
    sunGeo->setLocalTransform(scale(sunGeo->getLocalTransform(), { 3.0f, 3.0f, 3.0f })); // make sun bigger size
    SceneGraph::getInstance().setDirectionalLight(sun);

    // Add earth node
    auto earth = make_shared<Node>("Earth Holder");
    auto earthGeo = make_shared<GeometryNode>("Earth Geometry", "planetShader", _planetObject, fvec3{ 0.2f, 0.5f, 0.8f }, initializeTexture("Earth.png"));
    auto earthOrbit = make_shared<GeometryNode>("Earth Orbit", "orbitShader", _orbitObject, fvec3{ 0.2f, 0.5f, 0.8f });
    root->addChild(earthOrbit);
    root->addChild(earth);
    earth->addChild(earthGeo);
    earthGeo->setLocalTransform(translate(earthGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set earth geo's position
    earthOrbit->setLocalTransform(scale(earthOrbit->getLocalTransform(), { distanceBetweenPlanetInX, distanceBetweenPlanetInX, distanceBetweenPlanetInX })); // set earth orbit's size
    
    // Add moon as child of earth geometry
    auto moonSize = 0.5f;
    auto moon = make_shared<Node>("Moon Holder");
    auto moonGeo = make_shared<GeometryNode>("Moon Geometry", "planetShader", _planetObject, fvec3{ 0.75f, 0.75f, 0.75f }, initializeTexture("Moon.png"));
    auto moonOrbit = make_shared<GeometryNode>("Moon Orbit", "orbitShader", _orbitObject, fvec3{ 0.75f, 0.75f, 0.75f });
    earthGeo->addChild(moonOrbit);
    earthGeo->addChild(moon);
    moon->addChild(moonGeo);
    moonGeo->setLocalTransform(scale(moonGeo->getLocalTransform(), { moonSize,moonSize,moonSize })); // make moon smaller
    moonGeo->setLocalTransform(translate(moonGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f })); // set moon position
    moonOrbit->setLocalTransform(scale(moonOrbit->getLocalTransform(), { distanceBetweenPlanetInX*moonSize, distanceBetweenPlanetInX*moonSize, distanceBetweenPlanetInX*moonSize })); // set moon orbit size

    // Add remaining 7 planets as children of root node
    map<string, fvec3> planets = {
        {"Mercury", fvec3{0.5f, 0.5f, 0.5f}},
        {"Venus", fvec3{0.95f, 0.92f, 0.84f}},
        {"Mars", fvec3{0.8f, 0.4f, 0.3f}},
        {"Jupiter", fvec3{0.8f, 0.7f, 0.6f}},
        {"Saturn", fvec3{0.9f, 0.85f, 0.75f}},
        {"Uranus", fvec3{0.5f, 0.8f, 0.9f}},
        {"Neptune", fvec3{0.1f, 0.2f, 0.9f}}
    };
    for (const auto& each : planets) {
        auto planet = make_shared<Node>(each.first + " Holder");
        auto planetGeo = make_shared<GeometryNode>(each.first + " Geometry", "planetShader", _planetObject, each.second, initializeTexture(each.first + ".png"));
        auto planetOrbit = make_shared<GeometryNode>(each.first + " Orbit", "orbitShader", _orbitObject, each.second);
        root->addChild(planetOrbit);
        root->addChild(planet);
        planet->addChild(planetGeo);

        // set position and gap between each planet
        distanceBetweenPlanetInX += 5.0f;
        planetGeo->setLocalTransform(translate(planetGeo->getLocalTransform(), { distanceBetweenPlanetInX, 0.0f, 0.0f }));

        // Orbit geo node will be at the same position as sun, and its scale is big as distance between sun and planet geometry
        planetOrbit->setLocalTransform(scale(planetOrbit->getLocalTransform(), { distanceBetweenPlanetInX, distanceBetweenPlanetInX, distanceBetweenPlanetInX }));
    }

    // Add star geometry node and scale its size as big as possible
    auto starGeo = make_shared<GeometryNode>("Star", "starShader", _starObject, fvec3{1.0f, 1.0f, 1.0f});
    starGeo->setLocalTransform(scale(starGeo->getLocalTransform(), { 50.0f, 50.0f, 50.0f }));
    root->addChild(starGeo);

    // Add sky box node and encompass the entire scene
    auto skyboxGeo = make_shared<GeometryNode>("Skybox", "skyboxShader", _skyboxObject, fvec3{ 1.0f, 1.0f, 1.0f }, initializeCubemapTexture());
    skyboxGeo->setLocalTransform(scale(skyboxGeo->getLocalTransform(), { 40.0f, 40.0f, 40.0f }));
    //skyboxGeo->setLocalTransform(rotate(skyboxGeo->getLocalTransform(), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    root->addChild(skyboxGeo);
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

void ApplicationSolar::initializeFrameBuffer(unsigned width, unsigned height) {
    // Generate and bind framebuffer object (1fbo need color attachment, depth attachment and stencil attachment)
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    
    // Create a color attachment texture
    glGenTextures(1, &_screenTexture);
    glBindTexture(GL_TEXTURE_2D, _screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _screenTexture, 0); // attach texture to framebuffer
    
    // Create a renderbuffer object for depth and stencil attachment 
    glGenRenderbuffers(1, &_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo); // attach renderbuffer object to depth and stencil attachment of framebuffer

    // Check if it is actually complete now
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind back to default framebuffer
}
///////////////////////////// intialisation functions /////////////////////////

///////////////////////////// render functions /////////////////////////
void ApplicationSolar::render() const {
    // 1. Render the scene as usual to our new framebuffer
    offScreenRender();

    // 2. Traverse scenegraph to render Geometry node
    auto transformAndDrawGeometry = [this](shared_ptr<Node> node) {
        auto geoNode = dynamic_pointer_cast<GeometryNode>(node);
        if (!geoNode) { return; } // Render only GeometryNode

        // ------------------------ Transformation section ---------------------------
        if (geoNode->getShader() == "planetShader" && geoNode->getName() != "Sun Geometry" && _isRotating) {
            // Rotate GeometryNode's parent, because rightnow all holder node is in the same position as sun
            // Then the rotation of holder will affect position of childe geometry node aswell
            auto parent = geoNode->getParent();
            parent->setLocalTransform(rotate(parent->getLocalTransform(), static_cast<float>(_timer.getElapsedTime() * 10.0f), fvec3{ 0.0f, 1.0f, 0.0f }));
        }
        // ------------------------ End transformation section ------------------------
        
        // ------------------- Shading & Drawing section ------------------------------- 
        // (todo-moch: we can extract rendering process to a method in Node object)
        auto geometry = geoNode->getGeometry();
        auto shaderToUse = geoNode->getShader();
        
        auto geoNodeWorldTransform = geoNode->getWorldTransform();
        auto geoNodeColor = geoNode->getGeometryColor();
        auto geoNodeTexture = geoNode->getTexture();
        auto sunNode = SceneGraph::getInstance().getDirectionalLight();
        auto sunNodeWorldTransform = sunNode->getWorldTransform();
        auto sunNodeColor = sunNode->getLightColor() * sunNode->getLightIntensity();
        auto cameraNode = SceneGraph::getInstance().getCamera();
        auto cameraNodeWorldTransform = cameraNode->getWorldTransform();

        // Bind shader to use
        glUseProgram(m_shaders.at(shaderToUse).handle);
        
        // Upload ModelMatrix & NormalMatrix
        glUniformMatrix4fv(m_shaders.at(shaderToUse).u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(geoNodeWorldTransform)); // Note: glUniformMatrix4fv() is used for per draw call (i.e. uniforms, entire primitive), while glVertexAttribPointer() is used for per vertex
        glm::fmat4 normalMatrix = glm::inverseTranspose(glm::inverse(cameraNodeWorldTransform) * geoNodeWorldTransform);
        glUniformMatrix4fv(m_shaders.at(shaderToUse).u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix)); // extra matrix for normal transformation to keep them orthogonal to surface

        // Upload light attribute to fragment shader
        if (shaderToUse == "planetShader") {
            // glUniform3fv(m_shaders.at(shaderToUse).u_locs.at("GeometryColor"), 1, glm::value_ptr(geoNodeColor));
            glUniform3fv(m_shaders.at(shaderToUse).u_locs.at("AmbientColor"), 1, glm::value_ptr(fvec3{ 1.0f, 1.0f, 1.0f }));
            glUniform1f(m_shaders.at(shaderToUse).u_locs.at("AmbientStrength"), geoNode->getName() == "Sun Geometry" ? sunNode->getLightIntensity() : 0.2f);
            glUniform3fv(m_shaders.at(shaderToUse).u_locs.at("LightColor"), 1, glm::value_ptr(sunNodeColor));
            glUniform3fv(m_shaders.at(shaderToUse).u_locs.at("LightPosition"), 1, glm::value_ptr(sunNodeWorldTransform * glm::vec4{ 0, 0, 0, 1 })); // Make sure 
            glUniform3fv(m_shaders.at(shaderToUse).u_locs.at("CameraPosition"), 1, glm::value_ptr(cameraNodeWorldTransform * glm::vec4{ 0, 0, 0, 1 }));
            glUniform1b(m_shaders.at(shaderToUse).u_locs.at("EnableToonShading"), _enableToonShading);
        }

        // Select texture, access it and upload texture data to shader program
        if (shaderToUse == "planetShader" || shaderToUse == "skyboxShader") {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(geoNodeTexture.target, geoNodeTexture.handle);
            glUniform1i(m_shaders.at(shaderToUse).u_locs.at("Texture"), 0);
        }

        // Draw VBO
        glBindVertexArray(geometry.vertex_AO);
        if (shaderToUse == "planetShader") {
            glDrawElements(geometry.draw_mode, geometry.num_elements, model::INDEX.type, NULL);
        }
        else {
            glDrawArrays(geometry.draw_mode, 0, geometry.num_elements);
        }
        // ------------------- End drawing section --------------------------
    };
    SceneGraph::getInstance().getRoot()->traverse(transformAndDrawGeometry);

    // 3. Draw a quad that spans the entire screen with the new framebuffer's color buffer as its texture.
    renderScreenTextureToQuadObject();
}

void ApplicationSolar::offScreenRender() const {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);               // make sure we clear the framebuffer's content every frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glEnable(GL_DEPTH_TEST);                            // enable depth testing
}

void ApplicationSolar::renderScreenTextureToQuadObject() const {
    // Bind back to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);       // Set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw a quad plane with the attached framebuffer color texture
    glDisable(GL_DEPTH_TEST);                   // Disabling depth testing since we want to make sure the quad always renders in front of everything else
    glUseProgram(m_shaders.at("quadShader").handle);
    glBindVertexArray(_screenQuadObject.vertex_AO);
    glBindTexture(GL_TEXTURE_2D, _screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ApplicationSolar::uploadView() {
    // vertices are transformed in camera space, so camera transform must be inverted
    fmat4 viewMatrix = glm::inverse(SceneGraph::getInstance().getCamera()->getWorldTransform());
    
    // upload matrix to gpu
    for (auto& const each : _shaderList) {
        glUseProgram(m_shaders.at(each.first).handle);
        glUniformMatrix4fv(m_shaders.at(each.first).u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    }
    
    // For quad.frag
    glUseProgram(m_shaders.at("quadShader").handle);
    glUniform1i(m_shaders.at("quadShader").u_locs.at("ScreenTexture"), 0);
    glUniform1b(m_shaders.at("quadShader").u_locs.at("EnableVerticalMirror"), _enableVericallMirror);
    glUniform1b(m_shaders.at("quadShader").u_locs.at("EnableHorizontalMirror"), _enableHorizontalMirror);
    glUniform1b(m_shaders.at("quadShader").u_locs.at("EnableGrayscale"), _enableGrayscale);
    glUniform1b(m_shaders.at("quadShader").u_locs.at("EnableBlur"), _enableBlur);
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
///////////////////////////// render functions /////////////////////////

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
    } else if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        _isRotating = !_isRotating;
    } else if (key == GLFW_KEY_1 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        _enableToonShading = !_enableToonShading;
    } else if (key == GLFW_KEY_7 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
         _enableGrayscale = !_enableGrayscale;
         uploadView();
    } else if (key == GLFW_KEY_8 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
         _enableHorizontalMirror = !_enableHorizontalMirror;
         uploadView();
    } else if (key == GLFW_KEY_9 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        _enableVericallMirror = !_enableVericallMirror;
        uploadView();
    } else if (key == GLFW_KEY_0 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        _enableBlur = !_enableBlur;
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
  SceneGraph::getInstance().getCamera()->setProjectionMatrix(utils::calculate_projection_matrix(float(width) / float(height))); // recalculate projection matrix for new aspect ration
  uploadProjection(); // upload new projection matrix
  initializeFrameBuffer(width, height); // Re-size window impact frame buffer !!
}

///////////////////////////// exe entry point /////////////////////////////
int main(int argc, char* argv[]) {
  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}