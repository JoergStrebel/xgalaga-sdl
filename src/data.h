/*
 * XGalaga-SDL - a SDL port of XGalaga, clone of the game Galaga
 * Copyright (c) 1995-1998 Joe Rumsey (mrogre@mediaone.net)
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

extern SDL_Window *screen;
extern SDL_Renderer* renderer;

extern char *imagedir;
extern int verbose_image_loading;
extern int forceMono;

extern int buttonDown;
extern char *imagedirend;

extern int score;

extern int ships;
extern int shieldsleft;
extern int shieldon;

extern int level, metaLevel;
extern int startLevel;

extern int nextBonus;

extern int counter;

extern int mouseControl;
extern int wantStars;

extern int winwidth;
extern int winheight;

extern int alien_shape[];

extern int weapon; //type of weapon
extern int maxtorps, numtorps;

extern int plx; //player x position
extern int pldead; //flag for player's death

extern int movespeed;

extern int playSounds;

extern int plshield;

extern unsigned int title_page, pagetimer;

extern int gotlemon;

extern struct alien aliens[];

extern SFont_Font *fnt_reg_green;
extern SFont_Font *fnt_reg_cyan;
extern SFont_Font *fnt_reg_yellow;
extern SFont_Font *fnt_reg_red;
extern SFont_Font *fnt_reg_grey;
extern SFont_Font *fnt_big_red;

extern enum gstate gstate;

extern int fullscreen;
