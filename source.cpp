#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <vector>
#include <iostream>
#include <cmath>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
const int SCREEN_FPS = 60;
const int SCREEN_TICFKS_PER_FRAME = 1000 / SCREEN_FPS;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

const int ENEMY_AMOUNT = 30;

bool shooting = false;

bool automaticShooting = false;

class LTexture {
	public:
		LTexture() {
            mTexture = NULL;
            mWidth = 0;
            mHeight = 0;
        }
		~LTexture() {
            free();
        }
		bool loadFromFile( std::string path ) {
            free();
            SDL_Texture* newTexture = NULL;
            SDL_Surface* loadedSurface = IMG_Load(path.c_str());
            SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
            newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
            SDL_FreeSurface(loadedSurface);
            mTexture = newTexture;
            return mTexture != NULL;
        }
        bool loadFromRenderedText(std::string textureText, SDL_Color textColor) {
            free();
            SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
            mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
            mWidth = textSurface->w;
            mHeight = textSurface->h;
            SDL_FreeSurface(textSurface);
            return mTexture != NULL;
        }
		void free() {
            if(mTexture != NULL){
                SDL_DestroyTexture(mTexture);
                mTexture = NULL;
                mWidth = 0;
                mHeight = 0;
            }
        }
		void setColor( Uint8 red, Uint8 green, Uint8 blue ) {
            SDL_SetTextureColorMod(mTexture, red, green, blue);
        }
		void setBlendMode( SDL_BlendMode blending ) {
            SDL_SetTextureBlendMode(mTexture, blending);
        }
		void setAlpha( Uint8 alpha ) {
            SDL_SetTextureAlphaMod(mTexture, alpha);
        }
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){
            if(clip == NULL){
                SDL_Rect renderQuad = {x, y, mWidth, mHeight};
                SDL_RenderCopyEx(gRenderer, mTexture, NULL, &renderQuad, angle, center, flip);
            } else {
                SDL_Rect renderQuad = {x, y, clip->w, clip->h};
                SDL_RenderCopyEx(gRenderer, mTexture, NULL, &renderQuad, angle, center, flip);
            }
        }
		int getWidth(){
            return mWidth;
        }
		int getHeight(){
            return mHeight;
        }

	private:
		SDL_Texture* mTexture;
		int mWidth;
		int mHeight;
};

// class Ltimer {
//     public: 
//         LTimer() {
//             mStartTicks = 0;
//             mPausedTicks = 0;
//             mPaused = false;
//             mStarted = false;
//         }
//         void start() {
//             mStarted = true;
//             mPaused = false;
//             mStartTicks = SDL_GetTicks();
//             mPausedTicks = 0;
//         }
//         void stop() {
//             mStarted = false;
//             mPaused = false;
//             mStartTicks = 0;
//             mPausedTicks = 0;
//         }
//         void pause() {
//             if(mStarted && !mPaused){
//                 mPaused = true;
//                 mPausedTicks = SDL_GetTicks() - mStartTicks;
//                 mStartTicks = 0;
//             }
//         }
//         void unpause() {
//             if(mStarted && mPaused){
//                 mPaused = false;
//                 mStartTicks = SDL_GetTicks() - mPausedTicks;
//                 mPausedTicks = 0;
//             }
//         }
//         Uint32 getTicks() {
//             Uint32 time = 0;
//             if(mStarted){
//                 if(mPaused){
//                     time = mPausedTicks;
//                 } else {
//                     time = SDL_GetTicks() - mStartTicks;
//                 }
//             }
//             return time;
//         }
//         bool isStarted(){
//             return mStarted;
//         }
//         bool isPaused(){
//             return mPaused && mStarted;
//         }
//     private:
//         Uint32 mStartTicks;
//         Uint32 mPausedTicks;
//         bool mPaused;
//         bool mStarted;
// }

LTexture gPlayerTexture;
LTexture gProjectileTexture;
LTexture gEnemyTexture;

