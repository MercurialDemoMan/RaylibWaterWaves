#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    finalColor  = vec4(clamp((102.0+fragPosition.y*20.0)/255.0, 0.0, 1.0), 
                       clamp((191.0+fragPosition.y*20.0)/255.0, 0.0, 1.0), 
                       255.0/255.0, 1.0);

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}