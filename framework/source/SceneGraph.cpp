#include "Node.hpp"
#include "SceneGraph.hpp"
#include <string>
#include <memory>
using std::string;
using std::shared_ptr;

SceneGraph::SceneGraph() { }

// Public method
string SceneGraph::getName() { return _name; }
shared_ptr<Node> SceneGraph::getRoot() { return _root; }
string SceneGraph::printGraph() {
    return ""; // todo-moch: need to implement
}
SceneGraph& SceneGraph::getInstance() {
    static SceneGraph instance;
    return instance;
}

// Private method
void SceneGraph::setName(string name) { _name = name; }
void SceneGraph::setRoot(shared_ptr<Node> rootNode) { _root = rootNode; }