LTexture gTextTexture;
LTexture gTextStyleTexture;

class Projectile {
    public:
        int PROJECTILE_WIDTH;
        int PROJECTILE_HEIGHT;
        int PROJECTILE_VEL;
        float mPosX, mPosY;

    Projectile(int playerX, int playerY, int playerW, int playerH, int mouseX, int mouseY, int width, int height, int vel) {
        PROJECTILE_WIDTH = width;
        PROJECTILE_HEIGHT = height;
        PROJECTILE_VEL = vel;

        mPosX = playerX + (playerW/2) - (width/2);
        mPosY = playerY + (playerH/2) - (height/2);

        float Dlen = sqrt(((playerX - mouseX) * (playerX - mouseX)) + ((playerY - mouseY) * (playerY - mouseY)));
        mVelX = (playerX - mouseX)/Dlen;
        mVelY = (playerY - mouseY)/Dlen;
    }

    void move() {
        mPosX += -mVelX*PROJECTILE_VEL;
        mPosY += -mVelY*PROJECTILE_VEL;
    }

    bool invalidPos() {
        if((mPosX < 0) || mPosX + PROJECTILE_WIDTH > SCREEN_WIDTH || mPosY < 0 || mPosY + PROJECTILE_HEIGHT > SCREEN_HEIGHT){
            return true;
        }
        return false;
    }

    void render() {
        SDL_Rect r;
        r.w = PROJECTILE_WIDTH;
        r.h = PROJECTILE_HEIGHT;
        gProjectileTexture.render(mPosX, mPosY, &r);
    }

    private:
        float mVelX, mVelY;
};

class Player {
    public:
        static const int PLAYER_WIDTH = 20;
        static const int PLAYER_HEIGHT = 20;
        static const int PLAYER_VEL = 3;
        float mPosX, mPosY;

        Player() {
            mPosX = SCREEN_WIDTH/2;
            mPosY = SCREEN_HEIGHT/2;

            mVelX = 0;
            mVelY = 0;
        }

        void handleEvent(SDL_Event& e){
            if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
                switch(e.key.keysym.sym){
                    case SDLK_w: mVelY -= PLAYER_VEL; break;
                    case SDLK_s: mVelY += PLAYER_VEL; break;
                    case SDLK_a: mVelX -= PLAYER_VEL; break;
                    case SDLK_d: mVelX += PLAYER_VEL; break;
                }
            } else if(e.type == SDL_KEYUP && e.key.repeat == 0){
                switch(e.key.keysym.sym){
                    case SDLK_w: mVelY += PLAYER_VEL; break;
                    case SDLK_s: mVelY -= PLAYER_VEL; break;
                    case SDLK_a: mVelX += PLAYER_VEL; break;
                    case SDLK_d: mVelX -= PLAYER_VEL; break;
                }
            }

        }

        void move() {
            mPosX += mVelX;
            if((mPosX < 0) || (mPosX + PLAYER_WIDTH > SCREEN_WIDTH)){
                mPosX -= mVelX;
            }

            mPosY += mVelY;
            if((mPosY < 0) || (mPosY + PLAYER_HEIGHT > SCREEN_HEIGHT)){
                mPosY -= mVelY;
            }
        }

        void render() {
            SDL_Rect r;
            r.w = PLAYER_WIDTH;
            r.h = PLAYER_HEIGHT;
            gPlayerTexture.render(mPosX, mPosY, &r);
        }

        private:
            int mVelX, mVelY;
};

class Enemy {
    public:
        static const int ENEMY_WIDTH = 20;
        static const int ENEMY_HEIGHT = 20;
        static const int ENEMY_VEL = 1;
        float mPosX, mPosY;
        float Dlen;

