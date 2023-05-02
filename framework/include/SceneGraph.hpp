#pragma once
#include <string>
#include <memory>
#include "Node.hpp"
using std::string;
using std::shared_ptr;

class SceneGraph {
    public:
        SceneGraph();
        string getName();
        void printGraph();
        shared_ptr<Node> getRoot(); // get root node in this scenegraph
        static SceneGraph& getInstance(); // get singleton instance
        void setName(string name);
        void setRoot(shared_ptr<Node> rootNode);

    private:
        string _name;
        shared_ptr<Node> _root;
};
