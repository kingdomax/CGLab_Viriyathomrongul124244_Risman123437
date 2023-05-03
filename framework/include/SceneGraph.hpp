#pragma once
#include <string>
#include <memory>
#include "Node.hpp"
#include "CameraNode.hpp"
using std::string;
using std::shared_ptr;

class SceneGraph {
    public:
        SceneGraph();
        static SceneGraph& getInstance(); // get SceneGraph instance (singleton)
        string getName();
        void setName(string name);
        shared_ptr<Node> getRoot(); // get root node in this scenegraph
        void setRoot(shared_ptr<Node> rootNode);
        shared_ptr<CameraNode> getCamera(); // get a camera node in this scenegraph
        void setCamera(shared_ptr<CameraNode> cameraNode);
        void printGraph();

    private:
        string _name;
        shared_ptr<Node> _root;
        shared_ptr<CameraNode> _camera;
};
