#include "ball.h"


Ball ball_new(float radius, Color color, Vector2 pos) {
    return (Ball){
        .radius = radius,
        .mass = 2.0 * radius,
        .color = color,
        .position = pos,
        .velocity = { 0, 0 },
    };
}

