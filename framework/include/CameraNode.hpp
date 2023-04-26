#pragma once
#include "Node.hpp"
#include <glm/gtc/matrix_transform.hpp>
using glm::mat4;

class CameraNode : public Node {
	public:
		CameraNode(string name, mat4 localTransform);
		bool getPerspective();
		bool getEnabled();
		void setEnabled(bool isEnable);
		mat4 getProjectionMatrix();
		void setProjectionMatrix(mat4 projMat);

	private:
		bool _isEnabled;
		bool _isPerspective;
		mat4 _projectionMatrix;
};
