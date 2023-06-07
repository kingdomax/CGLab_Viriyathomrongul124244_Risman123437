#pragma once
#include "Node.hpp"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
using std::string;
using glm::fvec3;

class PointLightNode : public Node {
	public:
		PointLightNode(string name, fvec3 lightColor, float lightIntensity);
		fvec3 getLightColor();
		float getLightIntensity();

	private: 
		fvec3 _lightColor;
		float _lightIntensity;
};