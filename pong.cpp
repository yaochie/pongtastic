#define _USE_MATH_DEFINES
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstdlib>

//constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

const int SCORE_FONT_SIZE = 48;
const int PAUSE_FONT_SIZE = 24;

const int DIVIDER_WIDTH = 2;
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT_DEF = 50;
const int BALL_WIDTH = 10;
const int BALL_INIT_VEL = 150;

const int LEFT_SCORE_X = 260;
const int LEFT_SCORE_Y = 50;
const int RIGHT_SCORE_X = 378;
const int RIGHT_SCORE_Y = 50;

const int SCORE_LIMIT = 7;

// key settings
const SDLKey leftUp = SDLK_a;
const SDLKey leftDown = SDLK_z;
const SDLKey rightUp = SDLK_UP;
const SDLKey rightDown = SDLK_DOWN;

//variables
SDL_Surface *screen;
SDL_Surface *left, *right;

SDL_Event event;

TTF_Font *font;
TTF_Font *fontPause;

SDL_Color textColor = {0xFF, 0xFF, 0xFF};

//Game states
enum GameStates
{
    STATE_NULL,
    STATE_INTRO,
    STATE_HELP,
    STATE_CREDITS,
    STATE_SETTINGS,
    STATE_GAME,
    STATE_EXIT,
};

//functions
bool init();
bool load_files();
void clean_up();
void apply_surface(int x, int y, SDL_Surface *source, SDL_Surface *destination, SDL_Rect *clip = NULL);

bool check_collision(int ballX, int ballY, SDL_Rect *pad);

bool show_start();

//Game state handling
void set_next_state(int newState);
void change_state();

// Timer - to regulate ball speed
class Timer
{
    private:
        int startTicks;
        bool started;
    public:
        Timer();
        void start();
        int get_ticks();
};

//Paddle class - movement of paddle
class Paddle
{
    private:
        int vel;
        double frameVel;
        //int score;
        double realY;
        SDL_Rect position;
        SDLKey goUp, goDown;
    public:
        Paddle();
        Paddle(SDL_Rect initialPosition, SDLKey up, SDLKey down);
        void init(SDL_Rect initialPosition, SDLKey up, SDLKey down);
        void set_keys(SDLKey up, SDLKey down);
        SDLKey get_up_key();
        SDLKey get_down_key();
        void handle_input();
        void move(int delta);
        void show();
        SDL_Rect *get_position();
};

//Ball class - movement of paddle, also handles collisions between ball and scoring areas, walls, paddle
class Ball
{
    private:
        double vel, frameVel;
        bool right;
        bool delayed;
        int delayTick;
        bool scored;
        int scoredTick;
        double angle;
        double realX, realY;
        SDL_Rect position;
    public:
        Ball();
        void init();
        void speed_up();
        int move(SDL_Rect *leftPad, SDL_Rect *rightPad, int delta);
        void show();
        void reset();
        bool is_delayed();
        int delayed_ticks();
        void delay();
        void stop_delay();
        void have_scored();
        bool is_scored();
        int scored_ticks();
        void begin();
};

//GameState class
class GameState
{
    public:
        virtual void handle_events() = 0;
        virtual void logic() = 0;
        virtual void render() = 0;
        virtual ~GameState(){};
};

int stateID = STATE_NULL;
int nextState = STATE_NULL;

GameState *currentState = NULL;

//Game states
//-------------------------------------------------
class Intro : public GameState
{
    private:
        SDL_Surface *message;
        SDL_Surface *help;
        SDL_Surface *credits;
    public:
        Intro();
        ~Intro();
        void handle_events();
        void logic();
        void render();
};

class Game : public GameState
{
    private:
        SDL_Rect divider;
        Paddle leftPaddle;
        Paddle rightPaddle;
        Ball theBall;
        int leftScore, rightScore;
        bool paused, endGame, justStarted;
        int startedTick;
        Timer delta;
    public:
        Game();
        ~Game();
        void handle_events();
        void logic();
        void render();
        void update_scores(int leftScore, int rightScore);
        int start_ticks();
        void reset_start();
        void show_pause();
};

class Help : public GameState
{
    private:
        SDL_Surface *player1;
        SDL_Surface *player1Instructions;
        SDL_Surface *player2;
        SDL_Surface *player2Instructions;
        SDL_Surface *pause;
        SDL_Surface *escape;
    public:
        Help();
        ~Help();
        void handle_events();
        void logic();
        void render();
};

