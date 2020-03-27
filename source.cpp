#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <vector>

#include <iostream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

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

LTexture gPlayerTexture;
LTexture gProjectileTexture;

class Projectile {
    public:
        static const int PROJECTILE_WIDTH = 5;
        static const int PROJECTILE_HEIGHT = 5;
        static const int PROJECTILE_VEL = 12;
        float mPosX, mPosY;

    Projectile(int playerX, int playerY, int mouseX, int mouseY) {
        mPosX = playerX;
        mPosY = playerY;

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

bool mousePress(SDL_MouseButtonEvent& b){
    if(b.button == SDL_BUTTON_LEFT){
        return true;
    }
    return false;
}

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

void close() {
    gPlayerTexture.free();
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
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
			}
		}
	}

	return success;
}

int main(int argc, char* args[]){
    srand((unsigned)time(NULL));
    if(!init()){
        printf("Failed to initialize!\n");
    } else {
        gPlayerTexture.loadFromFile("images/dot.bmp");
        gProjectileTexture.loadFromFile("images/dot.bmp");

        std::vector<Projectile> projectiles;

        bool quit = false;

        SDL_Event e;

        Player player;

        while(!quit){
            while(SDL_PollEvent(&e) != 0){
                if(e.type == SDL_QUIT){
                    quit = true;
                }
                if(e.type == SDL_MOUSEBUTTONDOWN){
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);

                    if(mousePress(e.button)){
                        Projectile p(player.mPosX + (player.PLAYER_WIDTH/2), player.mPosY + (player.PLAYER_HEIGHT/2), mouseX, mouseY);
                        projectiles.push_back(p);
                 
                    }
                }

                player.handleEvent(e);
            }

            player.move();

            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);

            player.render();

            for(int i = 0; i < projectiles.size(); i++){
                projectiles[i].move();
                if(projectiles[i].invalidPos()){
                    projectiles.erase(projectiles.begin()+i);
                } else {
                    projectiles[i].render();
                }
            }

            SDL_RenderPresent(gRenderer);
        }
    }
    close();

    return 0;
}