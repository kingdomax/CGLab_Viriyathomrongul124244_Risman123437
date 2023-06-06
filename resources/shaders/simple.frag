#version 150

const int SpecularShinessSize = 32;
const float SpecularStrength = 0.5f;
const float AmbientStrength = 0.1f;

uniform vec3 SunColor;
uniform vec3 SunPosition;
uniform vec3 AmbientColor; // hint: ambient color can be the same as the diffuuse color and specular color can be white
uniform vec3 GeometryColor;
uniform vec3 OutlineColor;
uniform vec3 CameraPosition;

in vec3 normal_vector;
in vec3 fragment_position;
out vec4 out_Color;

// https://learnopengl.com/Lighting/Basic-Lighting
void main() {
    // 0) Normalized all variables in lighting equation
    vec3 normalVector = normalize(normal_vector);
    vec3 lightDirection = normalize(SunPosition - fragment_position);   // Calculate lighting vector
    vec3 viewDirection = normalize(CameraPosition - fragment_position); // Calculate view vector

    // 1) Ambient light
    vec3 ambientLight = AmbientColor * AmbientStrength;

    // 2) Diffuse light
    float diffuseIntensity = max(dot(lightDirection, normalVector), 0.0); // Calculate the diffuse impact of the light on the current fragment
    vec3 diffuseLight = diffuseIntensity * SunColor;

    // 3) Specular light
    vec3 reflectDirection = reflect(-lightDirection, normalVector); // Calculate a reflection vector by reflecting the light direction around the normal vector
    float vDotR = max(dot(viewDirection, reflectDirection), 0.0); // Calculate the angle distance between this reflection vector and the view direction
    float specularIntensity = pow(vDotR, SpecularShinessSize); // Calculate the specular component
    vec3 specularLight = specularIntensity * SpecularStrength * SunColor; 
  
    // 4) Toon Shading (20%)
    // 4.1 Further step of color discretisation, if colour > threshold then m colour1 else m colour2
    // 4.2 In order to detect a relevant edge you should calculate the dot product between the surface normal and the view direction
    //     If this value is above a threshold assign to that fragment the outlineColor

    // 5. Return fragment color
    vec3 result = (ambientLight + diffuseLight + specularLight) * GeometryColor;
    out_Color = vec4(result, 1.0);
}
