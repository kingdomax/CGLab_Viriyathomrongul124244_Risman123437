#version 150

in vec2 texture_coordinate;
out vec4 out_Color;

uniform sampler2D ScreenTexture;
uniform bool EnableHorizontalMirror;
uniform bool EnableVerticalMirror;
uniform bool EnableGrayscale;
uniform bool EnableBlur;

// Postprocessing order is matter
// https://learnopengl.com/Advanced-OpenGL/Framebuffers
void main() {
    // 1. Inverting the texture coordinates on x axis or y axis
    vec2 target_texture = texture_coordinate;
    if (EnableHorizontalMirror) { target_texture.y = 1.0 - texture_coordinate.y; }
    if (EnableVerticalMirror) { target_texture.x = 1.0 - texture_coordinate.x; }
    out_Color = texture(ScreenTexture, target_texture); 

    // 2. Blur
    if (EnableBlur) {
        // Define an array for each surrounding texture coordinate
        const float offset = 1.0 / 300.0;
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right    
        );

        // 3x3 Blur Gaussian Kernel
        float gaussian_kernel[9] = float[](
            1.0/16, 1.0/8, 1.0/16,
            1.0/8,  1.0/4, 1.0/8,
            1.0/16, 1.0/8, 1.0/16
        );
       
        // Calculate color: by sampling and multiply texture values with the blur kernel values 
        vec3 result = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i < 9; ++i) {
            result += vec3(texture(ScreenTexture, target_texture.st + offsets[i])) * gaussian_kernel[i];
        }

        out_Color = vec4(result, 1.0);
    }

    // 3. Luminance Preserving Grayscale
    if (EnableGrayscale) {
        float average = (0.2126 * out_Color.r + 0.7152 * out_Color.g + 0.0722 * out_Color.b);
        out_Color = vec4(average, average, average, 1.0);
    }
}