        Enemy(int playerX, int playerY) {
            if(rand() % 2 == 0){
                mPosX = rand() % SCREEN_WIDTH;
                mPosY = rand() % 2 == 0 ? -ENEMY_HEIGHT: SCREEN_HEIGHT + ENEMY_HEIGHT;
            } else {
                mPosX = rand() % 2 == 0 ? -ENEMY_WIDTH: SCREEN_WIDTH + ENEMY_WIDTH;
                mPosY = rand() % SCREEN_HEIGHT;
            }

            Dlen = sqrt(((mPosX - playerX) * (mPosX - playerX)) + ((mPosY - playerY) * (mPosY - playerY)));
            mVelX = (mPosX - playerX)/Dlen;
            mVelY = (mPosY - playerY)/Dlen;
        }

        bool intersect(Projectile b){
            float x1 = mPosX + ENEMY_WIDTH/2;
            float y1 = mPosY + ENEMY_HEIGHT/2;
            int r1 = ENEMY_WIDTH/2;

            float x2 = b.mPosX + b.PROJECTILE_WIDTH/2;
            float y2 = b.mPosY + b.PROJECTILE_HEIGHT/2;
            int r2 = b.PROJECTILE_WIDTH/2;

            if((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <= (r1+r2)*(r1+r2)){
                return true;
            }

            return false;
        }

        bool checkPlayerHit(Player b){
            float x1 = mPosX + ENEMY_WIDTH/2;
            float y1 = mPosY + ENEMY_HEIGHT/2;
            int r1 = ENEMY_WIDTH/2;

            float x2 = b.mPosX + b.PLAYER_WIDTH/2;
            float y2 = b.mPosY + b.PLAYER_HEIGHT/2;
            int r2 = b.PLAYER_WIDTH/2;

            if((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <= (r1+r2)*(r1+r2)){
                return true;
            }

            return false;
        }                             

        void move(int playerX, int playerY, int playerR) {
            Dlen = sqrt(((mPosX - playerX) * (mPosX - playerX)) + ((mPosY - playerY) * (mPosY - playerY)));
            mVelX = (mPosX - playerX)/Dlen;
            mVelY = (mPosY - playerY)/Dlen;

            mPosX += -mVelX*ENEMY_VEL;
            mPosY += -mVelY*ENEMY_VEL;
        }

        void render() {
            SDL_Rect r;
            r.w = ENEMY_WIDTH;
            r.h = ENEMY_HEIGHT;
            gEnemyTexture.render(mPosX, mPosY, &r);
        }
    private:
        float mVelX, mVelY;
};

bool mousePress(SDL_MouseButtonEvent& b){
    if(b.button == SDL_BUTTON_LEFT){
        return true;
    }
    return false;
}

Projectile createProjectile(Player player, int shootingStyle, int shootingStyles[2][4], std::vector<Projectile> projectiles){
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Projectile p(
        player.mPosX, 
        player.mPosY, 
        player.PLAYER_WIDTH,
        player.PLAYER_HEIGHT,
        mouseX, 
        mouseY, 
        shootingStyles[shootingStyle][0], 
        shootingStyles[shootingStyle][1], 
        shootingStyles[shootingStyle][2]
    );
    return p;
}

bool loadMedia(){
    bool success = true;
    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    if(gFont == NULL){
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        SDL_Color textColor = {0, 0, 0};
        if( !gTextTexture.loadFromRenderedText( "The quick brown fox jumps over the lazy dog", textColor ) || 
            !gTextStyleTexture.loadFromRenderedText( "The quick brown fox jumps over the lazy dog", textColor ) 
            ) {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }

    return success;
}

void close() {
    gPlayerTexture.free();
    gProjectileTexture.free();
    gEnemyTexture.free();
    gTextTexture.free();
    gTextStyleTexture.free();
    TTF_CloseFont(gFont);
    gFont = NULL;
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool init() {
	bool success = true;
	if(SDL_Init( SDL_INIT_VIDEO ) < 0) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else {
		if(!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
			printf( "Warning: Linear texture filtering not enabled!" );
		}
		gWindow = SDL_CreateWindow( "Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if(gWindow == NULL) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if(gRenderer == NULL) {
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else {
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)) {
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

                if(TTF_Init() == -1){
                    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
			}
		}
	}
	return success;
}

int score = 0;

int main(int argc, char* args[]){
    srand((unsigned)time(NULL));
    if(!init()){
        printf("Failed to initialize!\n");
    } else if(!loadMedia()){
        printf("Failed to load media!\n");
    } else {
        gPlayerTexture.loadFromFile("images/dot.bmp");
        gProjectileTexture.loadFromFile("images/dot.bmp");
        gEnemyTexture.loadFromFile("images/enemy.bmp");

        std::string shootingStyleNames[3]{
            "Lightning",
            "Rocket",
            "Ultimate"
        };

        int shootingStyles[sizeof(shootingStyleNames)/sizeof(shootingStyleNames[0])][4]{
            {5, 5, 20, 1},
            {50, 50, 6, 0},
            {50, 50, 12, 1}
        };

        int shootingStyle = 1;

        std::vector<Projectile> projectiles;

        std::vector<Enemy> enemies;
        for(int i = 0; i < ENEMY_AMOUNT; i++){
            Enemy e(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
            enemies.push_back(e);
        }

        bool quit = false;

        SDL_Event e;

        Player player;

        SDL_Color c;
        c.r = 0;
        c.g = 0;
        c.b = 0;

        while(!quit){
            while(SDL_PollEvent(&e) != 0){
                if(e.type == SDL_QUIT){
                    quit = true;
                }

                if(e.type == SDL_MOUSEBUTTONDOWN){
                    if(mousePress(e.button)){
                        if(shootingStyles[shootingStyle][3]){
                            shooting = true;
                        } else {
                            projectiles.push_back(createProjectile(player, shootingStyle, shootingStyles, projectiles));
                        }
                        
                    } 
                } else if(e.type == SDL_MOUSEBUTTONUP && shootingStyles[shootingStyle][3]){
                    shooting = false;
                }

                if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
                    switch(e.key.keysym.sym){
                        case SDLK_r: shootingStyle++; if(shootingStyle >= sizeof(shootingStyles)/sizeof(shootingStyles[0])) shootingStyle = 0; break;
                    }
                }

                player.handleEvent(e);
            }

            if(shooting && shootingStyles[shootingStyle][3]){
                projectiles.push_back(createProjectile(player, shootingStyle, shootingStyles, projectiles));
            }

            player.move();

            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);

            gTextTexture.loadFromRenderedText(std::to_string(score), c);
            gTextStyleTexture.loadFromRenderedText(shootingStyleNames[shootingStyle], c);

            player.render();

            for(int i = 0; i < projectiles.size(); i++){
                projectiles[i].move();
                if(projectiles[i].invalidPos()){
                    projectiles.erase(projectiles.begin()+i);
                } else {
                    projectiles[i].render();
                }
            }

            for(int i = 0; i < enemies.size(); i++){
                enemies[i].move(player.mPosX + (player.PLAYER_WIDTH/2), player.mPosY + (player.PLAYER_HEIGHT/2), player.PLAYER_WIDTH/2);
                enemies[i].render();
                if(enemies[i].checkPlayerHit(player)){
                    score -= 5;
                    enemies.erase(enemies.begin()+i);
                } else {
                    for(int j = 0; j < projectiles.size(); j++){
                        if(enemies[i].intersect(projectiles[j])){
                            score++;
                            enemies.erase(enemies.begin()+i);
                            if(shootingStyle != 1) projectiles.erase(projectiles.begin()+j);
                        }
                    }
                }
            }

            for(int i = enemies.size(); i < ENEMY_AMOUNT; i++){
                Enemy e(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                enemies.push_back(e);
            }

            if(score < 0){
                score = 0;
            }

            gTextTexture.render(0, 0);
            gTextStyleTexture.render(SCREEN_WIDTH/2 - (gTextStyleTexture.getWidth()/2), 0);


            SDL_RenderPresent(gRenderer);
        }
    }
    close();
    return 0;
}