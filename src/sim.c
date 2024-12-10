#include <stdlib.h>
#include <math.h>
#include "sim.h"
#include "misc.h"

struct sim sim;

void init_sim(void) {
    if (scanf("%d %f %f", &sim.n, &sim.h, &sim.e2) != 3) 
        die("invalid header\n");
    size_t sz = sizeof(*sim.m) + 
                sizeof(*sim.x) + 
                sizeof(*sim.v) + 
                sizeof(*sim.a) +
                sizeof(*sim.c) +
                sizeof(*sim.r);
    void *d = xmalloc(sim.n * sz);
    sim.m = d;
    d += sim.n * sizeof(*sim.m);
    sim.x = d; 
    d += sim.n * sizeof(*sim.x);
    sim.v = d;
    d += sim.n * sizeof(*sim.v);
    sim.a = d;
    d += sim.n * sizeof(*sim.a);
    sim.c = d;
    d += sim.n * sizeof(*sim.c);
    sim.r = d;
    for (int i = 0; i < sim.n; i++) {
        int j = scanf("%f %f %f %f %f %f %f %f %f %f %f",
                      &sim.m[i], 
                      &sim.x[i][0], &sim.x[i][1], &sim.x[i][2],
                      &sim.v[i][0], &sim.v[i][1], &sim.v[i][2],
                      &sim.c[i][0], &sim.c[i][1], &sim.c[i][2], 
                      &sim.r[i]);
        if (j <= 0)
            break;
        if (j != 11)
            die("invalid line\n");
    }
    sim.e2 = fmaxf(sim.e2, FLT_EPSILON);
}

void step_sim(void) {
    for (int i = 0; i < sim.n; i++) {
        glm_vec3_zero(sim.a[i]);
        for (int j = 0; j < sim.n; j++) {
            vec3 r;
            glm_vec3_sub(sim.x[j], sim.x[i], r);
            float r2 = glm_vec3_norm2(r);
            float s = sim.m[j] * powf(r2 + sim.e2, -1.5f);
            glm_vec3_muladds(r, s, sim.a[i]);
        }
    }
    for (int i = 0; i < sim.n; i++) {
        glm_vec3_muladds(sim.a[i], sim.h, sim.v[i]);
        glm_vec3_muladds(sim.v[i], sim.h, sim.x[i]);
    }
}
