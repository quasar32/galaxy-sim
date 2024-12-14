#include <glad/gl.h>
#include "draw.h"
#include "sim.h"

#define FPS 50 

static vec3 eye = {0.0f, 0.0f, 5.0f};
static vec3 front;
static vec3 right;
static float yaw = -GLM_PI_2f;
static float pitch = 0.0f;
static const Uint8 *keys;
static int n_keys;

#define MAX_PITCH (GLM_PI_2f - 0.01F)
#define MIN_PITCH (-MAX_PITCH)

static void update_rots(void) {
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    yaw += x * 0.001f;
    pitch -= y * 0.001f;
    yaw = fmodf(yaw, GLM_PIf * 2.0F);
    pitch = fminf(fmaxf(pitch, MIN_PITCH), MAX_PITCH);
}

static void update_dirs(void) {
    float xp = cosf(pitch);
    front[0] = cosf(yaw) * xp;
    front[1] = sinf(pitch);
    front[2] = sinf(yaw) * xp;
    glm_vec3_cross(front, GLM_YUP, right);
}

static void update_eye(void) {
    if (keys[SDL_SCANCODE_W])
        glm_vec3_muladds(front, 0.1f, eye);
    if (keys[SDL_SCANCODE_S])
        glm_vec3_mulsubs(front, 0.1f, eye);
    if (keys[SDL_SCANCODE_A])
        glm_vec3_mulsubs(right, 0.1f, eye);
    if (keys[SDL_SCANCODE_D])
        glm_vec3_muladds(right, 0.1f, eye);
}

int main(void) {
    init_draw(640, 480);
    init_sim();
    int w = -1;
    int h = -1;
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 frame = freq / FPS;
    Uint64 t0 = SDL_GetPerformanceCounter();
    Uint64 acc = 0;
    SDL_ShowWindow(wnd);
    SDL_SetRelativeMouseMode(GL_TRUE);
    keys = SDL_GetKeyboardState(&n_keys);
    while (!SDL_QuitRequested()) {
        int w0, h0;
        SDL_GetWindowSizeInPixels(wnd, &w0, &h0);
        if (w != w0 && h != h0) {
            w = w0;
            h = h0;
            glViewport(0, 0, w, h);
        }
        draw(w, h, eye, front);
        SDL_GL_SwapWindow(wnd);
        Uint64 t1 = SDL_GetPerformanceCounter();
        acc += t1 - t0;
        t0 = t1;
        if (acc > freq / 10)
            acc = freq / 10;
        while (acc >= frame) {
            acc -= frame;
            SDL_PumpEvents();
            update_rots();
            update_dirs();
            update_eye();
            step_sim();
        }
    }
    return EXIT_SUCCESS;
}
