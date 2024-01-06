#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_SDL_ERROR     printf("SDL Error: %s\n", SDL_GetError())
#define WINDOW_HEIGHT       600
#define WINDOW_WEIGHT       800
#define BACKGROUND          0xFF2B353B
#define BALL_COLOR          0xFFD24A42
#define BRICKS_COLOR        0xFF6681AA
#define FPS 100

enum {
    BLUE,
    GREEN,
    RED,
    ALPHA,
    TOTAL
};

typedef union {
    unsigned int color;
    uint8_t      rgba[TOTAL];
} Color;

typedef struct{
    float   cx;
    float   cy;
    float   dx;
    float   dy;
    uint8_t radius;
} Ball;

void in_points(SDL_Point *points, int drawCount, const Ball *ball, int x, int y, int x1, int y1);
void DrawCircle( SDL_Renderer *renderer,const Ball *ball); 
void set_color(SDL_Renderer *renderer, const unsigned int color);
void render(SDL_Renderer *renderer, Ball *ball, SDL_Rect *bar, SDL_Rect *blocks, int size);
void control_input(SDL_Window *window, uint8_t *end, SDL_Rect *bar);
uint8_t collision(const SDL_Rect *rectball, const SDL_Rect *rectbar, SDL_Rect *blocks, int *size);
uint8_t update(Ball *ball, const SDL_Rect *bar, SDL_Rect *blocks, int *size);
void init_blocks(SDL_Rect *blocks, int bricks_size);
int main(void);

/*TODO: FPS, draw better circle*/

void in_points(SDL_Point *points, int drawCount,const Ball *ball, int x, int y, int x1, int y1){
    points[drawCount + 0].x = ball->cx + x;    
    points[drawCount + 1].x = ball->cx + x1;    
    points[drawCount + 2].x = ball->cx - x;    
    points[drawCount + 3].x = ball->cx - x1;    
    points[drawCount + 4].x = ball->cx - x;    
    points[drawCount + 5].x = ball->cx - x1;    
    points[drawCount + 6].x = ball->cx + x;    
    points[drawCount + 7].x = ball->cx + x1;  

    points[drawCount + 0].y = ball->cy - y;    
    points[drawCount + 1].y = ball->cy - y1;    
    points[drawCount + 2].y = ball->cy - y;    
    points[drawCount + 3].y = ball->cy - y1;    
    points[drawCount + 4].y = ball->cy + y;    
    points[drawCount + 5].y = ball->cy + y1;    
    points[drawCount + 6].y = ball->cy + y;    
    points[drawCount + 7].y = ball->cy + y1;
}


void DrawCircle( SDL_Renderer *renderer, const Ball *ball) {
    SDL_Point points[209];
    int       drawCount = 0;

    double pi  = 3.14159265358979323846264338327950288419716939937510;
    double pih = pi / 2.0; 

    const int prec = 152; 
    float theta = 0;     

    int x  = (float)ball->radius * cos(theta);
    int y  = (float)ball->radius * sin(theta);
    int x1 = x;
    int y1 = y;

    float step = pih/(float)prec; 
    for(theta=step;  theta <= pih;  theta+=step){
        x1 = (float)ball->radius * cosf(theta) + 0.5; 
        y1 = (float)ball->radius * sinf(theta) + 0.5; 

        if( (x != x1) || (y != y1) ){
            in_points(points, drawCount, ball,x, y, x1, y1);
            drawCount += 8;
        }
        x = x1;
        y = y1;
    }
    if(x!=0){
        x=0;
        drawCount += 8; 
    }
    SDL_RenderDrawLines(renderer, points, drawCount);
}

void set_color(SDL_Renderer *renderer, const unsigned int color){
    Color c;
    c.color = color;
    SDL_SetRenderDrawColor(renderer, c.rgba[RED], c.rgba[GREEN], c.rgba[BLUE], c.rgba[ALPHA]);
}

