#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform float time;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

#define MAX_WATER_WAVES 6

struct WaterWave
{
    int   enabled;
    vec2  displacement;
    float amplitude;
    float speed;
    float frequency;
    vec2  direction;
};

uniform WaterWave waves[MAX_WATER_WAVES];

vec3 wave(WaterWave wave, vec3 pos)
{
    float new_y = wave.amplitude * sin((wave.displacement.x + pos.x) * wave.direction.x * wave.frequency +
                                       (wave.displacement.y + pos.z) * wave.direction.y * wave.frequency +
                                       time * wave.speed);

    float new_z = wave.amplitude * cos((wave.displacement.x + pos.x) * wave.direction.x * wave.frequency +
                                       (wave.displacement.y + pos.z) * wave.direction.y * wave.frequency +
                                       time * wave.speed);
                                       
    return vec3(new_z * wave.direction.x, new_y, new_z * wave.direction.y);
}

vec3 group_wave(vec3 pos)
{
    vec3 displacement = vec3(0);
    for(int i = 0; i < MAX_WATER_WAVES; i++)
    {
        if(waves[i].enabled == 1)
        {
            displacement += wave(waves[i], pos);
        }
    }
    return pos + displacement;
}

void main()
{
    // Sample wave points to calculate normal
    vec3 a = group_wave(vertexPosition);
    vec3 b = group_wave(vertexPosition + vec3(1, 0, 0));
    vec3 c = group_wave(vertexPosition + vec3(0, 0, 1));

    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(a, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(cross(c - a, b - a), 1.0)));

    // Calculate final vertex position
    gl_Position = mvp*vec4(a, 1.0);
}