#pragma once
#include "Node.hpp"
#include <string>
#include <model.hpp>
using std::string;

class GeometryNode : public Node {
	public:
		GeometryNode(string name, model geometry);
		model getGeometry();
		void setGeometry(model geoModel);

	private:
		model _geometry;
};