class Credits : public GameState
{
    private:
        SDL_Surface *cred;
        SDL_Surface *fontCred;
        SDL_Surface *nameCred;
    public:
        Credits();
        ~Credits();
        void handle_events();
        void logic();
        void render();
};

//-------------------------------------------------

//Error logger
std::ofstream logger("log.txt");
void log(std::string message)
{
    logger << message << std::endl;
    logger.flush();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    //Key settings array
    //SDLKey keys[4];

    //Initialization
    if (!init())
        return 1;
    if (!load_files())
        return 1;

    stateID = STATE_INTRO;
    currentState = new Intro();

    while (stateID != STATE_EXIT)
    {
        currentState->handle_events();
        currentState->logic();
        change_state();
        currentState->render();

        if (SDL_Flip(screen) == -1)
            return 1;
    }

    clean_up();
    return 0;
}

bool init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        return false;

    if (TTF_Init() == -1)
        return false;

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (screen == NULL)
        return false;

    SDL_WM_SetCaption("Pong", NULL);

    return true;
}
//SDLKey &keys[]
bool load_files()
{
    font = TTF_OpenFont("Eurosti.TTF", SCORE_FONT_SIZE);
    fontPause = TTF_OpenFont("Eurosti.TTF", PAUSE_FONT_SIZE);

    if (font == NULL || fontPause == NULL)
        return false;

    return true;
}

void clean_up()
{
    //SDL_FreeSurface(text);
    //SDL_FreeSurface(frames);

    TTF_CloseFont(font);
    TTF_CloseFont(fontPause);

    logger.close();
/*
    std::ofstream save("savedata");
    save << leftUp << " ";
    save << leftDown << " ";
    save << rightUp << " ";
    save << rightDown;
    save.close();*/

    SDL_Quit();
}

void apply_surface(int x, int y, SDL_Surface *source, SDL_Surface *destination, SDL_Rect *clip)
{
    SDL_Rect offset;

    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, clip, destination, &offset);
}

Timer::Timer()
{
    startTicks = 0;
    started = false;
}

void Timer::start()
{
    startTicks = SDL_GetTicks();
    started = true;
}

int Timer::get_ticks()
{
    return (SDL_GetTicks() - startTicks);
}

bool check_collision(int ballX, int ballY, SDL_Rect *pad)
{
    int ballLeft, ballRight, ballTop, ballBottom;
    int paddleLeft, paddleRight, paddleTop, paddleBottom;

    ballLeft = ballX;
    ballRight = ballX + BALL_WIDTH;
    ballTop = ballY;
    ballBottom = ballY + BALL_WIDTH;

    paddleLeft = pad->x;
    paddleRight = pad->x + pad->w;
    paddleTop = pad->y;
    paddleBottom = pad->y + pad->h;

    if ((ballLeft >= paddleRight || ballRight <= paddleLeft || ballTop >= paddleBottom || ballBottom <= paddleTop) == false)
        return true;

    return false;
}

Paddle::Paddle()
{
    vel = 0;
}

Paddle::Paddle(SDL_Rect initialPosition, SDLKey up, SDLKey down)
{
    vel = 0;
    position = initialPosition;
    realY = position.y;
    goUp = up;
    goDown = down;
}

void Paddle::init(SDL_Rect pos, SDLKey up, SDLKey down)
{
    position = pos;
    realY = position.y;
    goUp = up;
    goDown = down;
}

void Paddle::show()
{
    SDL_FillRect(screen, &position, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
}

void Paddle::handle_input()
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == goUp)
            vel -= 250;
        else if (event.key.keysym.sym == goDown)
            vel += 250;
    }
    else if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == goUp)
            vel += 250;
        else if (event.key.keysym.sym == goDown)
            vel -= 250;
    }
}

void Paddle::move(int delta)
{
    frameVel = vel * double(delta) / 1000;
    realY += frameVel;

    //handle collision
    if (realY < 0)
        realY = 0;
    else if ((realY + position.h) > SCREEN_HEIGHT)
        realY = SCREEN_HEIGHT - position.h;

    position.y = int(realY);
}

SDL_Rect *Paddle::get_position()
{
    return &position;
}

Ball::Ball()
{
    vel = BALL_INIT_VEL;
    delayed = true;
    scored = false;
    delayTick = SDL_GetTicks();

    if (rand()%2 == 1)
        right = true;
    else
        right = false;

    if (rand()%2 == 1)
        angle = M_PI_4;
    else
        angle = -M_PI_4;

    realX = (SCREEN_WIDTH - BALL_WIDTH)/2;
    realY = (SCREEN_HEIGHT - BALL_WIDTH)/2;
    position.x = int(realX);
    position.y = int(realY);
    position.w = BALL_WIDTH;
    position.h = BALL_WIDTH;
}

