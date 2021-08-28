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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "xgalaga.h"

static struct torp torps[MAXTORPS];
static struct star stars[NUMSTARS];
static struct torp *first_etorp;

static int convoyx = 0, convoymove = 1;
static int livecount = 0; //counter for the number of alive aliens
static int starspeed = 1;
static int attacking = 0, maxattacking, entering=0;
static int maxetorps = 5, numetorps=0;
static int plflash = 50; //timer for flashing and invulnerability
static int mx;

static struct W_Image *playerShip;
static struct W_Image *playerTorp;
static struct W_Image *enemyTorp;
static struct W_Image *shieldImage;

#define convoy_x_pos(i) (convoyx+(20 * (i - 10 * (i/10))))
#define convoy_y_pos(i) (20 + (20*(i/10)))

static int moves[16][2] = {
    {0,-4},
    {1,-4},
    {3,-3},
    {4,-1},
    {4,0},
    {4,1},
    {3,3},
    {1,4},
    {0,4},
    {-1,4},
    {-3,3},
    {-4,1},
    {-4,0},
    {-4,-1},
    {-3,-3},
    {-1,-4}
};

static void xgal_exit(int v)
{
	sound_exit();
    exit(v);
}

static void print_usage(void)
{
    printf("XGalaga v%s\n"
		   "Command line options:\n"
		   "  -scores              Prints out the high score files and exits\n"
		   "  -nosound             Turn sound OFF\n"
		   "  -level <number>      Choose starting level (>= 1)\n"
		   "  -window              Start in windowed mode instead of fullscreen\n"
		   "  -winsize <WxH>       Window size (default 468 x 596)\n"
		   "\n"
		   "This game is free software, under the GPLv2\n"
		   "\n"
		   "Keyboard commands:\n"
		   "  p - pauses\n"
		   "  q - end this game\n"
		   "  alt + enter - Toggle fullscreen - window\n",
		   VERSION);
}

/*------------------stars-----------------*/

/* Pick a random color, except black. */
static void get_random_star_color(struct star *star)
{
	Uint8 r, g, b;

	switch(random() % 5) {
	case 0:
		r = 0xF9; g = 0xF9; b = 0xF9; break; //white
	case 1:
		r = 0x00; g = 0xFF; b = 0x00; break; //green
	case 2:
		r = 0x00; g = 0xFF; b = 0xFF; break; // turquoise
	case 3:
		r = 0x90; g = 0x90; b = 0x90; break; 
	default:
		r = 0xFF; g = 0xFF; b = 0x00; break; // yellow
	}

	star->pixel.r = r;
	star->pixel.g = g;
	star->pixel.b = b;

}

static void init_stars(void)
{
    int i;

    for(i=0;i<NUMSTARS;i++) {
        stars[i].x = random()%winwidth;
        stars[i].y = random()%winheight;
        stars[i].speed = (random()%3)+1;
		get_random_star_color(&stars[i]);
    }
}

static void do_stars(void)
{
    int i;

    if(wantStars) {

#if 0
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
#endif

        for(i=0;i<NUMSTARS;i++) {
			stars[i].y+=stars[i].speed*((starspeed < 20) ? ABS(starspeed) : 20);
            if(stars[i].y >= winheight) {
                stars[i].y-=winheight-ABS(starspeed);
                stars[i].x = random() % winwidth;
				get_random_star_color(&stars[i]);
            }

			S_DrawPoint(stars[i].x, stars[i].y, stars[i].pixel);
        }

#if 0
		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);
#endif
    }

    if(starspeed != 1) {
        char buf[20];
		int y;

        sprintf(buf, "LEVEL %d", level+1);

		y = (winheight - SFont_TextHeight(fnt_big_red))/2;
		SFont_WriteCenter(fnt_big_red, y, buf);

    }
}

/*-------------------aliens---------------*/

static void delete_etorps(void)
{
    struct torp *tmp;

    while(first_etorp) {
        tmp = first_etorp->next;
        free(first_etorp);
        first_etorp = tmp;
    }
}

