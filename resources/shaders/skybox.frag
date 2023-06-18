#version 150

uniform samplerCube Texture;

in vec3 texture_coordinate;
out vec4 out_Color;

void main() {
    out_Color = texture(Texture, texture_coordinate);
}