int Ball::move(SDL_Rect *leftPad, SDL_Rect *rightPad, int delta)
{
    //display the velocity
    /*std::stringstream stream;
    stream << vel;
    log(stream.str());*/

    //if delayed, don't do anything
    if (delayed || scored)
        return 0;

    frameVel = vel * double(delta) / 1000;

    if (right)
        realX += frameVel * cos(angle);
    else
        realX -= frameVel * cos(angle);
    realY -= frameVel * sin(angle);

    //handle collision
    if (realY < 0)
    {
        realY = 0;
        angle = -angle;
        realY -= frameVel * sin(angle);
    }
    else if ((realY + position.h) > SCREEN_HEIGHT)
    {
        realY = SCREEN_HEIGHT - position.h;
        angle = -angle;
        realY -= frameVel * sin(angle);
    }

    if (check_collision(int(realX), int(realY), leftPad))
    {
        right = true;
        realX = leftPad->x + leftPad->w;
        realX += (frameVel * cos(angle));
        vel += 20;
    }
    else if (check_collision(int(realX), int(realY), rightPad))
    {
        right = false;
        realX = rightPad->x - position.w;
        realX -= (frameVel * cos(angle));
        vel += 20;
    }
    else if (realX < 0)
        return 2;
    else if ((realX + position.w) > SCREEN_WIDTH)
        return 1;

    position.x = int(realX);
    position.y = int(realY);

    return 0;
}

void Ball::show()
{
    SDL_FillRect(screen, &position, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
}

void Ball::reset()
{
    vel = BALL_INIT_VEL;
    delayed = true;
    scored = false;
    delayTick = SDL_GetTicks();

    if (rand()%2 == 1)
        right = true;
    else
        right = false;

    if (rand()%2 == 1)
        angle = M_PI_4;
    else
        angle = -M_PI_4;

    realX = (SCREEN_WIDTH - BALL_WIDTH)/2;
    realY = (SCREEN_HEIGHT - BALL_WIDTH)/2;
    position.x = int(realX);
    position.y = int(realY);
    position.w = BALL_WIDTH;
    position.h = BALL_WIDTH;
}

bool Ball::is_delayed()
{
    return delayed;
}

void Ball::delay()
{
    delayed = true;
    delayTick = SDL_GetTicks();
}

int Ball::delayed_ticks()
{
    return (SDL_GetTicks() - delayTick);
}

void Ball::stop_delay()
{
    delayed = false;
}

void Ball::begin()
{
    delayTick = SDL_GetTicks();
}

void Ball::have_scored()
{
    scored = true;
    scoredTick = SDL_GetTicks();
}

bool Ball::is_scored()
{
    return scored;
}

int Ball::scored_ticks()
{
    return (SDL_GetTicks() - scoredTick);
}

void set_next_state(int newState)
{
    if (nextState != STATE_EXIT)
        nextState = newState;
}

void change_state()
{
    if (nextState != STATE_NULL)
    {
        if (nextState != STATE_EXIT)
            delete currentState;

        switch(nextState)
        {
            case STATE_INTRO:
                currentState = new Intro();
                break;
            case STATE_GAME:
                currentState = new Game();
                break;
            case STATE_HELP:
                currentState = new Help();
                break;
            case STATE_CREDITS:
                currentState = new Credits();
                break;
            /*case STATE_SETTINGS:
                currentState = new Settings();
                break;*/
            default:
                break;
        }
        stateID = nextState;
        nextState = STATE_NULL;
    }
}

Intro::Intro()
{
    message = TTF_RenderText_Blended(font, "Press Spacebar to start.", textColor);
    help = TTF_RenderText_Blended(fontPause, "Help: H", textColor);
    credits = TTF_RenderText_Blended(fontPause, "Credits: C", textColor);
}

Intro::~Intro()
{
    SDL_FreeSurface(message);
    SDL_FreeSurface(help);
    SDL_FreeSurface(credits);
}

void Intro::handle_events()
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            set_next_state(STATE_EXIT);
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
                set_next_state(STATE_GAME);
            else if (event.key.keysym.sym == SDLK_h)
                set_next_state(STATE_HELP);
            else if (event.key.keysym.sym == SDLK_c)
                set_next_state(STATE_CREDITS);
            else if (event.key.keysym.sym == SDLK_ESCAPE)
                set_next_state(STATE_EXIT);
        }
    }
}

