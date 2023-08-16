#ifndef WAVES_H
#define WAVES_H

#include <raylib.h>
#include <raymath.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_WATER_WAVES 6

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct
{
    int     enabled;
    Vector2 displacement;
    float   amplitude;
    float   speed;
    float   frequency;
    Vector2 direction;

    // Shader locations
    int enabled_loc;
    int displacement_loc;
    int amplitude_loc;
    int speed_loc;
    int frequency_loc;
    int direction_loc;
} WaterWave;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Copy wave data from CPU to GPU 
     */
    void UpdateWaterWaveValues(Shader shader, WaterWave wave);

    /**
     * Allocate wave on CPU and GPU 
     */
    WaterWave CreateWave(Vector2 displacement, float amplitude, float speed, float frequency, Vector2 direction, Shader shader);

    /**
     * Calculate displacement of a wave at position <x, z> and at the time <t>
     */
    Vector3 CircularWaveFunction(float x, float z, float t, WaterWave wave);

    /**
     * Calculate new position of a wave at position <pos> and at the time <t>
     */
    Vector3 WaveFunction(Vector3 pos, float t, WaterWave wave);

    /**
     * Calculate new position of a group of waves at position <pos> and at the time <t>
     */
    Vector3 GroupWaveFunction(Vector3 pos, float t, WaterWave* waves, size_t num_waves);

#ifdef __cplusplus
}
#endif

#endif // WAVES_H

/***********************************************************************************
*
*   WAVES IMPLEMENTATION
*
************************************************************************************/

#if defined(WAVES_IMPLEMENTATION)

void UpdateWaterWaveValues(Shader shader, WaterWave wave)
{
    SetShaderValue(shader, wave.enabled_loc, &wave.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, wave.displacement_loc, &wave.displacement, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, wave.amplitude_loc, &wave.amplitude, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, wave.speed_loc, &wave.speed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, wave.frequency_loc, &wave.frequency, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, wave.direction_loc, &wave.direction, SHADER_UNIFORM_VEC2);
}

WaterWave CreateWave(Vector2 displacement, float amplitude, float speed, float frequency, Vector2 direction, Shader shader)
{
    static int wave_count = 0;

    if(wave_count >= MAX_WATER_WAVES)
    {
        printf("WARNING: trying to create new water wave, but water wave buffer is already full\n");
        return CLITERAL(WaterWave) { 0 };
    }

    float dot = direction.x * direction.x + direction.y * direction.y;

    WaterWave wave = (WaterWave)
    {
        true,
        displacement,
        amplitude,
        speed,
        frequency,
        (Vector2) { direction.x / dot, direction.y / dot },

        GetShaderLocation(shader, TextFormat("waves[%i].enabled", wave_count)),
        GetShaderLocation(shader, TextFormat("waves[%i].displacement", wave_count)),
        GetShaderLocation(shader, TextFormat("waves[%i].amplitude", wave_count)),
        GetShaderLocation(shader, TextFormat("waves[%i].speed", wave_count)),
        GetShaderLocation(shader, TextFormat("waves[%i].frequency", wave_count)),
        GetShaderLocation(shader, TextFormat("waves[%i].direction", wave_count)),
    };

    UpdateWaterWaveValues(shader, wave);

    wave_count++;

    return wave;
}

Vector3 CircularWaveFunction(float x, float z, float t, WaterWave wave)
{
    float new_y = wave.amplitude * sinf((wave.displacement.x + x) * wave.direction.x * wave.frequency + 
                                        (wave.displacement.y + z) * wave.direction.y * wave.frequency + 
                                        t * wave.speed);
    float new_z = wave.amplitude * cosf((wave.displacement.x + x) * wave.direction.x * wave.frequency + 
                                        (wave.displacement.y + z) * wave.direction.y * wave.frequency + 
                                        t * wave.speed);

    return (Vector3)
    {
        new_z * wave.direction.x,
        new_y,
        new_z * wave.direction.y
    };
}

Vector3 WaveFunction(Vector3 pos, float t, WaterWave wave)
{
    return Vector3Add(pos, CircularWaveFunction(pos.x, pos.z, t, wave));
}

Vector3 GroupWaveFunction(Vector3 pos, float t, WaterWave* waves, size_t num_waves)
{
    Vector3 displacement = (Vector3) { 0, 0, 0 };
    for(size_t i = 0; i < num_waves; i++)
    {
        displacement = Vector3Add(displacement, CircularWaveFunction(pos.x, pos.z, t, waves[i]));
    }
    return Vector3Add(pos, displacement);
}

#endif