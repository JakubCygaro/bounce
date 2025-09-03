#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "ball.h"
#include <raymath.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define SPEED 2.5f
#define SRC_WIDTH 600
#define SRC_HEIGHT 600
#define TARGET_FPS 60
#define MAX_BALLS 512
#define MIN_RADIUS 5.0
#define MAX_RADIUS 100.0
#define GRAVITY 8.0
#define SCROLL_SPEED 1.5
#define COEFFICIENT_OF_RESTITUTION 0.80

static Ball balls[MAX_BALLS] = { 0 };
static size_t ball_count = 0;
static double delta_t = 0;
static double current_frame_time = 0;
static double last_frame_time = 0;
static float ball_radius = 5.0;
static Sound bounce_sound;

bool collision(Ball* a, Ball* b){
    double dist = sqrt(pow(a->position.x - b->position.x, 2.0) + pow(a->position.y - b->position.y, 2.0));
    return dist < (a->radius + b->radius);
}

Vector2 calculate_velocity_after_collision(Ball* one, Ball* two){
    float mass_comp = (2.0 * two->mass) / (one->mass + two->mass);

    Vector2 direction = Vector2Subtract(one->position, two->position);
    float dir_dot = Vector2Distance(one->position, two->position);
    float dir_dot_squared = pow(dir_dot, 2.0);

    float dot = Vector2DotProduct(Vector2Subtract(one->velocity, two->velocity), Vector2Subtract(one->position, two->position));

    float dot_quoitent = dot / dir_dot_squared;

    float mass_x_quo = mass_comp * dot_quoitent;


    Vector2 right_comp = {
        .x = mass_x_quo * direction.x,
        .y = mass_x_quo * direction.y,
    };
    return Vector2Subtract(one->velocity, right_comp);
}

void input(){
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        Vector2 mouse = GetMousePosition();
        Color c = {
            .r = rand() % UCHAR_MAX + 10,
            .g = rand() % UCHAR_MAX + 10,
            .b = rand() % UCHAR_MAX + 10,
            .a = UCHAR_MAX
        };
        balls[ball_count++] = ball_new(ball_radius, c, mouse);
    }
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        Vector2 mouse = GetMousePosition();
        for(size_t i = 0; i < ball_count; i++){
            Ball* ball = &balls[i];
            if(powf(mouse.x - ball->position.x, 2.0) + powf(mouse.y - ball->position.y, 2.0) <= powf(ball->radius, 2.0)){
                balls[i] = balls[--ball_count];
                break;
            }
        }
    }
    float scroll = GetMouseWheelMove() * SCROLL_SPEED;
    ball_radius = Clamp(ball_radius + scroll, MIN_RADIUS, MAX_RADIUS);

    if(IsKeyReleased(KEY_R)){
        ball_count = 0;
    }
}

bool check_edge(Ball* ball){
    if(ball->position.x + ball->radius >= SRC_WIDTH){
        ball->velocity.x = -ball->velocity.x * COEFFICIENT_OF_RESTITUTION;
        ball->position.x = SRC_WIDTH - ball->radius;
        return true;
    } else if(ball->position.x - ball->radius <= 0){
        ball->velocity.x = -ball->velocity.x * COEFFICIENT_OF_RESTITUTION;
        ball->position.x = 0 + ball->radius;
        return true;
    }
    if(ball->position.y + ball->radius >= SRC_HEIGHT){
        ball->velocity.y = -ball->velocity.y * COEFFICIENT_OF_RESTITUTION;
        ball->position.y = SRC_HEIGHT - ball->radius;
        return true;
    } else if(ball->position.y - ball->radius <= 0){
        ball->velocity.y = -ball->velocity.y * COEFFICIENT_OF_RESTITUTION;
        ball->position.y = 0 + ball->radius;
        return true;
    }
    return false;
}

void update() {
    input();
    for(size_t i = 0; i < ball_count; i++){
        Ball* a = &balls[i];
        // collision with other balls
        for(size_t j = i + 1; j < ball_count; j++){
            Ball* b = &balls[j];
            if(collision(a, b)){
                Vector2 direction = Vector2Subtract(a->position, b->position);
                float len = Vector2Length(direction);
                Vector2 normal = Vector2Normalize(direction);

                a->acceleration = Vector2Add(a->acceleration, (Vector2){
                        .x = normal.x * ( len / a->radius ),
                        .y = normal.y * ( len / a->radius ),
                        });
                b->acceleration = Vector2Add(b->acceleration, (Vector2){
                        .x = -normal.x * ( len / b->radius ),
                        .y = -normal.y * ( len / b->radius ),
                        });
                // a->acceleration = Vector2Add(a->acceleration, (Vector2){
                //         .x = normal.x * delta_t,
                //         .y = normal.y * delta_t,
                //         });
                // b->acceleration = Vector2Add(b->acceleration, (Vector2){
                //         .x = -normal.x * delta_t,
                //         .y = -normal.y * delta_t,
                //         });

                //https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional
                a->velocity = calculate_velocity_after_collision(a, b);
                b->velocity = calculate_velocity_after_collision(b, a);

                PlaySound(bounce_sound);
            }
        }
        if(check_edge(a)){
            // float m = Vector2Length(a->velocity);
            // float dot = Vector2DotProduct(a->velocity, (Vector2){ 1.0, 0.0 });
            // dot = dot < 0 ? -dot : dot;
            // if(m > 0.5 && dot < 0.7)
            //     PlaySound(bounce_sound);
        }
    }
    for(size_t i = 0; i < ball_count; i++){
        Ball* ball = &balls[i];
        ball->acceleration = (Vector2){
            .x = ball->acceleration.x,
            .y = ball->acceleration.y + GRAVITY * delta_t
        };
        ball->velocity = Vector2Add(ball->velocity, ball->acceleration);
        ball->acceleration = (Vector2){ 0, 0 };
        ball->position = Vector2Add(ball->position, ball->velocity);
    }
}

void draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    for(size_t i = 0; i < ball_count; i++){
        Ball* ball = &balls[i];
        DrawCircle(ball->position.x, ball->position.y, ball->radius, ball->color);
    }
    char str[30] = { 0 };
    sprintf(str, "Radius: %.1f", ball_radius);
    DrawText(str, 0, 0, 18, WHITE);
    EndDrawing();
}

void update_draw_frame(){
    current_frame_time = GetTime();
    update();
    draw();
    delta_t = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;
}

int main(void) {
    InitWindow(SRC_WIDTH, SRC_HEIGHT, "Bounce");
    InitAudioDevice();
    SetTargetFPS(TARGET_FPS);
    bounce_sound = LoadSound("resources/qubodup-cfork-ccby3-jump.ogg");
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    while(!WindowShouldClose()){
        update_draw_frame();
    }
#endif
    UnloadSound(bounce_sound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
