#pragma once
#include "Node.hpp"
#include <string>
#include <model.hpp>
using std::string;

class GeometryNode : public Node {
	public:
		GeometryNode(string name, string shader, model geometry);
		string getShader();
		model getGeometry();
		void setGeometry(model geoModel);

	private:
		string _shader;
		model _geometry;
};