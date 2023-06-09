#include "Node.hpp"
#include "PointLightNode.hpp"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
using glm::fvec3;
using std::string;

PointLightNode::PointLightNode(string name, fvec3 lightColor, float lightIntensity) :
    Node(name),
    _lightColor(lightColor),
    _lightIntensity(lightIntensity){
}

fvec3 PointLightNode::getLightColor() { return _lightColor; }
float PointLightNode::getLightIntensity() { return _lightIntensity; }