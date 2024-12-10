#include <stdio.h>
#define __USE_GNU
#include <stdlib.h>
#include <math.h>
#include <glad/gl.h>
#include "draw.h"
#include "sim.h"
#include "misc.h"

SDL_Window *wnd;

static GLuint vao;
static GLuint vbo;
static GLuint tex;
static GLuint prog;
static GLint proj_ul;

struct vertex {
    vec3 xyz;
    vec2 uv;
    vec3 rgb;
};

static struct vertex rect[6] = {
    {{0.5f, 0.5f}, {1.0f, 1.0f}}, 
    {{-0.5f, 0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {1.0f, 1.0f}}, 
    {{-0.5f, -0.5f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f}, {1.0f, 0.0f}}
};

static GLuint gen_shader(GLenum type, const char *path) {
    GLuint shader = glCreateShader(type);
    FILE *fp = fopen(path, "r");
    if (!fp) 
        die("could not open %s\n", path);
    char data[1024];
    size_t sz = fread(data, 1, sizeof(data), fp);
    if (sz == sizeof(data))
        die("%s too big\n", path);
    if (ferror(fp))
        die("error reading %s\n");
    fclose(fp);
    data[sz] = '\0';
    const char *src = data;
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(data), NULL, data);
        die("%s: %s\n", path, data);
    }
    return shader;
}

static void sdl2_die(const char *func) {
    printf("%s(): %s\n", func, SDL_GetError());
    exit(EXIT_FAILURE);
}

static void std_die(const char *func) {
    perror(func);
    exit(EXIT_FAILURE);
}

static void init_sdl(int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        sdl2_die("SDL_Init");
    if (atexit(SDL_Quit))
        std_die("atexit");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                    SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    wnd = SDL_CreateWindow("Many Objects",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_OPENGL);
    if (!wnd)
        sdl2_die("SDL_CreateWindow");
    if (!SDL_GL_CreateContext(wnd))
        sdl2_die("SDL_GL_CreateContext");
    if (gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress) == 0)
        sdl2_die("SDL_GL_GetProcAddress");
}

static void init_prog(void) {
    prog = glCreateProgram();
    GLuint vs = gen_shader(GL_VERTEX_SHADER, "res/vert.glsl");
    glAttachShader(prog, vs);
    GLuint fs = gen_shader(GL_FRAGMENT_SHADER, "res/frag.glsl");
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    int success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        die("program: %s\n", log);
    }
    glDetachShader(prog, vs);
    glDeleteShader(vs);
    glDetachShader(prog, fs);
    glDeleteShader(fs);
    proj_ul = glGetUniformLocation(prog, "proj");
}

static void init_bufs(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char data[32][32];
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
            float fx = (x - 16) / 16.0f;
            float fy = (y - 16) / 16.0f;
            float fv = fmaxf(1.0f - sqrtf(fx * fx + fy * fy), 0.0f);
            data[x][y] = fv * 255;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 32, 32, 0, 
                 GL_RED, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void init_draw(int w, int h) {
    init_sdl(w, h);
    init_prog();
    init_bufs();
}

static int pos_cmp(const void *ip, const void *jp, void *arg) {
    int i = *(int *) ip;
    int j = *(int *) jp;
    vec4 *v = arg;
    float a = v[i][2];
    float b = v[j][2];
    return (a > b) - (a < b);
}

void draw(int w, int h, vec3 eye, vec3 front) {
    mat4 view;
    vec3 center;
    glm_vec3_add(eye, front, center);
    glm_lookat(eye, center, GLM_YUP, view);
    int n = 6 * sim.n;
    vec4 *poss = xmalloc(n * sizeof(*poss));
    for (int i = 0, k = 0; i < sim.n; i++) {
        mat4 mv;
        glm_translate_to(view, sim.x[i], mv);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                mv[i][j] = (i == j);
        }
        glm_scale_uni(mv, sim.r[i]);
        for (int j = 0; j < 6; j++, k++) {
            glm_vec4(rect[j].xyz, 1.0f, poss[k]);
            glm_mat4_mulv(mv, poss[k], poss[k]);
        }
    }
    struct vertex *vertices = xmalloc(n * sizeof(*vertices));
    int *indices = xmalloc(n * sizeof(*indices));
    for (int i = 0; i < n; i++)
        indices[i] = i;
    qsort_r(indices, n, sizeof(*indices), pos_cmp, poss);
    for (int i = 0, k = 0; i < sim.n; i++) {
        for (int j = 0; j < 6; j++, k++) {
            int iv = indices[k];
            glm_vec3_copy(poss[iv], vertices[k].xyz);
            glm_vec2_copy(rect[j].uv, vertices[k].uv);
            glm_vec3_copy(sim.c[iv / 6], vertices[k].rgb);
        }
    }
    free(indices);
    indices = NULL;
    free(poss);
    poss = NULL;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    mat4 proj;
    glm_perspective_default(w / (float) h, proj);
    glUniformMatrix4fv(proj_ul, 1, GL_FALSE, (float *) proj);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(*vertices), 
                 vertices, GL_DYNAMIC_DRAW);
    free(vertices);
    vertices = NULL;
#define VERTEX_ATTRIB(index, size, type, member) \
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, \
                          sizeof(type), &((type *) 0)->member)
    VERTEX_ATTRIB(0, 3, struct vertex, xyz);
    VERTEX_ATTRIB(1, 2, struct vertex, uv);
    VERTEX_ATTRIB(2, 3, struct vertex, rgb);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLES, 0, n);
}