void Intro::logic()
{
}

void Intro::render()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
    apply_surface((SCREEN_WIDTH - message->w)/2, (SCREEN_HEIGHT - message->h)/2, message, screen);
    apply_surface(30, 420, help, screen);
    apply_surface(500, 420, credits, screen);
}

Game::Game()
{
    //Set up paddles, ball and central divider
    divider.x = (SCREEN_WIDTH-DIVIDER_WIDTH)/2;
    divider.y = 0;
    divider.w = DIVIDER_WIDTH;
    divider.h = SCREEN_HEIGHT;

    SDL_Rect leftPos, rightPos;

    leftPos.x = 30;
    leftPos.y = (SCREEN_HEIGHT-PADDLE_HEIGHT_DEF)/2;
    leftPos.w = PADDLE_WIDTH;
    leftPos.h = PADDLE_HEIGHT_DEF;

    rightPos.x = SCREEN_WIDTH - 30 - PADDLE_WIDTH;
    rightPos.y = (SCREEN_HEIGHT-PADDLE_HEIGHT_DEF)/2;
    rightPos.w = PADDLE_WIDTH;
    rightPos.h = PADDLE_HEIGHT_DEF;

    leftPaddle.init(leftPos, leftUp, leftDown);
    rightPaddle.init(rightPos, rightUp, rightDown);

    leftScore = 0;
    rightScore = 0;
    paused = false;
    endGame = false;
    justStarted = true;
    startedTick = SDL_GetTicks();
}

Game::~Game()
{
}

void Game::handle_events()
{
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                set_next_state(STATE_EXIT);
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    set_next_state(STATE_INTRO);
                else if (event.key.keysym.sym == SDLK_p)
                {
                    if (!paused)
                        paused = true;
                    else
                        paused = false;
                }
            default:
                leftPaddle.handle_input();
                rightPaddle.handle_input();
                break;
        }
    }
}

void Game::logic()
{
    if (!paused)
    {
        leftPaddle.move(delta.get_ticks());
        rightPaddle.move(delta.get_ticks());
        if (!justStarted)
        {
            switch ( theBall.move(leftPaddle.get_position(), rightPaddle.get_position(), delta.get_ticks()) )
            {
                case 1:
                    leftScore++;
                    if (leftScore >= SCORE_LIMIT)
                        endGame = true;
                    theBall.have_scored();
                    break;
                case 2:
                    rightScore++;
                    if (rightScore >= SCORE_LIMIT)
                        endGame = true;
                    theBall.have_scored();
                    break;
                default:
                    break;
            }
        }
        else if (justStarted)
        {
            if (start_ticks() > 500)
            {
                justStarted = false;
                theBall.begin();
            }
        }
    }
    else if (justStarted)
        reset_start();
    delta.start();
}

void Game::render()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
    SDL_FillRect(screen, &divider, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

    leftPaddle.show();
    rightPaddle.show();

    update_scores(leftScore, rightScore);

    if (justStarted)
    {
        SDL_Surface* startMessage;
        std::stringstream startMess;
        startMess << "First to " << SCORE_LIMIT;
        startMessage = TTF_RenderText_Blended(fontPause, startMess.str().c_str(), textColor);
        apply_surface((SCREEN_WIDTH - startMessage->w)/2, 100, startMessage, screen);
        SDL_FreeSurface(startMessage);
        if (paused)
            show_pause();
        delta.start();
    }
    else if (!endGame)
    {
        if (!paused)
        {
            if (theBall.is_delayed())
            {
                if (theBall.delayed_ticks() > 2000)
                {
                    theBall.stop_delay();
                }
                else if (theBall.delayed_ticks() < 500 || (theBall.delayed_ticks() > 1000 && theBall.delayed_ticks() < 1500) )
                    theBall.show();
            }
            else if (theBall.is_scored())
            {
                //log("scored");
                if (theBall.scored_ticks() > 400)
                    theBall.reset();
                else
                    theBall.show();
            }
            else
                theBall.show();
        }
        else
        {
            theBall.show();
            show_pause();
        }
    }
    else
    {
        theBall.show();
        SDL_Surface *endMessage;
        if (leftScore >= SCORE_LIMIT)
        {
            endMessage = TTF_RenderText_Blended(fontPause, "Player 1 wins!", textColor);
            apply_surface((SCREEN_WIDTH/2 - endMessage->w)/2, 400, endMessage, screen);
        }
        else
        {
            endMessage = TTF_RenderText_Blended(fontPause, "Player 2 wins!", textColor);
            apply_surface((SCREEN_WIDTH*3/2 - endMessage->w)/2, 400, endMessage, screen);
        }
        if (SDL_Flip(screen) == -1)
            return;
        SDL_FreeSurface(endMessage);
    }
}

