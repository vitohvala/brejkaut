#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define PRINT_SDL_ERROR     printf("SDL Error: %s\n", SDL_GetError())
#define WINDOW_HEIGHT       600
#define H_CENTER(x)         (WINDOW_HEIGHT / 2) - (x / 2)
#define WINDOW_WIDTH        800
#define W_CENTER(x)         (WINDOW_WIDTH / 2) - (x / 2)
#define BACKGROUND          0xFF2B353B
#define BALL_COLOR          0xFFD24A42
#define BRICKS_COLOR        0xFF6681AA
#define WHITE               0xFFFFFFFF
#define TEXT_W              200
#define TEXT_H              80

enum {
    BLUE,
    GREEN,
    RED,
    ALPHA,
    TOTAL
};

enum {
    PAUSE,
    CONTINUE,
    MAIN
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

typedef struct{
    TTF_Font *font;
    SDL_Surface *surf[3];
    SDL_Texture *tex[3];
    SDL_Rect pos[3];
    int selected;
}TEXT;

void in_points(SDL_Point *points, int drawCount, const Ball *ball, int x, int y, int x1, int y1);
void DrawCircle( SDL_Renderer *renderer,const Ball *ball); 
void set_color(SDL_Renderer *renderer, const unsigned int color);
void render(SDL_Renderer *renderer, Ball *ball, SDL_Rect *bar, SDL_Rect *blocks, int size);
void control_input(SDL_Window *window, uint8_t *end, SDL_Rect *bar, uint8_t *pause, uint8_t *cont,TEXT *t, Ball *ball, SDL_Rect *blocks, int *bricks_size);
uint8_t collision(const SDL_Rect *rectball, const SDL_Rect *rectbar, SDL_Rect *blocks, int *size);
uint8_t update(Ball *ball, const SDL_Rect *bar, SDL_Rect *blocks, int *size);
void init_blocks(SDL_Rect *blocks, const int *bricks_size);
void new_game(Ball *ball, SDL_Rect *bar, SDL_Rect *blocks, int *bricks_size);
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
void render_pause(SDL_Renderer *renderer, TEXT *t){
    set_color(renderer, BACKGROUND);
    SDL_RenderClear(renderer);

    set_color(renderer, WHITE);
    SDL_Rect text_box = {t->pos[t->selected].x - 20, t->pos[t->selected].y, t->pos[t->selected].w + 40, t->pos[t->selected].h + 5};
    SDL_RenderFillRect(renderer, &text_box);


    SDL_RenderCopy(renderer, t->tex[0], NULL, &t->pos[0]);
    SDL_RenderCopy(renderer, t->tex[1], NULL, &t->pos[1]);
    SDL_RenderCopy(renderer, t->tex[2], NULL, &t->pos[2]);

    SDL_RenderPresent(renderer);
}
void 
control_input(SDL_Window *window, uint8_t *end, SDL_Rect *bar, uint8_t *pause, uint8_t *cont,TEXT *t,
        Ball *ball, SDL_Rect *blocks, int *bricks_size){
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
                        (*pause) = MAIN;
                        break;
                    case SDLK_SPACE:
                        if(*pause != MAIN && *pause == CONTINUE) (*pause) = PAUSE;
                        else (*pause) = CONTINUE;
                        break;
                    case SDLK_DOWN:
                        if(*pause == 2 && t->selected < 2){
                            if(*cont > 0) t->selected++;
                            else t->selected =  2;
                        }
                        else t->selected = 0;
                        break;
                    case SDLK_UP:
                        if(*pause == 2 && t->selected > 0){
                            if(*cont > 0) t->selected--;
                            else t->selected = 0;
                        }  
                        else t->selected = 2;
                        break;
                    case SDLK_RETURN:
                        if(t->selected == 0 && *pause == MAIN){ 
                            (*pause) = CONTINUE;
                            new_game(ball, bar, blocks, bricks_size);
                        } else if(t->selected == 1 && *pause == MAIN) (*pause = CONTINUE);
                        else if(t->selected == 2 && *pause == MAIN)(*end) = 0;
                        break;
                }
                break;
            default:
                break;
        }
    }
    if(*pause == CONTINUE){
        const uint8_t *keyboard = SDL_GetKeyboardState(NULL);
        if((keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT]) && bar->x > 0)
            bar->x -= 4;
        else if((keyboard[SDL_SCANCODE_D] || keyboard[SDL_SCANCODE_RIGHT]) && bar->x + bar->w < WINDOW_WIDTH)
            bar->x += 4;
    }
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
    if(ball->cx > WINDOW_WIDTH - ball->radius || ball->cx - ball->radius  < 0 || collision(&rectball, bar, blocks, size)){
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
    if(ball->cy - ball->radius > bar->y + bar->h) flag = 3;

    ball->cx = nx;
    ball->cy = ny;
    return flag;
}

void init_blocks(SDL_Rect *blocks,const int *bricks_size){
    int tempx = 0;
    int tempy = 30;

    for(int i = 0; i < *bricks_size; i++){
        blocks[i].x = 4 + tempx;
        blocks[i].y = tempy;
        blocks[i].w = (WINDOW_WIDTH / 8) - 8;
        blocks[i].h = 25;
        tempx += (WINDOW_WIDTH / 8);
        if(i % 8 == 7){
            tempx = 0;
            tempy += 30;
        }
    }


}

