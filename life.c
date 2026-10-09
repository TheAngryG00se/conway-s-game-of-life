#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#define W 300
#define H 300
#define SCALE 3

static unsigned char a[H][W], b[H][W];

static void step(unsigned char (*cur)[W], unsigned char (*next)[W])
{
    #pragma omp parallel for
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int n = 0;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    if (dx || dy)
                        n += cur[(y + dy + H) % H][(x + dx + W) % W];
            next[y][x] = n == 3 || (n == 2 && cur[y][x]);
        }
}

#ifndef RANDOM
static void load(const char *path, int x0, int y0)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); exit(1); }
    char line[1024];
    for (int y = y0; fgets(line, sizeof line, f);) {
        if (line[0] == '!') continue;
        for (int x = 0; line[x]; x++)
            if (line[x] == 'O') a[y % H][(x0 + x) % W] = 1;
        y++;
    }
    fclose(f);
}
#endif

int main(void)
{
#ifdef _OPENMP
    printf("OpenMP: %d threads\n", omp_get_max_threads());
#else
    printf("Single-threaded\n");
#endif
#ifdef RANDOM
    srand(42);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            a[y][x] = rand() % 4 == 0;
#else
    FILE *f = fopen("patterns/layout.txt", "r");
    if (!f) { perror("patterns/layout.txt"); return 1; }
    char name[256], path[300];
    int x, y;
    while (fscanf(f, "%255s %d %d", name, &x, &y) == 3) {
        snprintf(path, sizeof path, "patterns/%s", name);
        load(path, x, y);
    }
    fclose(f);
#endif

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       W * SCALE, H * SCALE, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, W, H);
    static Uint32 pix[H][W];
    unsigned char (*cur)[W] = a, (*next)[W] = b, (*tmp)[W];
    Uint64 freq = SDL_GetPerformanceFrequency(), step_ticks = 0, last = SDL_GetPerformanceCounter();
    int gens = 0;

    for (int running = 1; running;) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT) running = 0;

        Uint64 t0 = SDL_GetPerformanceCounter();
        step(cur, next);
        step_ticks += SDL_GetPerformanceCounter() - t0;
        gens++;
        tmp = cur; cur = next; next = tmp;

        if (SDL_GetPerformanceCounter() - last >= freq) {
            double ms = 1000.0 * step_ticks / freq / gens;
            char title[64];
            snprintf(title, sizeof title, "Life | step %.3f ms | %.0f gen/s", ms, 1000.0 / ms);
            SDL_SetWindowTitle(win, title);
            step_ticks = 0;
            gens = 0;
            last = SDL_GetPerformanceCounter();
        }

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                pix[y][x] = cur[y][x] ? 0xFFFFFFFF : 0xFF000000;
        SDL_UpdateTexture(tex, NULL, pix, W * sizeof(Uint32));
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
