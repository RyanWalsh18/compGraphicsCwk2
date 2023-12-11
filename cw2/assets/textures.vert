#version 430

// Input attributes
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;
layout(location = 2) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord;


// Uniform variable (no location qualifier needed for uniforms)
layout (location = 0) uniform mat4 uProjCameraWorld;
layout (location = 1) uniform mat3 uNormalMatrix;

// Output attributes
out vec3 v2fColor;
out vec3 v2fNormal;
out vec2 v2fTexCoord;


// Main function, run for each vertex processed
void main()
{
    // Copy input color to output color attribute
    v2fColor = iColor;
    v2fTexCoord = iTexCoord;
    v2fNormal = normalize(uNormalMatrix * iNormal);
    
    // Copy position to built-in gl_Position attribute
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    
}