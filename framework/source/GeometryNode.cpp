#include "Node.hpp"
#include "GeometryNode.hpp"
#include <string>
#include <model.hpp>
using std::string;

GeometryNode::GeometryNode(string name, string shader, model geo) : 
    Node(name),
    _shader(shader),
    _geometry(geo)
{

}

model GeometryNode::getGeometry() { return _geometry; }
void GeometryNode::setGeometry(model geoModel) { _geometry = geoModel; }