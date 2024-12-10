#pragma once

#include <cglm/cglm.h>

struct sim {
    int n;
    float h;
    float e2;
    float *restrict m;
    vec3 *restrict x;
    vec3 *restrict v;
    vec3 *restrict a;
    vec3 *restrict c;
    float *restrict r;
};

extern struct sim sim;

void init_sim(void);
void step_sim(void);