static void init_aliens(int level)
{
    int i;

    convoyx=0;
    convoymove = 1;

    maxattacking = 1 + (level*2);
    if(maxattacking > 30)
        maxattacking = 30;
    attacking = 0;
    maxetorps = 10 + (level*5);
    numetorps = 0;

    delete_etorps();
    metaLevel = 1;
    if (read_level(level) <= 0) {
		fprintf(stderr, "Error reading level %d\n", level);
		exit(0);
    }

    for(i=0;i<MAXALIENS;i++) {
        livecount++;
        new_alien(i, &aliens[i]);
    }

    for(i=0;i<MAXTORPS;i++)
        torps[i].alive = 0;
}


static void undo_aliens(void)
{
    int i;

    for(i=0;i<MAXALIENS;i++) {
        if(aliens[i].dying) {
            aliens[i].alive = 0;
            aliens[i].dying=0;
        }
    }
}

static void do_escort(int i)
{
    int fs = aliens[i].escorting;

    if(!aliens[fs].alive) {
        aliens[i].escorting = -1;
    } else if(aliens[fs].dir >= 0) {
        aliens[i].dir = aliens[fs].dir;
    } else {
        aliens[i].x = 20 * (i - 10 * (i/10)) + convoyx + convoymove;
        aliens[i].y = -10;
        aliens[i].dir = -2;
        aliens[i].path = -1;
        aliens[i].steer = 2;
        aliens[i].escorting = -1;
    }
}

static void do_convoy(int i)
{
    aliens[i].x += convoymove;
    if((entering == 0) &&
       (attacking < maxattacking) &&
       ((livecount < maxattacking) ||
        ((random()%10000) < (level + 2 *(48-(livecount)))))) {
        switch(random()%2) {
        case 0:
            path_dir(P_PEELLEFT, 0, &aliens[i].dir, &aliens[i].steer);
            aliens[i].path = P_PEELLEFT;
            break;
        case 1:
            path_dir(P_PEELRIGHT, 0, &aliens[i].dir, &aliens[i].steer);
            aliens[i].path = P_PEELRIGHT;
            break;
        }
        aliens[i].path_pos = 0;
        attacking++;
        if(i<10) { /* Flagship, grab escorts */
            int e;
            for(e=i+9;e<i+12;e++) {
                if(aliens[e].alive && aliens[e].dir == -1) {
                    aliens[e].escorting = i;
                }
            }
        }
    }
}

static void new_etorp(int x, int y, int xs, int ys)
{
    struct torp *t;

    t = malloc(sizeof(struct torp));
    t->next = first_etorp;
    if(t->next)
        t->next->prev = t;
    t->prev = NULL;
    first_etorp = t;

    t->x = x;
    t->y = y;
    t->xspeed = xs;
    t->yspeed = ys;
    t->alive = 1;
    t->frame = 0;
    numetorps++;
}

