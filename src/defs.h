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

/* Area for setting flags to enable changes by Bryce and Neelix */

#define ORIGINAL_XGALAGA
/*#define IM_A_BIG_FAT_CHEATER*/

#define FPS 30
#define FPSFACTOR 30.0f/FPS

#define MAXTORPS 10
#define MINTORPS 6
#define TORPSPEED 12*FPSFACTOR

#define ETORPSPEED 8*FPSFACTOR

#define MINSPEED 5*FPSFACTOR
#define MAXSPEED 8*FPSFACTOR

#define TORPDELAY 5*FPSFACTOR

/* Modified slightly to better fit on 800x600 in fullscreen mode */
#define WINTOPOV    0
#define WINBOTOV    0
#define WINWIDTH  400
#define WINHEIGHT 450

#define NUMSTARS 30

#define MAXALIENS 60

#define TURNSPEED 10*FPSFACTOR

/*#define UTIMER 33333*/

#define BONUSSHIPSCORE 50000

#define ABS(a)			/* abs(a) */ (((a) < 0) ? -(a) : (a))

#define LEFTKEY 1
#define RIGHTKEY 2
#define FIREKEY 4

#ifndef HAVE_RANDOM
# ifndef HAVE_LRAND48
#  define random() rand()
# else
#  define random() lrand48()
# endif
#endif

  #define NUMWEAPONS 3
#define SINGLESHOT 0
#define DOUBLESHOT 1
#define TRIPLESHOT 2
#define SPREADSHOT 3
#define MACHINEGUN 4

# define PRIZECHANCE 30

#define TORPCHANCE 60

# define SHIELDTIME 300*FPSFACTOR

#define ALIENSHAPES 17


#define MAXFILENAME 1024

#define SCORE_FILE_NAME ".xgalaga-sdl"
#define ENABLE_LOGGING 0

