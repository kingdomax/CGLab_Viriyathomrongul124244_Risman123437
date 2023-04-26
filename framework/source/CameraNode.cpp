#include "Node.hpp"
#include "CameraNode.hpp"
#include <glm/gtc/matrix_transform.hpp>
using glm::mat4;

CameraNode::CameraNode(string name, mat4 localTransform) :
    Node(name, localTransform) {

}

bool CameraNode::getPerspective() { return _isPerspective; }

bool CameraNode::getEnabled() { return _isEnabled; }
void CameraNode::setEnabled(bool isEnable) { _isEnabled = isEnable;  }

mat4 CameraNode::getProjectionMatrix() { return _projectionMatrix; }
void CameraNode::setProjectionMatrix(mat4 projMat) { _projectionMatrix = projMat; }