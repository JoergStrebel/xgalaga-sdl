/*
 * XGalaga-SDL - a SDL port of XGalaga, clone of the game Galaga
 * Copyright (c) 1995-1998 Joe Rumsey (mrogre@mediaone.net)
 * Copyright (c) 2000 Andy Tarkinson <atark@thepipeline.net>
 * Copyright (c) 2010,2011 Frank Zago
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "xgalaga.h"


SDL_Window *screen;
SDL_Renderer *renderer;

void toggle_fullscreen(void)
{
	const int screen_height = winheight+WINTOPOV+WINBOTOV;
	Uint32 flags = SDL_GetWindowFlags(screen) ^ SDL_WINDOW_FULLSCREEN_DESKTOP;

	if (SDL_SetWindowFullscreen(screen, flags) < 0)
        return;

	SDL_SetWindowSize(screen, winwidth, screen_height);
}

void S_Initialize(int fullscreen)
{
	const int screen_height = winheight+WINTOPOV+WINBOTOV;
	Uint32 flags;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
        	fprintf(stderr, "Couldn't init video: %s\n", SDL_GetError());
        	exit(1);
    	}

	atexit(SDL_Quit);
    	
	if (fullscreen)	{
		flags =  SDL_WINDOW_FULLSCREEN_DESKTOP;    
		screen = SDL_CreateWindow("Xgalaga SDL", SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 0, 0, flags);
	} else {
		flags = SDL_WINDOW_SHOWN;
		screen = SDL_CreateWindow("Xgalaga SDL", SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, winwidth, screen_height, flags);
	}
        if (!screen) {
        	fprintf(stderr, "Couldn't set %dx%d video mode: %s\n",
                	winwidth, screen_height, SDL_GetError());
		exit(1);
	}

	// We must call SDL_CreateRenderer in order for draw calls to affect this window.
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Accelerated Renderer not available");   
		renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    }
    
	if (!renderer) {
		fprintf(stderr, "Couldn't find a renderer\n");
        exit(1);
	}

	if (fullscreen) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  
		SDL_RenderSetLogicalSize(renderer, winwidth, screen_height);
	}

	SDL_ShowCursor(SDL_DISABLE);
}

/* Draw a point. Surface must be locked with SDL_LockSurface(). */
void S_DrawPoint(unsigned int x, unsigned int y, struct color color)
{

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawPoint(renderer, x, y + WINTOPOV);

	return;
}

void S_ClearScreen(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void S_UpdateDisplay(void)
{

	SDL_RenderPresent(renderer);

}

void S_DrawImage(int x, int y, int frame, struct W_Image *image)
{
	int height, width;
	SDL_Rect srcrect, dstrect;

	y+= WINTOPOV;

	if (frame < 0) {
		/* Draw the whole thing regardless of frames. */
		height = image->height * image->frames;
		frame = 0;
	} else {
		/* Draw the given frame. */
		height = image->height;
		frame = frame % image->frames;
	}
	width = image->width;

	dstrect.x = x;
	dstrect.y = y;
	dstrect.w = width;
	dstrect.h = height;

	srcrect.x = 0;
	srcrect.y = height * frame;
	srcrect.w = width;
	srcrect.h = height;


	SDL_RenderCopy(renderer, image->surface, &srcrect, &dstrect);

}

void S_DrawRect(int x, int y, int w, int h, struct  color color)
{
    SDL_Rect dstrect;

	y+= WINTOPOV;

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = w;
    dstrect.h = h;

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &dstrect);

}
