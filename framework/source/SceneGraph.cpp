#include "Node.hpp"
#include "SceneGraph.hpp"
#include <string>
#include <memory>
using std::string;
using std::shared_ptr;

SceneGraph::SceneGraph() { 

}

// Public method
shared_ptr<Node> SceneGraph::getRoot() { return _root; }
string SceneGraph::getName() { return _name; }
string SceneGraph::printGraph() {
    return ""; // todo-moch: need to implement
}

// Private method
void SceneGraph::setRoot(shared_ptr<Node> rootNode) { _root = rootNode; }
void SceneGraph::setName(string name) { _name = name; }
