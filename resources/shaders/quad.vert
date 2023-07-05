#version 150
#extension GL_ARB_explicit_attrib_location : require

// Not going to include fancy matrix transformations since we'll be supplying the vertex coordinates as normalized device coordinates
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texture_coordinate;
out vec2 texture_coordinate;

void main() {
    texture_coordinate = in_texture_coordinate;
    gl_Position = vec4(in_position.x, in_position.y, 0.0, 1.0);
}
