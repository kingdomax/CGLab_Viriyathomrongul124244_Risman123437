#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include "Timer.hpp"
#include <map>
#include <string>
using std::map;
using std::string;

// gpu representation of model
class ApplicationSolar : public Application {
	public:
		// allocate and initialize objects
		ApplicationSolar(std::string const& resource_path);
		// free allocated objects
		~ApplicationSolar();
		// react to key input
		void keyCallback(int key, int action, int mods);
		//handle delta mouse movement input
		void mouseCallback(double pos_x, double pos_y);
		//handle resizing
		void resizeCallback(unsigned width, unsigned height);
		// draw all objects
		void render() const;

	private:
		// initialize scenegraph's hierarchy object
		void initializeSceneGraph();
		// setup camera node
		void initializeCamera(glm::fmat4 camInitialTransform, glm::fmat4 camInitialProjection);
		void initializeFrameBuffer(unsigned width, unsigned height);
		void offScreenRender() const;
		void renderScreenTextureToQuadObject() const;
		// timer class
		mutable Timer _timer;
		// key=shader name, value=file name
		map<string, string> _shaderList;
		bool _isRotating;
		bool _enableToonShading;
		bool _enableHorizontalMirror;
		bool _enableVericallMirror;
		bool _enableBlur;
		bool _enableGrayscale;
		unsigned int _fbo; // frame buffer object
		unsigned int _rbo; // render buffer object
		unsigned int _screenTexture; // texture

	protected:
		void initializeShaderPrograms();
		void initializeGeometry();
		texture_object initializeTexture(const string& textureFile);
		texture_object initializeCubemapTexture();
		// update uniform values
		void uploadUniforms();
		// upload projection matrix
		void uploadProjection();
		// upload view matrix
		void uploadView();

		// cpu representation of model
		model_object _planetObject;
		model_object _starObject;
		model_object _orbitObject;
		model_object _skyboxObject;
		model_object _screenQuadObject;

		// camera transform matrix
		glm::fmat4 m_view_transform;
		// camera projection matrix
		glm::fmat4 m_view_projection;
};

#endif