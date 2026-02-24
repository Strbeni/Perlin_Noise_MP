#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <ctime>
#include <numeric>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
const int GRID_SCALE = 10; 
const int COLS = SCREEN_WIDTH / GRID_SCALE;
const int ROWS = SCREEN_HEIGHT / GRID_SCALE;
const int NUM_PARTICLES = 3000;
const float NOISE_SCALE = 0.05f; 
const float TIME_STEP = 0.002f; 
const float FORCE_MAGNITUDE = 0.5f;

using namespace std;

// Math helper
struct Vec2 {
    float x, y;
};

// --- Perlin Noise Class ---
class PerlinNoise {
    vector<int> p;

    float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    float grad(int hash, float x, float y, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

public:
    PerlinNoise() {
        p.resize(512);
        iota(p.begin(), p.begin() + 256, 0); // Fill 0-255
        random_device rd;
        mt19937 g(rd());
        shuffle(p.begin(), p.begin() + 256, g);
        for (int i = 0; i < 256; i++) p[i + 256] = p[i];
    }

    float noise(float x, float y, float z) {
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(w,
            lerp(v,
                lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))
            ),
            lerp(v,
                lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))
            )
        );
    }
};

// --- Particle Class ---
class Particle {
public:
    float x, y;
    float vx, vy;
    float ax, ay;
    float prevX, prevY;
    float maxSpeed;

    Particle() {
        x = (float)(rand() % SCREEN_WIDTH);
        y = (float)(rand() % SCREEN_HEIGHT);
        prevX = x;
        prevY = y;
        vx = 0; vy = 0;
        ax = 0; ay = 0;
        maxSpeed = 2.0f + ((float)(rand() % 100) / 50.0f);
    }

    void update() {
        vx += ax;
        vy += ay;
        
        float speed = sqrt(vx*vx + vy*vy);
        if (speed > maxSpeed) {
            vx = (vx / speed) * maxSpeed;
            vy = (vy / speed) * maxSpeed;
        }

        prevX = x;
        prevY = y;
        x += vx;
        y += vy;
        
        ax = 0;
        ay = 0;

        edges();
    }

    void applyForce(Vec2 force) {
        ax += force.x;
        ay += force.y;
    }

    void follow(const vector<Vec2>& flowField) {
        int xGrid = (int)(x / GRID_SCALE);
        int yGrid = (int)(y / GRID_SCALE);
        
        // Clamp to grid
        if (xGrid >= 0 && xGrid < COLS && yGrid >= 0 && yGrid < ROWS) {
            int index = xGrid + yGrid * COLS;
            if (index >= 0 && index < flowField.size()) {
                Vec2 force = flowField[index];
                applyForce(force);
            }
        }
    }

    void edges() {
        bool wrapped = false;
        if (x >= SCREEN_WIDTH) { x = 0; wrapped = true; }
        else if (x < 0) { x = SCREEN_WIDTH - 1; wrapped = true; }
        if (y >= SCREEN_HEIGHT) { y = 0; wrapped = true; }
        else if (y < 0) { y = SCREEN_HEIGHT - 1; wrapped = true; }
        
        if (wrapped) {
            prevX = x;
            prevY = y;
        }
    }

    void show(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 20); 
        SDL_RenderDrawLine(renderer, (int)prevX, (int)prevY, (int)x, (int)y);
    }
};

int main(int argc, char* argv[]) {
    srand((unsigned int)time(0));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL Init Failed: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Perlin Noise Flow Field", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
    if (!window) return 1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) {
        cerr << "Renderer Failed (Need Target Texture support): " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Texture* trailTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    SDL_SetTextureBlendMode(trailTexture, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, trailTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);

    PerlinNoise pn;
    vector<Particle> particles(NUM_PARTICLES);
    vector<Vec2> flowField(COLS * ROWS);
    
    float zOff = 0.0f;
    
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        float yOff = 0.0f;
        for (int y = 0; y < ROWS; y++) {
            float xOff = 0.0f;
            for (int x = 0; x < COLS; x++) {
                float n = pn.noise(xOff, yOff, zOff);
                
                float angle = n * 2.0f * 3.14159f * 4.0f;
                Vec2 v;
                v.x = cos(angle) * FORCE_MAGNITUDE;
                v.y = sin(angle) * FORCE_MAGNITUDE;
                
                int index = x + y * COLS;
                flowField[index] = v;

                xOff += NOISE_SCALE;
            }
            yOff += NOISE_SCALE;
        }
        zOff += TIME_STEP; 
        SDL_SetRenderTarget(renderer, trailTexture);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 5);
        SDL_Rect screenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &screenRect);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (auto& p : particles) {
            p.follow(flowField);
            p.update();
            p.show(renderer);
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, trailTexture, NULL, NULL);

        SDL_RenderPresent(renderer);
        
    }

    SDL_DestroyTexture(trailTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}