static void do_enter(int i)
{
    int diffx, diffy;
    int tc;

    if(aliens[i].enterdelay) {
        aliens[i].enterdelay--;
        return;
    }

    if(aliens[i].path >= 0) {
        aliens[i].x += moves[aliens[i].dir][0] + metaLevel * moves[aliens[i].dir][0]/2;
        aliens[i].y += moves[aliens[i].dir][1] + metaLevel * moves[aliens[i].dir][1]/2;

        aliens[i].steer--;
        if(aliens[i].steer <= 0) {
            aliens[i].path_pos++;
            enter_path_dir(aliens[i].path, aliens[i].path_pos, &aliens[i].dir, &aliens[i].steer);
			if(metaLevel > 1)
				aliens[i].steer = aliens[i].steer / (1 + ((metaLevel - 1) * .5));
			/*aliens[i].steer -= ((metaLevel - 1) * (aliens[i].steer / 3));*/

            if(aliens[i].dir < 0) {
                aliens[i].path = -1;
            }
        }
        tc = TORPCHANCE - level/2 - weapon*5;
        if(tc < 35) tc = 35;
        if(numetorps < maxetorps && (!(random()%tc))) {
            int xs, ys;

            /* could aim better, not sure it should! */

            if(aliens[i].x > plx + 200) {
                xs = -3;
            } else if(aliens[i].x > plx + 100) {
                xs = -2;
            } else if(aliens[i].x < plx - 200) {
                xs = 3;
            } else if(aliens[i].x < plx - 100) {
                xs = 2;
            } else {
                xs = 0;
            }
            ys = (ETORPSPEED+level/5) - ABS(xs);
            new_etorp(aliens[i].x, aliens[i].y, xs, ys);
        }
    } else {
        diffx = ABS(convoy_x_pos(i) - aliens[i].x);
        diffy = ABS(convoy_y_pos(i) - aliens[i].y);
        if(diffy< 4 + (metaLevel * 2)) {
            aliens[i].y = convoy_y_pos(i);
            if(diffx < 4 + (metaLevel * 2)) {
                aliens[i].x = convoy_x_pos(i);
                aliens[i].dir = -1;
                aliens[i].entering = 0;
                return;
            }
            aliens[i].dir = 0;
            if(convoy_x_pos(i) > aliens[i].x)
                aliens[i].dir = 4;
            else
                aliens[i].dir = 12;
        } else {
			if(convoy_y_pos(i) < aliens[i].y) {
				if(diffx < 4 + (metaLevel * 2)) {
					aliens[i].x = convoy_x_pos(i);
					aliens[i].dir = 0;
				} else {
					if(convoy_x_pos(i) > aliens[i].x)
						aliens[i].dir = 2;
					else
						aliens[i].dir = 14;
				}
			} else {
				if(diffx < 4 + (metaLevel * 2)) {
					aliens[i].x = convoy_x_pos(i);
					aliens[i].dir = 8;
				} else {
					if(convoy_x_pos(i) > aliens[i].x)
						aliens[i].dir = 6;
					else
						aliens[i].dir = 10;
				}
			}
        }
        aliens[i].x += moves[aliens[i].dir][0] + metaLevel * moves[aliens[i].dir][0]/2;
        aliens[i].y += moves[aliens[i].dir][1] + metaLevel * moves[aliens[i].dir][1]/2;
    }
}

