#include <stdio.h>
#include "sim.h"

int main(int argc, char **argv) {
    init_sim();
    for (int t = 0; t < 100; t++) {
        step_sim();
    }
    for (int i = 0; i < sim.n; i++) {
        printf("(%f, %f, %f)\n", sim.x[i][0], sim.x[i][1], sim.x[i][2]);
    }
    return EXIT_SUCCESS;
}
