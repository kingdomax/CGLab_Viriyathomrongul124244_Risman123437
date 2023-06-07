#include "Node.hpp"
#include "SceneGraph.hpp"
#include "CameraNode.hpp"
#include "PointLightNode.hpp"
#include <string>
#include <memory>
#include <iostream>
using std::string;
using std::shared_ptr;
using std::dynamic_pointer_cast;

SceneGraph::SceneGraph() { }

SceneGraph& SceneGraph::getInstance() {
    static SceneGraph instance;
    return instance;
}

string SceneGraph::getName() { return _name; }
void SceneGraph::setName(string name) { _name = name; }

shared_ptr<Node> SceneGraph::getRoot() { return _root; }
void SceneGraph::setRoot(shared_ptr<Node> rootNode) { _root = rootNode; }

shared_ptr<PointLightNode> SceneGraph::getDirectionalLight() { return _dirLight; }
void SceneGraph::setDirectionalLight(shared_ptr<PointLightNode> dirLight) { _dirLight = dirLight; }

shared_ptr<CameraNode> SceneGraph::getCamera() { return _camera; }
void SceneGraph::setCamera(shared_ptr<CameraNode> cameraNode) { _camera = cameraNode; }

void SceneGraph::printGraph() {
    std::cout << "------------ SceneGraph ------------" << std::endl;
    auto printName = [this](shared_ptr<Node> node) {
        std::string empty_string(node->getDepth() * 2, '    ');
        std::cout << empty_string << node->getName() << std::endl;
    };
    printName(_root);
    _root->traverse(printName);
    std::cout << "------------------------------------" << std::endl;
}
