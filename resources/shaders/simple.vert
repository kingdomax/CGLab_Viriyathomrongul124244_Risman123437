#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TextureCoordinate;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec3 normal_vector;
out vec3 fragment_position;
out vec2 texture_coordinate;

// https://learnopengl.com/Lighting/Basic-Lighting
void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	fragment_position = vec3(ModelMatrix * vec4(in_Position, 1.0)); 					// Generate actual fragment position in to world space
	normal_vector = vec3(inverse(transpose(ModelMatrix)) * vec4(in_Normal, 1.0)); 			// Generate the normal vector by using the inverse and transpos
	texture_coordinate = in_TextureCoordinate;
}
