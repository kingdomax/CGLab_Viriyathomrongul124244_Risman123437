#pragma once
#include "Node.hpp"
#include "structs.hpp"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
using glm::fvec3;
using std::string;

class GeometryNode : public Node {
	public:
		GeometryNode(string name, string shader, model_object geometry, fvec3 geoColor);
		string getShader();
		fvec3 getGeometryColor();
		model_object getGeometry();
		void setGeometry(model_object geoModel);

	private:
		string _shader;
		fvec3 _geoColor;
		model_object _geometry;
};