static void do_aliens(void)
{
    int i, j;
    int tc;

    if(gstate != PAUSED) {
        convoyx += convoymove;
        if(convoyx <= 0) {
            convoyx=0;
            convoymove = -convoymove;
        } else if(convoyx >= (winwidth-180)) {
            convoyx = winwidth - 180;
            convoymove = -convoymove;
        }
    }

    /* checking how many alien ships are still alive, entering, attacking */
    livecount=0; attacking = 0;
    for(i=0, livecount=0, entering=0; i < MAXALIENS; i++) {
        if(aliens[i].alive) {
            livecount++;
            if(aliens[i].dir >= 0 && aliens[i].escorting < 0 && !(aliens[i].entering))
                attacking++;
            if(aliens[i].entering)
                entering++;
        }
    }

    for(i=0; i < MAXALIENS; i++) {
        if(aliens[i].alive) {
            if (gstate != PAUSED) {
                if(aliens[i].escorting >= 0) {
                    do_escort(i);
                }

                if(aliens[i].entering) {
                    do_enter(i);
                } else if(aliens[i].dir == -1) {
                    do_convoy(i);
                } else if(aliens[i].dir == -2) {
                    aliens[i].x += convoymove;
                    aliens[i].y+=2;
                    if(aliens[i].y >= 20 + (20*(i/10))) {
                        aliens[i].y = 20 + (20*(i/10));
                        aliens[i].dir = -1;
                    }
                } else {
                    aliens[i].x += moves[aliens[i].dir][0];
                    aliens[i].y += moves[aliens[i].dir][1];

		    //warp around the screen borders
		    /*
                    if(aliens[i].x > winwidth+20)
                        aliens[i].x = -20;
                    else if(aliens[i].x < -20)
                        aliens[i].x = winwidth+20;
			*/

                    if(aliens[i].y > winheight) {
                        aliens[i].x = 20 * (i - 10 * (i/10)) + convoyx + convoymove;
                        aliens[i].y = -30;
                        aliens[i].dir = -2;
                        aliens[i].path = -1;
                        aliens[i].steer = 2;
                        aliens[i].escorting = -1;
                        attacking--;
                        if(i < 10) {
                            for(j=i+9;j<i+12;j++)
                                aliens[j].escorting = -1;
                        }
                    } else if(aliens[i].y < 0) {
                        aliens[i].dir = 8;
                    }

                    if(aliens[i].escorting < 0) {
                        aliens[i].steer--;
                        if(aliens[i].steer <= 0) {
                            if(aliens[i].path >= 0) {
                                int lastdir=aliens[i].dir;

                                aliens[i].path_pos++;
                                path_dir(aliens[i].path, aliens[i].path_pos, &aliens[i].dir, &aliens[i].steer);
                                if(aliens[i].dir < 0) {
                                    aliens[i].dir = lastdir;
                                newpath:
                                    switch(random()%8) {
                                    case 0:
                                        start_path(P_LOOP, &aliens[i]);
                                        break;
                                    case 1:
                                        start_path(P_SWOOP1, &aliens[i]);
                                        break;
                                    case 2:
                                        start_path(P_SWOOP2, &aliens[i]);
                                        break;
                                    case 3:
                                        start_path(P_ZIGZAG, &aliens[i]);
                                        break;
                                    case 4:
                                        start_path(P_LOOP2, &aliens[i]);
                                        break;
                                    case 5:
                                        start_path(P_SPIN, &aliens[i]);
                                        break;
                                    case 6:
                                        start_path(P_LEFTDIAG, &aliens[i]);
                                        break;
                                    case 7:
                                        start_path(P_RIGHTDIAG, &aliens[i]);
                                        break;
                                    default:
                                        aliens[i].steer = TURNSPEED;
                                        aliens[i].path = -1;
                                    }
                                    if((aliens[i].path < 0) || (aliens[i].steer < 0)) {
                                        goto newpath;
                                    }
                                }
                            } else {
                                if(random()%2) {
                                    aliens[i].dir++;
                                    if(aliens[i].dir > 15)
                                        aliens[i].dir = 0;
                                } else {
                                    aliens[i].dir--;
                                    if(aliens[i].dir < 0)
                                        aliens[i].dir = 15;
                                }
                                aliens[i].steer = TURNSPEED;
                            }
                        }
                    }
                    tc = TORPCHANCE - level/2 - weapon*5;
                    if(tc < 35) tc = 35;

                    if(numetorps < maxetorps && (!(random()%tc))) {
                        int xs, ys;

                        /* could aim better, not sure it should! */

                        if(aliens[i].x > plx + 200) {
                            xs = -3;
                        } else if(aliens[i].x > plx + 100) {
                            xs = -2;
                        } else if(aliens[i].x < plx - 200) {
                            xs = 3;
                        } else if(aliens[i].x < plx - 100) {
                            xs = 2;
                        } else {
                            xs = 0;
                        }
                        ys = (ETORPSPEED+level/5) - ABS(xs);
                        new_etorp(aliens[i].x, aliens[i].y, xs, ys);
                    }
                }
                S_DrawImage(aliens[i].x-(aliens[i].shape->width/2),
                            aliens[i].y-(aliens[i].shape->height/2),
                            aliens[i].dir < 0 ? 0 : aliens[i].dir,
							aliens[i].shape);
            } else {
				/* paused */
                S_DrawImage(aliens[i].x-(aliens[i].shape->width/2),
                            aliens[i].y-(aliens[i].shape->height/2),
                            aliens[i].dir < 0 ? 0 : aliens[i].dir,
							aliens[i].shape);
            }
        }
    }

    /* do warp sequence 
     * TODO: check if player is dead and ship is not visible, 
     * because there is no point going to warp then
     */
    if ((livecount == 0) && (gstate != PAUSED) && (pldead==0)) {
        starspeed++;
        if(starspeed == 2)
            play_sound(SND_WARP);
        if(starspeed >= 120) {
            starspeed = -20;
        } else if(starspeed == 1) {
            init_aliens(++level);
            gotlemon = 0;
            starspeed = 1;
            numtorps=0;
        }
    }
}

