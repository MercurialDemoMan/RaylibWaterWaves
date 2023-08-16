#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "common.inl"

#define LIGHTS_IMPLEMENTATION
#include "lights.h"

#define WAVES_IMPLEMENTATION
#include "waves.h"

#define SHIP_ROTATE_FACTOR  20
#define WATER_SURFACE_WIDTH 200
#define BACKGROUND_COLOR    CLITERAL(Color) { 120, 190, 255, 255 }

typedef struct
{
    Model model;
    float front_angle;
    float side_angle;
    Vector3 position;
} Ship;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Global time tracking
    float time = 0.0f;

    // Initialize raylib and window
    InitWindow(1920, 1080, "Water Waves");

    // Start in fullscreen mode
    if(!IsWindowFullscreen())
        ToggleFullscreen();

    // Load models
    Ship ship = (Ship)
    {
        LoadModel("models/ship.obj"),
        0.0,
        0.0,
        (Vector3){0, 0, 0}
    };
    Model water_model = LoadModelFromMesh(GenMeshPlane(WATER_SURFACE_WIDTH, WATER_SURFACE_WIDTH, 
                                                       WATER_SURFACE_WIDTH, WATER_SURFACE_WIDTH));

    // Initialize shaders
    Shader water_shader = LoadShader("shaders/water.vs", "shaders/water.fs");
    water_shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(water_shader, "viewPos");

    Shader material_shader = LoadShader("shaders/material.vs", "shaders/material.fs");
    material_shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(material_shader, "viewPos");

    int water_shader_time_loc = GetShaderLocation(water_shader, "time");

    SetShaderValue(water_shader, GetShaderLocation(water_shader, "ambient"), (float[4]) {0.1, 0.1, 0.1, 1.0}, SHADER_UNIFORM_VEC4);
    SetShaderValue(material_shader, GetShaderLocation(material_shader, "ambient"), (float[4]) {0.1, 0.1, 0.1, 1.0}, SHADER_UNIFORM_VEC4);

    // Setup shaders as materials
    water_model.materials[0].shader = water_shader;
    ship.model.materials[0].shader = material_shader;

    // Create lights which will work with the material shader
    Light lights[MAX_LIGHTS] = 
    {
        CreateLight(LIGHT_POINT, (Vector3){ -10, 5, -10 }, Vector3Zero(),  YELLOW, material_shader),
        CreateLight(LIGHT_POINT, (Vector3){ 10, 5, 10 },   Vector3Zero(),  RED,    material_shader),
        CreateLight(LIGHT_POINT, (Vector3){ -10, 5, 10 },  Vector3Zero(),  GREEN,  material_shader),
        CreateLight(LIGHT_POINT, (Vector3){ 10, 5, -10 },  Vector3Zero(),  WHITE,  material_shader),    
    };

    // Create waves which will work with the water shader
    WaterWave waves[MAX_WATER_WAVES] = 
    {
        CreateWave((Vector2) {0, 0}, 0.4, 1.9, 0.4,  (Vector2){1, 0},     water_shader),
        CreateWave((Vector2) {0, 0}, 0.2, 1.2, 0.8,  (Vector2){1, 0.3},   water_shader),
        CreateWave((Vector2) {0, 0}, 0.3, 1.23, 1.2, (Vector2){1, -0.5},  water_shader),
        CreateWave((Vector2) {0, 0}, 0.5, 0.8, 0.2,  (Vector2){0.2, 0.8}, water_shader),
        CreateWave((Vector2) {0, 0}, 0.08, 3.2, 1.7, (Vector2){0.1, 0.9}, water_shader),
        CreateWave((Vector2) {0, 0}, 1.0, 0.2, 0.1,  (Vector2){0.7, 0.1}, water_shader),
    };

    // Create camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 90.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            

    DisableCursor();
    SetTargetFPS(60);

    // Main loop
    while(!WindowShouldClose())
    {
        // Get delta time
        float frame_time = GetFrameTime();

        // Update time
        time += frame_time;

        // Update ship position and rotation
        {
            #define SHIP_FLOATING_SMOOTHING_FACTOR 5.0f
            #define SHIP_FLOATING_SINKING_FACTOR -0.2f
            
            Vector3 ship_pos = GroupWaveFunction(CLITERAL(Vector3) {0, SHIP_FLOATING_SINKING_FACTOR, 0}, time, waves, MAX_WATER_WAVES);
            Vector3 ship_pos_front = GroupWaveFunction(CLITERAL(Vector3) {0.2, SHIP_FLOATING_SINKING_FACTOR, 0}, time, waves, MAX_WATER_WAVES);
            Vector3 ship_pos_back = GroupWaveFunction(CLITERAL(Vector3) {-1, SHIP_FLOATING_SINKING_FACTOR, 0}, time, waves, MAX_WATER_WAVES);
            Vector3 ship_pos_left = GroupWaveFunction(CLITERAL(Vector3) {0, SHIP_FLOATING_SINKING_FACTOR, 0.5}, time, waves, MAX_WATER_WAVES);
            Vector3 ship_pos_right = GroupWaveFunction(CLITERAL(Vector3) {0, SHIP_FLOATING_SINKING_FACTOR, -0.5}, time, waves, MAX_WATER_WAVES);

            Vector2 front_diff = CLITERAL(Vector2) { ship_pos_front.x - ship_pos_back.x, ship_pos_front.y - ship_pos_back.y };
            Vector2 side_diff = CLITERAL(Vector2) { ship_pos_left.z - ship_pos_right.z, ship_pos_left.y - ship_pos_right.y };

            float front_angle = atan2(front_diff.y, front_diff.x);
            float side_angle = atan2(side_diff.y, side_diff.x);

            ship.front_angle += (front_angle - ship.front_angle) / SHIP_FLOATING_SMOOTHING_FACTOR;
            ship.side_angle += (side_angle - ship.side_angle)    / SHIP_FLOATING_SMOOTHING_FACTOR;
            ship.position = Vector3Add(ship.position, Vector3Divide(Vector3Subtract(ship_pos, ship.position), (Vector3) {SHIP_FLOATING_SMOOTHING_FACTOR, SHIP_FLOATING_SMOOTHING_FACTOR, SHIP_FLOATING_SMOOTHING_FACTOR}));
        }

        // Update camera
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        // Update shader values
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(material_shader, material_shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(water_shader,    water_shader.locs[SHADER_LOC_VECTOR_VIEW],    cameraPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(water_shader,    water_shader_time_loc,                        &time,     SHADER_UNIFORM_FLOAT);
        for (int i = 0; i < MAX_LIGHTS; i++)
            UpdateLightValues(material_shader, lights[i]);
        for (int i = 0; i < MAX_WATER_WAVES; i++) 
            UpdateWaterWaveValues(water_shader, waves[i]);
        
        BeginDrawing();

            ClearBackground(BACKGROUND_COLOR);

            BeginMode3D(camera);

                // Draw ship
                rlPushMatrix();
                    rlRotatef((ship.front_angle * SHIP_ROTATE_FACTOR), 0, 0, 1);
                    rlRotatef((ship.side_angle  * SHIP_ROTATE_FACTOR), 1, 0, 0);
                    DrawModel(ship.model, ship.position, 0.1, GRAY);
                rlPopMatrix();

                // Draw water
                DrawModel(water_model, CLITERAL(Vector3) {0, 0, 0}, 1, CLITERAL(Color) { 102, 191, 255, 255 });
                
                // Draw lights
                for(int i = 0; i < MAX_LIGHTS; i++)
                {
                    if(lights[i].enabled)
                        DrawSphere(lights[i].position, 1, lights[i].color);
                }

                // Draw main axes
                DrawLine3D(CLITERAL(Vector3) {0, 0, 0}, CLITERAL(Vector3) {1, 0, 0}, RED);
                DrawLine3D(CLITERAL(Vector3) {0, 0, 0}, CLITERAL(Vector3) {0, 1, 0}, GREEN);
                DrawLine3D(CLITERAL(Vector3) {0, 0, 0}, CLITERAL(Vector3) {0, 0, 1}, BLUE);
        
            EndMode3D();

        EndDrawing();
    }

    return 0;
}