void Game::show_pause()
{
    SDL_Surface *pauseMessage;
    pauseMessage = TTF_RenderText_Blended(fontPause, "Press P to resume.", textColor);

    apply_surface((screen->w - pauseMessage->w)/2, (screen->h - pauseMessage->h)/2, pauseMessage, screen);
    if (SDL_Flip(screen) == -1)
        return;
    SDL_FreeSurface(pauseMessage);
}

void Game::update_scores(int leftScore, int rightScore)
{
    std::stringstream stream;
    std::string leftSc, rightSc;

    stream << leftScore << " " << rightScore;
    stream >> leftSc;
    stream >> rightSc;
    stream.flush();

    //handle error?
    left = TTF_RenderText_Blended(font, leftSc.c_str(), textColor);
    right = TTF_RenderText_Blended(font, rightSc.c_str(), textColor);

    apply_surface(LEFT_SCORE_X-left->w/2, LEFT_SCORE_Y-left->h/2, left, screen);
    apply_surface(RIGHT_SCORE_X-right->w/2, RIGHT_SCORE_Y-right->h/2, right, screen);

    SDL_FreeSurface(left);
    SDL_FreeSurface(right);
}

int Game::start_ticks()
{
    return (SDL_GetTicks() - startedTick);
}

void Game::reset_start()
{
    startedTick = SDL_GetTicks();
}

Help::Help()
{
    player1 = TTF_RenderText_Blended(fontPause, "Player 1:", textColor);
    player1Instructions = TTF_RenderText_Blended(fontPause, "Up: A, Down: Z", textColor);
    player2 = TTF_RenderText_Blended(fontPause, "Player 2:", textColor);
    player2Instructions = TTF_RenderText_Blended(fontPause, "Up: Up, Down: Down", textColor);
    pause = TTF_RenderText_Blended(fontPause, "Pause: P", textColor);
    escape = TTF_RenderText_Blended(fontPause, "Exit: Escape", textColor);
}

Help::~Help()
{
    SDL_FreeSurface(player1);
    SDL_FreeSurface(player1Instructions);
    SDL_FreeSurface(player2);
    SDL_FreeSurface(player2Instructions);
    SDL_FreeSurface(pause);
    SDL_FreeSurface(escape);
}

void Help::handle_events()
{
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                nextState = STATE_EXIT;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    nextState = STATE_INTRO;
                break;
        }
    }
}

void Help::logic()
{
}

void Help::render()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
    apply_surface(30, 30, player1, screen);
    apply_surface(50, 70, player1Instructions, screen);
    apply_surface(30, 110, player2, screen);
    apply_surface(50, 150, player2Instructions, screen);
    apply_surface(30, 230, pause, screen);
    apply_surface(30, 310, escape, screen);
}

Credits::Credits()
{
    cred = TTF_RenderText_Blended(fontPause, "Created with SDL 1.2.15 and SDL_ttf.", textColor);
    fontCred = TTF_RenderText_Blended(fontPause, "Font: Eurostile.", textColor);
    SDL_Color nameColor = {0x11, 0x11, 0x11};
    nameCred = TTF_RenderText_Blended(fontPause, "Made by Yao Chong", nameColor);
}

Credits::~Credits()
{
    SDL_FreeSurface(cred);
    SDL_FreeSurface(fontCred);
    SDL_FreeSurface(nameCred);
}

void Credits::handle_events()
{
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                nextState = STATE_EXIT;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    nextState = STATE_INTRO;
                break;
        }
    }
}

void Credits::logic()
{
}

void Credits::render()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
    apply_surface((SCREEN_WIDTH - cred->w)/2, (SCREEN_HEIGHT - cred->h)/2 - 40, cred, screen);
    apply_surface((SCREEN_WIDTH - fontCred->w)/2, (SCREEN_HEIGHT - fontCred->h)/2 + 40, fontCred, screen);
    apply_surface((SCREEN_WIDTH*3/2 - nameCred->w)/2, SCREEN_HEIGHT - 80, nameCred, screen);
}
