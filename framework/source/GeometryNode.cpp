#include "Node.hpp"
#include "structs.hpp"
#include "GeometryNode.hpp"
#include <string>
using std::string;

GeometryNode::GeometryNode(string name, string shader, model_object geo) :
    Node(name),
    _shader(shader),
    _geometry(geo)
{

}

string GeometryNode::getShader() { return _shader; }
model_object GeometryNode::getGeometry() { return _geometry; }
void GeometryNode::setGeometry(model_object geoModel) { _geometry = geoModel; }