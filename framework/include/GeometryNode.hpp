#pragma once
#include "Node.hpp"
#include "structs.hpp"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
using glm::fvec3;
using std::string;

class GeometryNode : public Node {
	public:
		GeometryNode(string name, string shader, model_object geometry, fvec3 geoColor, texture_object texture = texture_object());
		string getShader();
		fvec3 getGeometryColor();
		model_object getGeometry();
		texture_object getTexture();
		void setGeometry(model_object geoModel);

	private:
		string _shader;
		fvec3 _geoColor;
		model_object _geometry;
		texture_object _texture;
};