void init_text(TEXT *t, SDL_Renderer *renderer){
    Color c;
    c.color = BRICKS_COLOR;
    SDL_Color color = {c.rgba[RED], c.rgba[GREEN], c.rgba[BLUE], c.rgba[ALPHA]}; 
    SDL_Color color1 = {0, 0, 0, 255}; 
    t->surf[0] = TTF_RenderText_Blended(t->font, "New Game", color);
    t->surf[1] = TTF_RenderText_Blended(t->font, "Continue", color1);
    t->surf[2] = TTF_RenderText_Blended(t->font, "Quit",     color);
 
    for(size_t i = 0; i < 3; ++i){
        t->tex[i] = SDL_CreateTextureFromSurface(renderer, t->surf[i]);
    }
    t->selected = 0;
    t->pos[0].x = W_CENTER(TEXT_W);
    t->pos[0].y = H_CENTER(TEXT_H) - TEXT_H;
    t->pos[0].w = TEXT_W;
    t->pos[0].h = TEXT_H;

    t->pos[1].x = W_CENTER(TEXT_W);
    t->pos[1].y = H_CENTER(TEXT_H);
    t->pos[1].w = TEXT_W;
    t->pos[1].h = TEXT_H;

    t->pos[2].x = W_CENTER(TEXT_W / 2);
    t->pos[2].y = H_CENTER(TEXT_H) + TEXT_H;
    t->pos[2].w = TEXT_W / 2;
    t->pos[2].h = TEXT_H;
}

void new_game(Ball *ball, SDL_Rect *bar, SDL_Rect *blocks, int *bricks_size){
    ball->cx = ((float)WINDOW_WIDTH / 2) - 13;
    ball->cy = ((float)WINDOW_HEIGHT / 2) - 13;
    ball->dx = 2.92;
    ball->dy = 2.91;
    ball->radius = 13;

    bar->x = W_CENTER(100);
    bar->y = WINDOW_HEIGHT - 25;
    bar->w = 100;
    bar->h = 15;
    
    *bricks_size = 8 * 6;
    init_blocks(blocks, bricks_size);
}

int main(void){
    SDL_Window *window;
    SDL_Renderer *renderer;
    int retval = 0;

    Ball ball;
    SDL_Rect bar;

    int bricks_size = 8 * 6;
    SDL_Rect *blocks = (SDL_Rect *)malloc(bricks_size * sizeof(SDL_Rect));
    if(blocks == NULL){
        retval = -1;
        perror("malloc");
        goto ERR;
    }

    new_game(&ball, &bar, blocks, &bricks_size);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        PRINT_SDL_ERROR;
        return -1;
    }

    window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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

    TTF_Init();
    TEXT t;
    t.font = TTF_OpenFont("fonts/HeavyDataNerdFont-Regular.ttf", 21);
    if (t.font==NULL){
        t.font = TTF_OpenFont("/usr/share/fonts/HeavyDataNerdFont-Regular.ttf", 21);
        if(t.font == NULL){
            printf("Failed to load font: %s", SDL_GetError());
            retval = -1;
            goto TTF_ERR;
        }
    }
    init_text(&t, renderer);

    uint8_t end = 1, pause = MAIN, cont = 0;

    while (end){
        if(pause == CONTINUE){
            if(cont == 0) cont = 1;
            end = update(&ball, &bar, blocks, &bricks_size);
            if(end == 3 || bricks_size == 0) {
                pause = MAIN;
                SDL_Color color = {0, 0, 0, 255}; 
                t.surf[1] = TTF_RenderText_Blended(t.font, "Continue", color);
                t.tex[1] = SDL_CreateTextureFromSurface(renderer, t.surf[1]);
                cont = 0;
                t.selected = 0;
            }
            render(renderer, &ball, &bar, blocks, bricks_size);
            SDL_Delay(10);
        }else if(pause == MAIN) {
            if(cont == 1) {
                Color c;
                c.color = BRICKS_COLOR;
                SDL_Color color = {c.rgba[RED], c.rgba[GREEN], c.rgba[BLUE], c.rgba[ALPHA]}; 
                t.surf[1] = TTF_RenderText_Blended(t.font, "Continue", color);
                t.tex[1] = SDL_CreateTextureFromSurface(renderer, t.surf[1]);
                cont = 2;
            }
            render_pause(renderer, &t);
        }
        control_input(window, &end, &bar, &pause, &cont, &t, &ball, blocks, &bricks_size);
    }
    for(size_t i = 0; i < 3; ++i){
        SDL_FreeSurface(t.surf[i]);
        SDL_DestroyTexture(t.tex[i]);
    }
    TTF_CloseFont(t.font);
    TTF_Quit();
TTF_ERR: SDL_DestroyRenderer(renderer);
REN_ERR: SDL_DestroyWindow(window);
WIN_ERR: SDL_Quit();
         free(blocks);
ERR:     return retval;
}
