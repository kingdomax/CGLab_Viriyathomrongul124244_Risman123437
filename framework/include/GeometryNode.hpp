#pragma once
#include "Node.hpp"
#include "structs.hpp"
#include <string>
using std::string;

class GeometryNode : public Node {
	public:
		GeometryNode(string name, string shader, model_object geometry);
		string getShader();
		model_object getGeometry();
		void setGeometry(model_object geoModel);

	private:
		string _shader;
		model_object _geometry;
};