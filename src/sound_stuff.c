/* sound_stuff.c
**
**    code for sound sample data
**    Copyright (C) 2001  Florian Berger
**    Email: harpin_floh@yahoo.de, florian.berger@jk.uni-linz.ac.at
**
**    Updated Version foobillard++ started at 12/2010
**    Copyright (C) 2010 - 2013 Holger Schaekel (foobillardplus@go4more.de)
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License Version 2 as
**    published by the Free Software Foundation;
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/
#ifdef USE_SOUND
#ifdef USE_WIN
   #include <windows.h>
#endif
#include "options.h"
#include "sys_stuff.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>

/***********************************************************************/

// Placeholder for max. 500 Songs (that's hopefully enough, I think)

char musicfile[500][255];
char playfile[300];
int shufflesong[500];
int songs = 0;
int actualsong = 0;
int give_up = 0;
int music_to_play = 0;
int music_finished = 1;

// Mix_Music actually holds the music information.

Mix_Music *music = NULL;

// Mix_Chunk actually holds the noise information.

Mix_Chunk *ball_hole = NULL;
Mix_Chunk *wave_applause = NULL;
Mix_Chunk *wave_shuffle = NULL;
Mix_Chunk *wave_error = NULL;
Mix_Chunk *wave_oneball = NULL;
Mix_Chunk *wave_outball = NULL;
Mix_Chunk *wave_ooh = NULL;
Mix_Chunk *cue_sound = NULL;
Mix_Chunk *wall_sound = NULL;
Mix_Chunk *ball_sound = NULL;

/***********************************************************************/

/* Internal Routines */

void init_sound(void);
void exit_sound(void);
Mix_Chunk* loadsound (char *filename);
void PlayNoise(Mix_Chunk *chunkdata, int volume);

/***********************************************************************
 *              Check the given string extension mp3 or ogg            *
 ***********************************************************************/

int strsound ( char s1[] )
{
   int i = 0;
   char s[10];
   if(strlen(s1) > 4) {
      strcpy(s,&s1[strlen(s1)-4]);
      while (s[i]) {
         s[i] = toupper(s[i]);
         ++i;
      }
 	    if(strcmp(s,".MP3") || strcmp(s,".OGG")) {
         return(1);
 	    }
   }
   return(0);
}

/***********************************************************************
 *                music finished callback function                     *
 ***********************************************************************/

void musicFinished()
{
    music_finished = 1;
}

/***********************************************************************
 *           Skip a playing song, so the next one can play             *
 ***********************************************************************/

void SkipSong (void) {
	   if(options_use_music) {
	   	 if (!music_to_play || music_finished) {
	   		  return;
	   	 }
	   	 if(Mix_PlayingMusic()) {
	   	   Mix_FadeOutMusic(2000);
	   	 }
	   }
}

/***********************************************************************
 *              Play the next song if no music is playing              *
 ***********************************************************************/

void PlayNextSong (void) {
	   if(options_use_music) {
	   	 if (!music_to_play || !music_finished) {
	   		  return;
	   	 }
	   	 if(give_up >= songs) {
	   	 	 give_up = 0;
	   	 	 options_use_music = 0;
	   	 	 error_print("background-music failed too many times. Give up.",NULL);
	   	 	 return;
	   	 }
	     if(music) {
	   	   Mix_FreeMusic(music);
	   	   music = NULL;
	     }
	     if(++actualsong >= songs) {
	   	   actualsong = 0;
	     }
	     sprintf(playfile,"music/%s",musicfile[shufflesong[actualsong]]);
      if(!(music = Mix_LoadMUS(playfile))) {
         //fprintf(stderr,"background-music %s failed. Goto next one.\n",musicfile[shufflesong[actualsong]]);
         give_up ++;
         PlayNextSong();
      } else {
      	  fprintf(stderr,"Now playing %s.\n",musicfile[shufflesong[actualsong]]);
      	  Mix_PlayMusic(music,0);
         Mix_VolumeMusic(options_mus_volume);
         give_up = 0;
         //fprintf(stderr,"music is%s playing.\n", Mix_PlayingMusic()?"":" not");
         if(Mix_PlayingMusic()) {
         	 //fprintf(stderr,"Wait for next song\n");
           music_finished = 0;
         }
      }
	   }
}

/***********************************************************************
 *                             load sounds                             *
 ***********************************************************************/

Mix_Chunk* loadsound (char *filename) {

	   Mix_Chunk *chunkname;
    if(!(chunkname = Mix_LoadWAV(filename))) {
        error_print("Initializing %s failed. No sound in game.",filename);
 	    options_use_sound=0;
    }
    return (chunkname);
}

/***********************************************************************
 *                       Initialize the sound system                   *
 ***********************************************************************/

