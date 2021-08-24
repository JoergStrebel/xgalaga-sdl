/*
 * XGalaga-SDL - a SDL port of XGalaga, clone of the game Galaga
 * Copyright (c) 1995-1998 Joe Rumsey (mrogre@mediaone.net)
 * Copyright (c) 1994 Sujal M. Patel (smpatel@wam.umd.edu)
 * Copyright (c) 2000 Andy Tarkinson <atark@thepipeline.net>
 * Copyright (c) 2010 Frank Zago
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

#ifdef NOSOUND

void init_sound () {}
void sound_exit(void) {}
void play_sound (int sound) {}
void play_music (void) {}
void stop_music (void) {}

#else
#include <SDL_mixer.h>

#include "xgalaga.h"

static int audioOK;

static const char *FILENAME[] = {
	"explode.wav",
	"firetorp.wav",
	"shield.wav",
	"torphit.wav",
	"explode_big.wav",
	"ddloo.wav",
	"warp.wav",
	"smart.wav",
};

static const char *MUSICNAME = "sawsquarenoise_boss_theme.wav";

#define NUM_SOUNDS  (sizeof(FILENAME)/sizeof(char*))

static Mix_Chunk *sounds[NUM_SOUNDS];
static Mix_Music *title_music;

void init_sound (void)
{
	unsigned int i;
	char filename[MAXFILENAME];
    char musicname[MAXFILENAME];

    int initted=Mix_Init(0); //TODO: Use the return value and check for capabilities
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mixer Init capabilities %d\n", initted);

    /* Open the audio device */
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0 ) {
        fprintf(stderr,
				"Warning: Couldn't init mixer \n- Reason: %s\n",
				SDL_GetError());
		return;
    }
    
    /* Load the sound effects */
	for (i=0; i<NUM_SOUNDS; i++) {
		snprintf(filename, MAXFILENAME, "%s/sounds/%s", DATADIR, FILENAME[i]);
		filename[MAXFILENAME-1] = '\0';
		sounds[i] = Mix_LoadWAV(filename);

		if (!sounds[i])
			fprintf(stderr, "Warning: Couldn't load sound %s\n", filename);
	}
	
	/* Load the music */
	snprintf(musicname, MAXFILENAME, "%s/sounds/%s", DATADIR, MUSICNAME);
	musicname[MAXFILENAME-1] = '\0';
	title_music = Mix_LoadMUS(musicname);
	if (!title_music)
		fprintf(stderr, "Warning: Couldn't load music %s\n", musicname);
	Mix_VolumeMusic(MUSIC_VOL);

	audioOK = 1;
}

void sound_exit(void)
{
	unsigned int i;

	if (audioOK) {
		Mix_CloseAudio();
		for (i=0;i<NUM_SOUNDS;i++) {
			Mix_FreeChunk(sounds[i]);
		}
		audioOK = 0;
	}
}

void play_sound (int sound)
{
	if (audioOK && playSounds)
		Mix_PlayChannel(sound, sounds[sound], 0);
}

void play_music (void)
{
	if (audioOK ) {
		int initmusic=Mix_PlayMusic(title_music, -1);
		if(initmusic==-1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Mix_PlayMusic: %s\n", Mix_GetError());
		 	// well, there's no music, but most games don't break without music...
		}

	} else {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "No music output possible");
	}	
}

void stop_music (void)
{
	Mix_HaltMusic();
}	

#endif	/* NOSOUND */
