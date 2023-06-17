#include "Node.hpp"
#include "structs.hpp"
#include "GeometryNode.hpp"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
using glm::fvec3;
using std::string;

GeometryNode::GeometryNode(string name, string shader, model_object geo, fvec3 geoColor, texture_object texture) :
    Node(name),
    _shader(shader),
    _geometry(geo),
    _geoColor(geoColor),
    _texture(texture)
{ }

string GeometryNode::getShader() { return _shader; }
fvec3 GeometryNode::getGeometryColor() { return _geoColor; }
model_object GeometryNode::getGeometry() { return _geometry; }
texture_object GeometryNode::getTexture() { return _texture; }
void GeometryNode::setGeometry(model_object geoModel) { _geometry = geoModel; }