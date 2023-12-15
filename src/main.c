#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define PRINT_SDL_ERROR printf("SDL Error: %s\n", SDL_GetError())
#define WINDOW_HEIGHT 600
#define WINDOW_WEIGHT 800
#define FPS 60

typedef struct{
    int x_pos;
    int y_pos;
    int weight, height;
}Bricks;

typedef struct{
    float cx;
    float cy;
    uint8_t radius;
    float dx;
    float dy;
}Ball;


/*TODO: FPS, draw better circle*/

void DrawCircle( SDL_Renderer *renderer, Ball ball) {
    const int arrSize = ((int)(ball.radius * 8 * 1 / sqrt(2)) + (8 - 1)) & -8;
    SDL_FPoint points[arrSize];
    int       drawCount = 0;

    float diameter = (ball.radius * 2);
    float x = (ball.radius - 1);
    float y = 0;
    float tx = 1;
    float ty = 1;
    float error = (tx - diameter);

    while( x >= y ){
        points[drawCount+0].x =  ball.cx + x;
        points[drawCount+1].x =  ball.cx + x;
        points[drawCount+2].x =  ball.cx - x;
        points[drawCount+3].x =  ball.cx - x;
        points[drawCount+4].x =  ball.cx + y;
        points[drawCount+5].x =  ball.cx + y;
        points[drawCount+6].x =  ball.cx - y;
        points[drawCount+7].x =  ball.cx - y;

        points[drawCount+0].y = ball.cy - y;
        points[drawCount+1].y = ball.cy + y;
        points[drawCount+2].y = ball.cy - y;
        points[drawCount+3].y = ball.cy + y;
        points[drawCount+4].y = ball.cy - x;
        points[drawCount+5].y = ball.cy + x;
        points[drawCount+6].y = ball.cy - x;
        points[drawCount+7].y = ball.cy + x;


        drawCount += 8;

        if( error <= 0 )
        {
            y += 1;
            error += ty;
            ty += 2;
        }

        if( error > 0 )
        {
            x -= 1;
            tx += 2;
            error += (tx - diameter);
        }
    }

    SDL_RenderDrawLinesF(renderer, points, drawCount);
}

void rend(SDL_Renderer *renderer, Ball *ball, Bricks *bar, Bricks *blocks, int size){
    SDL_SetRenderDrawColor(renderer, 43, 53, 59, 255);
    
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 210, 74, 66, 255);
    DrawCircle(renderer, *ball);

    SDL_SetRenderDrawColor(renderer, 102, 129, 170, 255);
    SDL_Rect bar_rect = {bar->x_pos, bar->y_pos, bar->weight, bar->height};
    SDL_RenderFillRect(renderer, &bar_rect);

    for(int i = 0; i < size; i++){
        SDL_Rect block = {blocks[i].x_pos, blocks[i].y_pos, blocks[i].weight, blocks[i].height};
        SDL_RenderFillRect(renderer, &block);
    }

    SDL_RenderPresent(renderer);
}

void inputCntrl(SDL_Window *window, uint8_t *end, Bricks *bar){
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_QUIT:
                if(window){
                    SDL_DestroyWindow(window);
                    window = NULL;
                    *end = 0;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        *end = 0;
                        break;
                }
        }
    }
    const uint8_t *keyboard = SDL_GetKeyboardState(NULL);
    if((keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT]) && bar->x_pos > 0) bar->x_pos -= 4;
    else if((keyboard[SDL_SCANCODE_D] || keyboard[SDL_SCANCODE_RIGHT]) && bar->x_pos + bar->weight < WINDOW_WEIGHT) bar->x_pos += 4;
}

uint8_t collision(SDL_Rect rectball, SDL_Rect rectbar, Bricks *blocks, int *size){
    if(SDL_HasIntersection(&rectbar, &rectball)) return 1;
    
    for(int i = 0; i < *size; i++){
        rectbar.x = blocks[i].x_pos;
        rectbar.y = blocks[i].y_pos;
        rectbar.w = blocks[i].weight;
        rectbar.h = blocks[i].height;
        if(SDL_HasIntersection(&rectball, &rectbar)){
            (*size)--;
            blocks[i] = blocks[(*size)];
            return 1;
        }
    }
    return 0;
}

uint8_t update(Ball *ball, Bricks bar, Bricks *blocks, int *size){
    uint8_t flag = 1;
    float nx = ball->cx + ball->dx;
    SDL_Rect rectball = {(int)(nx - ball->radius), (int)(ball->cy - ball->radius), (int)ball->radius*2, (int)ball->radius*2};
    SDL_Rect rectbar = {bar.x_pos, bar.y_pos, bar.weight, bar.height};
    
    if(ball->cx > WINDOW_WEIGHT - ball->radius || ball->cx - ball->radius  < 0 || collision(rectball, rectbar, blocks, size)){
        ball->dx *= -1;
        nx = ball->cx + ball->dx;
        flag = 2;
    }

    rectball.x = ball->cx - ball->radius;
    float ny = ball->cy + ball->dy;
    rectball.y = ny - ball->radius;

    if(ball->cy - ball->radius < 0 || collision(rectball, rectbar, blocks, size)) {
        ball->dy *= -1;
        ny = ball->cy + ball->dy;
        flag = 2;
    }
    if(ball->cy - ball->radius > bar.y_pos + bar.height) flag = 0;

    ball->cx = nx;
    ball->cy = ny;
    return flag;
}

void init_blocks(Bricks *blocks, int bricks_size){
    int tempx = 0;
    int tempy = 30;

    for(int i = 0; i < bricks_size; i++){
        blocks[i].x_pos = 4 + tempx;
        blocks[i].y_pos = tempy;
        blocks[i].weight = (WINDOW_WEIGHT / 8) - 8;
        blocks[i].height = 25;
        tempx += (WINDOW_WEIGHT / 8);
        if(i % 8 == 7){
            tempx = 0;
            tempy += 30;
        }
    }


}

int main(){
    SDL_Window *window;
    int retval = 0;
    SDL_Renderer *renderer;

    Ball ball = {((float)WINDOW_WEIGHT / 2) - 13, ((float)WINDOW_HEIGHT / 2) - 13, 13, 2.5, 2.5};
    Bricks bar = {(WINDOW_WEIGHT / 2) - 50, WINDOW_HEIGHT - 25, 100, 15};

    int bricks_size = 8 * 6;
    Bricks blocks[bricks_size];
    init_blocks(blocks, bricks_size);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        PRINT_SDL_ERROR;
        return -1;
    }

    window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WINDOW_WEIGHT, WINDOW_HEIGHT, 0);
    if(window == NULL){
        PRINT_SDL_ERROR;
        retval = -1;
        goto WIN_ERR;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL){
        PRINT_SDL_ERROR;
        retval = -1;
        goto REN_ERR;
    }

    uint8_t end = 1;

    while (end && bricks_size){
        end = update(&ball, bar, blocks, &bricks_size);
        inputCntrl(window, &end, &bar);
        rend(renderer, &ball, &bar, blocks, bricks_size);
    }
    
    SDL_DestroyRenderer(renderer);
REN_ERR:    
    SDL_DestroyWindow(window);
WIN_ERR:
    SDL_Quit();

    return retval;
}