void init_sound(void)
{
    DIR * d;
    struct dirent * dp;
    int i,j,k,dummy;

    if(Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
      error_print("Unable to open audio!",NULL);
      options_use_sound=0;
      options_use_music=0;
    } else {
    	 Mix_AllocateChannels(50); // max. 50 Channels
    	 Mix_Volume(-1,MIX_MAX_VOLUME);

    	 // Extended Init for Version higher then SDL 1.2.10
#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
    	 // load support for the MP3, OGG music formats
    	 int flags=MIX_INIT_OGG | MIX_INIT_MP3;
    	 int initted=Mix_Init(flags);
    	 if((initted&flags) != flags) {
    	     fprintf(stderr,"Mix_Init: Failed to init both ogg and mp3 support!\nCheck only for ogg.\n");
    	     fprintf(stderr,"Mix_Init: %s\n", Mix_GetError());
    	     flags=MIX_INIT_OGG; // check only for ogg
    	     initted=Mix_Init(flags);
    	     if((initted&flags) != flags) {
     	       fprintf(stderr,"Mix_Init: Failed to init required ogg support!\n");
     	       fprintf(stderr,"Mix_Init: %s\n", Mix_GetError());
    	       options_use_music=0;
    	     }
    	 }
#endif
   	  /* Actually loads up the sounds */
    	 ball_hole = loadsound ("ballinhole.wav");
    	 wave_applause = loadsound ("applause.wav");
    	 wave_shuffle = loadsound ("shuffleballs.wav");
    	 wave_error = loadsound ("error.wav");
    	 wave_oneball = loadsound ("oneballontable.wav");
    	 wave_outball = loadsound ("balloutoftable.wav");
    	 wave_ooh = loadsound ("ooh.wav");
    	 ball_sound = loadsound ("ball.wav");
    	 wall_sound = loadsound ("wall.wav");
    	 cue_sound = loadsound ("cue.wav");

   	  /* Actually loads up the music */
#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
    	 if((initted&flags) == flags) {
#endif
    	 if((d = opendir("music"))) {
    	 	 i = 0;
   	    while ((dp = readdir(d))!= NULL && i <= 500) {
   	    	 if(strsound(dp->d_name)) {
   	    	 	  strcpy(musicfile[i++],dp->d_name);
   		        //fprintf(stderr,"%s\n", musicfile[i-1]);
   	    	 }
   	    }
   	    closedir(d);
   	    if(i>0) { //music to play
          if(i == 1) { // only one song, so play forever
          	sprintf(playfile,"music/%s",musicfile[0]);
          	if((music = Mix_LoadMUS(playfile))) {
          		  Mix_FadeInMusic(music,-1,4000);
          		  Mix_VolumeMusic(options_mus_volume);
          	}
          } else {
          	songs = i;
          	if(i > 499) {
          		  fprintf(stderr,"Max. 500 Songs are playable. Only the first 500 Songs are considered.\n");
          		  songs = 500;
          	}
          	// shuffle the playing-list
          	for (i=0;i<songs;i++) {
          		 shufflesong[i] = i;
          	}
          	for (k = 0; k < songs - 1; k++) {
          	  j = k + rand() / (RAND_MAX / (songs - k) + 1);
          	  dummy = shufflesong[j];
          	  shufflesong[j] = shufflesong[k];
          	  shufflesong[k] = dummy;
          	}
          	//for(i=0;i<songs;i++) {	fprintf(stderr,"%i, ",shufflesong[i]);	}
          	music_to_play = 1;
           PlayNextSong();
           Mix_HookMusicFinished(musicFinished);
          }
   	    } else {
   	    	 options_use_music=0;
   	    }
    	 } else {
           error_print("Initializing background-music failed. No background-music in game",NULL);
           options_use_music=0;
    	 }
#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
    	 }
#endif

    }
}

/***********************************************************************
 *                    plays sound with volume set                      *
 ***********************************************************************/
void PlayNoise(Mix_Chunk *chunkdata, int volume)
{
     static int channel = -1;
	   if(options_use_sound) {
	   	 if (chunkdata != NULL) {
	   	   //fprintf(stderr,"Volume: %i\n",volume);
	       Mix_VolumeChunk(chunkdata, volume);
	       channel = Mix_PlayChannel(-1, chunkdata, 0);
         if (channel < 0) {
           //fprintf(stderr,"Not enough mix channels");
         }
	   	 }
	   }
}


/***********************************************************************
 *                      close the sound system                         *
 ***********************************************************************/
void exit_sound(void)
{
    Mix_CloseAudio();
/* ### TODO ### really needed, if mixer closes ??
#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
	   Mix_Quit();
#endif
*/
}

#endif  /* #ifdef USE_SOUND */
