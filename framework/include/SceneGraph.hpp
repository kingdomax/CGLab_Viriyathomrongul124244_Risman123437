#pragma once
#include <string>
#include <memory>
#include "Node.hpp"
using std::string;
using std::shared_ptr;

class SceneGraph {
    public:
        SceneGraph();
        shared_ptr<Node> getRoot();
        string getName();
        string printGraph();

    private:
        shared_ptr<Node> _root;
        string _name;
        void setRoot(shared_ptr<Node> rootNode);
        void setName(string name);
};