void render(SDL_Renderer *renderer, Ball *ball, SDL_Rect *bar, SDL_Rect *blocks, int size){
    set_color(renderer, BACKGROUND);
    SDL_RenderClear(renderer);

    set_color(renderer, BALL_COLOR);
    DrawCircle(renderer, ball);

    set_color(renderer, BRICKS_COLOR);
    SDL_RenderFillRect(renderer, bar);

    for(int i = 0; i < size; i++){
        SDL_RenderFillRect(renderer, &blocks[i]);
    }

    SDL_RenderPresent(renderer);
}

void control_input(SDL_Window *window, uint8_t *end, SDL_Rect *bar){
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
                break;
            default:
                break;
        }
    }
    const uint8_t *keyboard = SDL_GetKeyboardState(NULL);
    if((keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT]) && bar->x > 0) bar->x -= 4;
    else if((keyboard[SDL_SCANCODE_D] || keyboard[SDL_SCANCODE_RIGHT]) && bar->x + bar->w < WINDOW_WEIGHT) bar->x += 4;
}

uint8_t collision(const SDL_Rect *rectball, const SDL_Rect *rectbar, SDL_Rect *blocks, int *size){
    if(SDL_HasIntersection(rectbar, rectball)) return 1;
    for(int i = 0; i < *size; i++){
        if(SDL_HasIntersection(rectball, &blocks[i])){
            (*size)--;
            blocks[i] = blocks[(*size)];
            return 1;
        }
    }
    return 0;
}

uint8_t update(Ball *ball,const  SDL_Rect *bar, SDL_Rect *blocks, int *size){
    uint8_t flag = 1;
    float nx = ball->cx + ball->dx;
    SDL_Rect rectball = {(int)(nx - ball->radius), (int)(ball->cy - ball->radius), (int)ball->radius*2, (int)ball->radius*2};
    if(ball->cx > WINDOW_WEIGHT - ball->radius || ball->cx - ball->radius  < 0 || collision(&rectball, bar, blocks, size)){
        ball->dx *= -1;
        nx = ball->cx + ball->dx;
        flag = 2;
    }

    rectball.x = (int)(ball->cx - ball->radius);
    float ny   = ball->cy + ball->dy;
    rectball.y = (int)(ny - ball->radius);

    if(ball->cy - ball->radius < 0 || collision(&rectball, bar, blocks, size)) {
        ball->dy *= -1;
        ny = ball->cy + ball->dy;
        flag = 2;
    }
    if(ball->cy - ball->radius > bar->y + bar->h) flag = 0;

    ball->cx = nx;
    ball->cy = ny;
    return flag;
}

void init_blocks(SDL_Rect *blocks, int bricks_size){
    int tempx = 0;
    int tempy = 30;

    for(int i = 0; i < bricks_size; i++){
        blocks[i].x = 4 + tempx;
        blocks[i].y = tempy;
        blocks[i].w = (WINDOW_WEIGHT / 8) - 8;
        blocks[i].h = 25;
        tempx += (WINDOW_WEIGHT / 8);
        if(i % 8 == 7){
            tempx = 0;
            tempy += 30;
        }
    }


}

int main(void){
    SDL_Window *window;
    SDL_Renderer *renderer;
    int retval = 0;

    Ball ball = {((float)WINDOW_WEIGHT / 2) - 13, ((float)WINDOW_HEIGHT / 2) - 13, 2.92, 2.91, 13};
    SDL_Rect bar = {(WINDOW_WEIGHT / 2) - 50, WINDOW_HEIGHT - 25, 100, 15};

    int bricks_size = 8 * 6;
    SDL_Rect *blocks = (SDL_Rect *)malloc(bricks_size * sizeof(SDL_Rect));
    if(blocks == NULL){
        retval = -1;
        perror("malloc");
        goto ERR;
    }

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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL){
        PRINT_SDL_ERROR;
        retval = -1;
        goto REN_ERR;
    }

    uint8_t end = 1;

    while (end && bricks_size){
        end = update(&ball, &bar, blocks, &bricks_size);
        control_input(window, &end, &bar);
        render(renderer, &ball, &bar, blocks, bricks_size);
        SDL_Delay(1000/FPS);
    }
    SDL_DestroyRenderer(renderer);
REN_ERR: SDL_DestroyWindow(window);
WIN_ERR: SDL_Quit();
         free(blocks);
ERR:
         return retval;
}
