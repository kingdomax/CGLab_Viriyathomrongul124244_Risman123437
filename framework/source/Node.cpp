#include "Node.hpp"
#include <list>
#include <string>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
using std::list;
using std::string;
using std::shared_ptr;
using glm::mat4;

Node::Node(string name) :
    _name(name),
    _depth(0),
    _children() {
}

// ------------- Get own attribute method -------------
string Node::getName() { return _name; }
int Node::getDepth() { return _depth; }
void Node::setDepth(int newDepth) { _depth = newDepth; }
string Node::getPath() { return _path; } // Not sure when/how we're going to use it, so leave the implement for now

// ------------- Get transform methods -------------
mat4 Node::getLocalTransform() { return _localTransform; }
void Node::setLocalTransform(mat4 localTransform) { _localTransform = _localTransform; }
void Node::setWorldTransform(mat4 worldTransform) { _worldTransform = worldTransform; }
mat4 Node::getWorldTransform() {
    // Return world coordinate of this node by multiplying own transform with parent's transform
    return _depth != 0 ? _parent->getWorldTransform() * _localTransform : _localTransform;
}

// ------------- Getter/Setter node methods -------------
shared_ptr<Node> Node::getParent() { return _parent; }
void Node::setParent(Node* parentNode) { _parent = shared_ptr<Node>(parentNode); }
list<shared_ptr<Node>> Node::getChildrenList() { return _children; }
shared_ptr<Node> Node::getChild(string childName) {
    for (auto child : _children) {
        if (child->getName() == childName) { return child; } // Return child node that has specific name
    }
    return nullptr;
}
void Node::addChild(shared_ptr<Node> child) {
    child->setDepth(_depth+1); // Set child's depth correctly
    child->setParent(this);
    _children.push_back(child); // Then we can add it to own children list
}
shared_ptr<Node> Node::removeChild(string childName) {
    for (auto child : _children) {
        if (child->getName() == childName) {
            child->setDepth(0); // Reset child's depth because we remove the Node from scenegraph's hierarchy
            child->setParent(nullptr); // Set null pointer to child's parent
            _children.remove(child); // Then we can remove that node from own children list
            return child;
        }
    }
    return nullptr;
}
