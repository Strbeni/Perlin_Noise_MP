#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 16;
const int GRID_WIDTH = SCREEN_WIDTH / GRID_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / GRID_SIZE;

struct Vec2 {
    float x, y;
};

float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float gradient(int hash, float x, float y) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float perlin(float x, float y, const vector<int>& p) {
    int X = floor(x);
    int Y = floor(y);
    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = p[X] + Y & 255;
    int B = p[X + 1] + Y & 255;

    return lerp(v, lerp(u, gradient(p[A], x, y), gradient(p[B], x - 1, y)),
                lerp(u, gradient(p[A + 1], x, y - 1), gradient(p[B + 1], x - 1, y - 1)));
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Perlin Noise", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    vector<int> p(512);
    for (int i = 0; i < 256; i++) p[i] = i;
    random_device rd;
    mt19937 g(rd());
    shuffle(p.begin(), p.end(), g);
    for (int i = 0; i < 256; i++) p[i + 256] = p[i];

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }

        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                float val = perlin((float)x / GRID_WIDTH, (float)y / GRID_HEIGHT, p);
                Uint8 color = (Uint8)(val * 255);
                SDL_SetRenderDrawColor(renderer, color, color, color, 255);
                SDL_Rect rect = {x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}