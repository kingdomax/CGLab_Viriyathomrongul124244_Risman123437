#pragma once
#include <list>
#include <string>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
using std::list;
using std::string;
using std::shared_ptr;
using glm::mat4;

class Node {
    public:
        Node(string name);
        shared_ptr<Node> getParent();
        void setParent(Node* parentNode);
        shared_ptr<Node> getChild(string childName);
        list<shared_ptr<Node>> getChildrenList();
        string getName();
        string getPath();
        int getDepth();
        void setDepth(int newDepth);
        mat4 getLocalTransform();
        void setLocalTransform(mat4 localTransform);
        mat4 getWorldTransform();
        void setWorldTransform(mat4 worldTransform);
        void addChild(shared_ptr<Node> child);
        shared_ptr<Node> removeChild(string childName);

    private:
        shared_ptr<Node> _parent;
        list<shared_ptr<Node>> _children;
        string _name;
        string _path;
        int _depth;
        mat4 _localTransform;
        mat4 _worldTransform;
};
