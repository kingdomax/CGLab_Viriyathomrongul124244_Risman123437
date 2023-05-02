#include "Node.hpp"
#include "SceneGraph.hpp"
#include <string>
#include <memory>
#include <iostream>
using std::string;
using std::shared_ptr;

SceneGraph::SceneGraph() { }

// Public method
string SceneGraph::getName() { return _name; }
shared_ptr<Node> SceneGraph::getRoot() { return _root; }
SceneGraph& SceneGraph::getInstance() {
    static SceneGraph instance;
    return instance;
}
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

// Private method
void SceneGraph::setName(string name) { _name = name; }
void SceneGraph::setRoot(shared_ptr<Node> rootNode) { _root = rootNode; }
