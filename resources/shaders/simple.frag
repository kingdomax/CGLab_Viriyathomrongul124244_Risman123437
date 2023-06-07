#version 150

const float TonnShadingBin = 1;
const int SpecularShinessSize = 16;
const float SpecularStrength = 0.5f;
const vec4 OutlineColor = vec4(0.0f, 0.88f, 1.0f, 1.0f);

uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 AmbientColor;
uniform float AmbientStrength;
uniform vec3 GeometryColor;
uniform vec3 CameraPosition;
uniform bool EnableToonShading;

in vec3 normal_vector;
in vec3 fragment_position;
out vec4 out_Color;

// https://learnopengl.com/Lighting/Basic-Lighting
void main() {
    // 0) Normalized all variables in lighting equation
    vec3 normalVector = normalize(normal_vector);
    vec3 lightDirection = normalize(LightPosition - fragment_position);   // Calculate lighting vector
    vec3 viewDirection = normalize(CameraPosition - fragment_position); // Calculate view vector

    // 1) Ambient light
    vec3 ambientLight = AmbientColor * AmbientStrength;

    // 2) Diffuse light
    float diffuseIntensity = max(dot(lightDirection, normalVector), 0.0); // Calculate the diffuse impact of the light on the current fragment
    vec3 diffuseLight = diffuseIntensity * LightColor;

    // 3) Specular light
    vec3 reflectDirection = reflect(-lightDirection, normalVector); // Calculate a reflection vector by reflecting the light direction around the normal vector
    float vDotR = max(dot(viewDirection, reflectDirection), 0.0); // Calculate the angle distance between this reflection vector and the view direction
    float specularIntensity = pow(vDotR, SpecularShinessSize); // Calculate the specular component
    vec3 specularLight = specularIntensity * SpecularStrength * LightColor; 
  
    // 4) Toon Shading
    if (EnableToonShading) {
      float edgeAngle = dot(normalVector, viewDirection); // Detect a relevant edge by calculate the dot product between the surface normal and the view direction
      if (edgeAngle > 0.0f && edgeAngle <= 0.2f) {
        out_Color = OutlineColor;
        return;
      } else {
        // color quantization
        diffuseLight = ceil(diffuseLight * TonnShadingBin) / TonnShadingBin;
      }
    }

    // 5. Return fragment color
    vec3 result = (ambientLight + diffuseLight + specularLight) * GeometryColor;
    out_Color = vec4(result, 1.0);
}
