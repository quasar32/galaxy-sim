#include <stdlib.h>
#include <math.h>
#include "sim.h"
#include "misc.h"
#include <cglm/struct.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>

struct sim sim;

static int next_octant;
static int max_octant;
static struct octant *octants;

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

struct octant {
    int children[8];
    float mass;
    vec3 com;
}; 

static int octant_index(vec3 min, vec3 max, vec3 pos) {
    int index = 0;
    for (int axis = 0; axis < 3; axis++) {
        float mid = (min[axis] + max[axis]) / 2.0f;
        if (pos[axis] >= mid) 
            index += 1 << axis;
    } 
    return index;
}

static void update_com(struct octant *oct, float m, vec3 x) {
    float total_m = oct->mass + m;
    oct->com[0] = (oct->mass * oct->com[0] + m * x[0]) / total_m;
    oct->com[1] = (oct->mass * oct->com[1] + m * x[1]) / total_m;
    oct->com[2] = (oct->mass * oct->com[2] + m * x[2]) / total_m;
    oct->mass = total_m;
}

static void descend_minmax(vec3 min, vec3 max, int index) {
    for (int axis = 0; axis < 3; axis++) {
        float mid = (min[axis] + max[axis]) / 2.0f;
        if (index & (1 << axis)) 
            min[axis] = mid;
        else
            max[axis] = mid;
    }
}

static int is_leaf(struct octant *oct) {
    for (int i = 0; i < 8; i++) {
        if (oct->children[i] >= 0)
            return 0;
    }
    return 1;
}

static int create_octant(float mass, vec3 com) {
    if (next_octant == max_octant)
        die("too many octants\n");
    struct octant *oct = &octants[next_octant];
    oct->mass = mass;
    glm_vec3_copy(com, oct->com);
    for (int i = 0; i < 8; i++)
        oct->children[i] = -1;
    return next_octant++;
}

static void octant_insert(struct octant *oct, int star, vec3 min, vec3 max) {
    if (sim.m[star] == 0.0f)
        return;
    if (oct->mass == 0.0f) {
        oct->mass = sim.m[star];
        glm_vec3_copy(sim.x[star], oct->com); 
        return;
    }
    int idx = octant_index(min, max, sim.x[star]);
    while (oct->children[idx] >= 0) {
        update_com(oct, sim.m[star], sim.x[star]); 
        descend_minmax(min, max, idx);
        oct = &octants[oct->children[idx]];
        idx = octant_index(min, max, sim.x[star]);
    }
    if (is_leaf(oct)) {
        float pm, cm;
        vec3 px, cx;
        pm = oct->mass;
        glm_vec3_copy(oct->com, px);
        float d2 = glm_vec3_distance2(px, sim.x[star]);
        if (d2 < FLT_EPSILON)
            return;
        update_com(oct, sim.m[star], sim.x[star]);
        cm = oct->mass;
        glm_vec3_copy(oct->com, cx);
        int pidx = octant_index(min, max, px);
        while (idx == pidx) {
            oct->children[idx] = create_octant(cm, cx);
            oct = &octants[oct->children[idx]];
            descend_minmax(min, max, idx);
            idx = octant_index(min, max, sim.x[star]);
            pidx = octant_index(min, max, px);
        }
        oct->children[pidx] = create_octant(pm, px);
    }
    oct->children[idx] = create_octant(sim.m[star], sim.x[star]);
}

static void add_acc(struct octant *o, int star) {
    vec3 r;
    glm_vec3_sub(o->com, sim.x[star], r);
    float r2 = glm_vec3_norm2(r);
    float s = o->mass * powf(r2 + sim.e2, -1.5f);
    glm_vec3_muladds(r, s, sim.a[star]);
}

static void calc_acc(struct octant *o, int s, float d) {
    if (is_leaf(o)) {
        add_acc(o, s);
    } else {
        float r2 = glm_vec3_distance2(sim.x[s], o->com);
        if (d * d < r2) {
            add_acc(o, s);
        } else {
            for (int i = 0; i < 8; i++) {
                if (o->children[i] >= 0) {
                    calc_acc(&octants[o->children[i]], s, d / 2.0f);
                }
            }
        }
    }
}

struct step {
    float sim_size;
    struct octant *root;
    int star0;
    int star1;
};

static void *update_acc(void *arg) {
    struct step *step = arg;
    struct octant *root = step->root;
    float sim_size = step->sim_size;
    int i = step->star0;
    int n = step->star1;
    for (; i < n; i++) {
        glm_vec3_zero(sim.a[i]);
        calc_acc(root, i, sim_size);
    }
    return NULL;
}

void step_sim(void) {
    float sim_min = INFINITY;
    float sim_max = -INFINITY;
    for (int i = 0; i < sim.n; i++) {
        for (int j = 0; j < 3; j++) {
            sim_min = fminf(sim_min, sim.x[i][j] - sim.r[i]);
            sim_max = fmaxf(sim_max, sim.x[i][j] + sim.r[i]);
        }
    }
    next_octant = 0;
    max_octant = sim.n * 2;
    octants = xmalloc(max_octant * sizeof(*octants));
    int root_idx = create_octant(0.0f, (vec3) {0.0f, 0.0f, 0.0f});
    struct octant *root = &octants[root_idx];
    for (int i = 0; i < sim.n; i++) {
        vec3 min = {sim_min, sim_min, sim_min};
        vec3 max = {sim_max, sim_max, sim_max};
        octant_insert(root, i, min, max);
    }
    int n_thrds = 12;
    pthread_t thrds[n_thrds];
    struct step steps[n_thrds];
    for (int i = 0; i < n_thrds; i++) {
        steps[i].sim_size = sim_max - sim_min;
        steps[i].star0 = sim.n * i / n_thrds;
        steps[i].star1 = sim.n * (i + 1) / n_thrds;
        steps[i].root = root;
        int err = pthread_create(&thrds[i], NULL, update_acc, &steps[i]);
        if (err)
            die("pthread_create: %s: %d\n", strerror(err), err);
    }
    for (int i = 0; i < n_thrds; i++) {
        pthread_join(thrds[i], NULL);
    }
    free(octants);
    next_octant = 0;
    max_octant = 0;
    octants = NULL;
    for (int i = 0; i < sim.n; i++) {
        glm_vec3_muladds(sim.a[i], sim.h, sim.v[i]);
        glm_vec3_muladds(sim.v[i], sim.h, sim.x[i]);
    }
}