/*------------------player----------------*/
static void init_player(void)
{
    int i;
    for(i=0;i<MAXTORPS;i++)
        torps[i].alive=0;
}

static void new_torp(int x, int y, int xs, int ys)
{
    int i;

    for(i=0;i<maxtorps;i++) {
        if(!torps[i].alive) {
            torps[i].x = x;
            torps[i].y = y;
            torps[i].alive = 1;
            torps[i].xspeed = xs;
            torps[i].yspeed = ys;
            numtorps++;
            play_sound(SND_FIRETORP);
            return;
        }
    }
}

static void do_torps(void)
{
    int i,j,k, ne;

    for(i=0;i<MAXTORPS;i++) {
        if(torps[i].alive) {
            if(gstate != PAUSED) {
                torps[i].y += torps[i].yspeed;
                torps[i].x += torps[i].xspeed;
                torps[i].frame++;
                for(j=0;j<MAXALIENS;j++) {
                    if(aliens[j].alive && !aliens[j].dying &&
                       (ABS(torps[i].x - aliens[j].x) < 8) &&
                       ((ABS(torps[i].y - aliens[j].y) < 8) ||
                        (ABS((torps[i].y + torps[i].yspeed/2)-aliens[j].y) < 8))) {
                        aliens[j].dying = 1;
                        if(aliens[j].dir >= 0)
                            attacking--;

                        torps[i].alive=0;
                        numtorps--;
                        if(j >= 10) {
                            if(aliens[j].dir < 0)
                                score += 50;
                            else {
                                score += (6-(j/10))*100;
                                if(!(random()%(gotlemon ? 3 : PRIZECHANCE)))
                                    new_prize(aliens[j].x, aliens[j].y);
                            }
                            new_explosion(aliens[j].x, aliens[j].y, 0);
                        } else {
                            if(aliens[j].dir < 0)
                                score += 200;
                            else {
                                ne=0; /* count how many escorts */
                                for(k = j+9;k < j+12; k++) {
                                    if(aliens[k].escorting == j)
                                        ne++;
                                }
                                score_flagship(aliens[j].x, aliens[j].y, ne);
                            }
                            new_explosion(aliens[j].x, aliens[j].y, 1);
                        }
                        goto skip;
                    }
                }
                if(torps[i].y < -torps[i].yspeed ||
                   torps[i].x < ABS(torps[i].xspeed) ||
                   torps[i].x > winwidth-ABS(torps[i].xspeed)) {
                    torps[i].alive = 0;
                    numtorps--;
                } else
                    S_DrawImage(torps[i].x-(playerTorp->width/2),
                                torps[i].y-(playerTorp->height/2),
                                torps[i].frame, playerTorp);
            skip: ;
            } else {
				/* paused */
                S_DrawImage(torps[i].x-(playerTorp->width/2),
                            torps[i].y-(playerTorp->height/2),
                            torps[i].frame, playerTorp);
            }
        }
    }
}

static void do_etorps(void)
{
    struct torp *t = first_etorp, *nextt;

    while(t) {
        nextt=t->next;
        if(t->alive) {
            if(gstate != PAUSED) {
                t->y+=t->yspeed;
                t->x+=t->xspeed;
                t->frame++;
                if(t->y > winheight || t->x < 0 || t->x > winwidth) { //this torpedo is out of bounds and gone
                    if(t->next)
                        t->next->prev = t->prev;
                    if(t->prev)
                        t->prev->next = t->next;
                    if(t == first_etorp)
                        first_etorp = t->next;
                    free(t);
                    numetorps--;
                } else //a hit - ship has no shields, is not yet dead and is not flashing
			if(!pldead && !plflash && !plshield &&
                          (ABS(t->x - plx) < 8) &&
                          (ABS(t->y - (winheight - (int)playerShip->height / 2)) < 8)) { /* DEAD! */
	                    pldead = 1;
        	            new_explosion(plx, winheight - playerShip->height/2, 2);
                } else { //torpedo is moving
                    S_DrawImage(t->x-(enemyTorp->width/2),
                                t->y-(enemyTorp->height/2),
                                t->frame, enemyTorp);
                }
            }  
	    else { //PAUSED - torpedoes are redrawn on screen but no movement
                S_DrawImage(t->x-(enemyTorp->width/2),
                            t->y-(enemyTorp->height/2),
                            t->frame, enemyTorp);
            }
        }
        t = nextt;
    }
}

