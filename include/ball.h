#ifndef BALL_H
#define BALL_H
#include <raylib.h>

typedef struct _Ball {
    float radius;
    float mass;
    Color color;
    Vector2 position;
    Vector2 velocity;
} Ball;

Ball ball_new(float radius, Color color, Vector2 pos);

#endif