static void start_game(void)
{
	gstate = PLAYING;
	maxtorps = MINTORPS;
	weapon = 0;
	movespeed = MINSPEED;
	ships=2;
	shieldsleft = 0;
	level=startLevel;  /* change made here */
	init_aliens(level);
	gotlemon = 0;
	pldead = 0;
	score = 0;
	nextBonus = 20000;
	plx = winwidth/2;
	mx = plx;
}

static void do_player(void)
{
    /* 
     * this variable keeps its value. it is the timer value that
     * is used to count down the time between two shots 
     * */ 
    static int torpok;
    SDL_Event event;
    static int keys = 0;
    int but=0; //fire button is pressed 

    /* 
     * here the two game states INTRO and GETTING_NAME are handled 
     * */
    if (gstate == INTRO) {
        while(SDL_PollEvent(&event)) {

            switch(event.type) {
		case SDL_QUIT:
			xgal_exit(0);
			break;


            	case SDL_KEYDOWN:
                	switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
		                case SDLK_q:
                		    xgal_exit(0);
		                    break;

				case SDLK_SPACE:
					start_game();
					break;

				case SDLK_RETURN:
					if (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
					toggle_fullscreen();
					break;

		                case SDLK_s:       /* toggle sound on the title screen */
                			playSounds = !playSounds;
					return;     /* this key must not start the game */
					break;

				case SDLK_RIGHT:
					title_page_next();
					break;
	
				case SDLK_LEFT:
					title_page_prev();
					break;	

				default:
	                	return;     /* unhandled key must not cause any action */
				break;
                	}
			break;

            	default:
			break;
            }
        } // while
        return;
    } //if
	else if (gstate == GETTING_NAME) {
	        while(SDL_PollEvent(&event)) {
            		switch(event.type) {
				case SDL_QUIT:
					xgal_exit(0);
					break;

            			case SDL_KEYDOWN:
			                if (score_key(event.key.keysym.sym))
                    			continue;
			}
		}
		return;
	}
	
 /* 
  * here the game states PLAYING and PAUSED are handled
  */
    while(SDL_PollEvent(&event)) {

        switch(event.type) {
		case SDL_QUIT:
			xgal_exit(0);
			break;

        	case SDL_KEYUP:
            		switch(event.key.keysym.sym) {
				case SDLK_RIGHT:
                			keys &= ~RIGHTKEY;
		                	break;

			        case SDLK_LEFT:
        	        		keys &= ~LEFTKEY;
                			break;

				case SDLK_SPACE:
		                	keys &= ~FIREKEY;
					break;

				default:
					break;
            		}
            		break;

        	case SDL_KEYDOWN:
            		switch(event.key.keysym.sym) {
            			case SDLK_RIGHT:
					keys |= RIGHTKEY;
                			break;

			        case SDLK_LEFT:
			                keys |= LEFTKEY;
			                break;

            			case SDLK_SPACE:
			                keys |= FIREKEY;
					break;

			        case SDLK_g:
                			if(!pldead  && (gstate != PAUSED)) {
			                    new_explosion(plx, winheight - ((playerShip->height)/2), 2);
                    			    ships = 0;
                    			    pldead = 1;
                			}
					break;

            			case SDLK_q:
                			xgal_exit(0);
				break;

            			case SDLK_p:
					if (gstate == PAUSED)
						gstate = PLAYING;
					else
						gstate = PAUSED;
					break;

            			case SDLK_s:
			                playSounds = !playSounds;
					break;

#ifdef IM_A_BIG_FAT_CHEATER
            			case SDLK_i:
                			if(plflash >= 0)
                    				plflash = -2;
                			else
                    				plflash = 0;
					break;

            			case SDLK_l: {
					int i;
					for(i=0;i<MAXALIENS;i++)
						aliens[i].alive=0;
						if(starspeed != 1)
						level++;
					}
					break;

            			case SDLK_c:
			                score+= BONUSSHIPSCORE;
					break;

			        case SDLK_h:
			                plshield = SHIELDTIME;
					play_sound(SND_SHIELD);
					break;

            			case SDLK_w:
                			weapon++;
					if(weapon == NUMWEAPONS)
						weapon=0;
					break;

            			case SDLK_t:
                			maxtorps++;
					if(maxtorps > MAXTORPS)
						maxtorps = MINTORPS;
					break;

#endif /* IM_A_BIG_FAT_CHEATER */

				case SDLK_RETURN:
					if (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
						toggle_fullscreen();
					break;

            			default:
	                		break;
            	} //keydown
			break;

        } //eventtype
    } //while

/*
 * The inputs are now available so now comes the actual game logic
 * The game state is calculated and the game data is updated
 */
    if (gstate != PAUSED) {
	torpok--; //count down torpedo timer

	// unclear what mx is doing
	if(keys & LEFTKEY)
		mx = 0;
	else 
		if(keys & RIGHTKEY)
			mx = winwidth;
		else
			mx = plx;

	if(keys & FIREKEY)
		but = 1;
	else
		but = 0;

	// the player has died
        if(pldead) {
            pldead++;
	    /* the game waits 100 frames between the explosion and either the next game state 
	     * or the continuation of the game with the next ship*/
            if(pldead >= 100) {
                if(ships<=0) {
                    if(check_score(score)) {
						gstate = GETTING_NAME;
                    } else {
						gstate = INTRO;
					}
                } else {
                    ships--;
                    maxtorps = MINTORPS;
                    weapon = 0;
                    movespeed = MINSPEED;
                    pldead = 0;
                    plflash = 50;
                    plx = winwidth/2;
                }
            }
            return;
        }
	// the player fires, the torpedo timer is 0 and we are not in warp mode 
	// -> go and launch torpedo!
        if(but && torpok <= 0 && (starspeed == 1)) {
            switch(weapon) {
            case SINGLESHOT:
                if(numtorps < maxtorps)
                    new_torp(plx, winheight - playerShip->height, 0, -TORPSPEED);
				torpok = TORPDELAY;
                break;
            case DOUBLESHOT:
                if(numtorps < maxtorps-1) {
                    new_torp(plx-5, winheight - playerShip->height, 0, -TORPSPEED);
                    new_torp(plx+5, winheight - playerShip->height, 0, -TORPSPEED);
					torpok = TORPDELAY;
                }
                break;
            case TRIPLESHOT:
                if(numtorps < maxtorps-2) {
                    new_torp(plx-5, winheight - playerShip->height, -2, 1-TORPSPEED);
                    new_torp(plx,   winheight - playerShip->height, 0,   -TORPSPEED);
                    new_torp(plx+5, winheight - playerShip->height, 2, 1-TORPSPEED);
					torpok = TORPDELAY;
                }
                break;
			}
		}


        if(!but)
            torpok=0;

	//unclear why we need to divide by movespeed here?
        if((mx/movespeed) > (plx/movespeed))
            plx+=movespeed;
        else if((mx/movespeed) < (plx/movespeed))
            plx-=movespeed;
	
	// boundary check
        if(plx < playerShip->width/2)
            plx=playerShip->width/2;
        if(plx> winwidth - playerShip->width/2)
            plx=winwidth - playerShip->width/2;

	//make ship flash
        if(plflash > 0)
            plflash--;
        if(!(plflash % 2))
            S_DrawImage(plx-(playerShip->width/2), winheight - playerShip->height, counter, playerShip);

	//draw shields 
	if(plshield > 0)
            plshield--;
        if(plshield && ((plshield > SHIELDTIME/4) || plshield % 2)) {
            S_DrawImage(plx-(shieldImage->width/2), winheight - shieldImage->height - 3, 0, shieldImage);
        }

    } //not paused
    else if (!pldead) { /* paused - but ship is redrawn*/
        S_DrawImage(plx-(playerShip->width/2), winheight - playerShip->height, counter, playerShip);
    }
}

static int init_fonts(void)
{
	fnt_reg_green = SFont_InitFont(F_REG_GREEN);
	fnt_reg_cyan = SFont_InitFont(F_REG_CYAN);
	fnt_reg_yellow = SFont_InitFont(F_REG_YELLOW);
	fnt_reg_red = SFont_InitFont(F_REG_RED);
	fnt_reg_grey = SFont_InitFont(F_REG_GREY);
	fnt_big_red = SFont_InitFont(F_BIG_RED);

    if (!fnt_reg_green || !fnt_reg_cyan || !fnt_reg_yellow ||
		!fnt_reg_red || !fnt_reg_grey || !fnt_big_red)
		return 0;

	return 1;
}

int main(int argc, char *argv[])
{
    for(int ac = 1; ac < argc; ac++) {
        if(*argv[ac] == '-') {
            int w, h;
            if(strcmp(argv[ac], "-scores") == 0) {
                print_scores();
                exit(0);
            } else if (strcmp(argv[ac], "-nosound") == 0) {
                playSounds = 0;
				/* '-level' option defined here */
            } else if (strcmp(argv[ac], "-level") == 0 && (ac+1 < argc)
                       && atoi(argv[ac+1]) >= 1)
            {
                int nlev;
                nlev = atoi(argv[ac+1]);
                if (nlev > 15 ) nlev = 15;
                startLevel = nlev;
                ac++;
            } else if (strcmp(argv[ac], "-nostars") == 0) {
                wantStars = 0;
            } else if (strcmp(argv[ac], "-window") == 0) {
                fullscreen = 0;
            } else if ((strcmp(argv[ac], "-winsize") == 0) && (++ac < argc) &&
                       (sscanf(argv[ac], "%dx%d", &w, &h) == 2)) {
                winwidth  = w;
                winheight = h;
            } else {
                print_usage();
                exit(0);
            }
        } else {
            print_usage();
            exit(0);
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen %d\n", fullscreen);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL version %d\n", SDL_COMPILEDVERSION);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_VERSION_ATLEAST(1,3,0) %d\n", SDL_VERSION_ATLEAST(1,3,0));
    
    
    S_Initialize(fullscreen); // init video
    init_sound(); //init audio
    
    if (!loadAllImages()) {
		fprintf(stderr, "Cannot load one or more images\n");
		return -1;
	}

    if (!init_fonts()) {
		fprintf(stderr, "Invalid font\n");
		return -1;
    }

    playerShip = getImage(I_PLAYER1);
    playerTorp = getImage(I_MTORP);
    enemyTorp = getImage(I_ETORP);
    shieldImage = getImage(I_SHIELD);

    level=startLevel;   /* change made here */

    load_scores();
    init_titles();
    init_player();
    init_stars();
    init_explosions();
    init_score();
    init_prizes();
    init_aliens(level);
    init_framerate();
    init_joystick();

    ships = 2;
    nextBonus = 20000;
    gstate = INTRO;
    play_music();
    plx = winwidth/2;

    while(1) {
        counter++;

	S_ClearScreen();

	/* For the benefit of unbuffered mode, the most important things are
	 * erased/redrawn closest together so they spend the least time blanked.
	 * player, aliens and etorps are most important for game play.
	 * pause, title and name are important in their modes and aren't done
	 * otherwise.
	 *
	 * The title, name, pause and score "extra ship" want to overlay
	 * everything else drawn, so they come last.
	 */
        undo_explosions();
        undo_prizes();
        undo_aliens();

        do_etorps();
        do_player();
        do_aliens();
        do_torps();
        do_prizes();
        do_explosions();
		do_stars();

		switch(gstate) {
		case INTRO:
			do_title();
			break;

		case GETTING_NAME:
			do_name();
			draw_score();
			break;

		case PAUSED:
			do_pause();
			/* fall through */

		case PLAYING:
			do_score();
		}

        /* Update the display */
		S_UpdateDisplay();

        do_framerate();
    }
    return (0);
}
