/* sys_stuff.c
**
**    code for system-behaviour
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

#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <SDL.h>
#include <SDL_syswm.h>
#ifdef USE_WIN
  #include <windows.h>
  #include <shellapi.h>
#else
  #include <sys/stat.h>
#endif

#ifdef NETWORKING
  #include <SDL_net.h>
#endif
#include "sound_stuff.h"
#ifdef __APPLE__
 #include <OpenGL/OpenGL.h>
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <OpenGL/glext.h>
 #include <CoreFoundation/CoreFoundation.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glext.h>
#endif
#include "sys_stuff.h"
#include "billard3d.h"

/***********************************************************************/

static int fullscreen = 0;
static int keymodif =0;
static int vidmode_bpp=0;
static int sdl_on = 0;
static int check_SDL;           // check for mousebutton for manual from fullscreen
static int ignore = 0;          // SDL bug set videomode calls reshape event twice SDL 1.2.8 and > ?
static int vsync_available = 1; // assume vsync available by default

SDL_Window    *glWindow = NULL;
SDL_GLContext glContext = NULL;

/***************************************************
 *    replace a string (max. 2048 Bytes long)       *
 ***************************************************/

char *replace(char *st, char *orig, char *repl) {
  static char buffer[2048];
  char *ch;
  if (!(ch = strstr(st, orig)))
   return st;
  strncpy(buffer, st, ch-st);
  buffer[ch-st] = 0;
  sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
  return buffer;
  }

/***************************************************
 *          Split a string with delimeter          *
 ***************************************************/
struct split
{
  char *pointers[512];
  int count;
};

struct split split (char *in, char delim)
{
  struct split sp;
  sp.count = 1;
  sp.pointers[0] = in;

  while (*++in) {
    if (*in == delim) {
      *in = 0;
      sp.pointers[sp.count++] = in+1;
    }
  }
  return sp;
}

/***************************************************
 * Get dialog program (deal with path environment  *
 * return index to the program array               *
 * -1 = error                                      *
 * 0 = gnome zenity                                *
 * 1 = kde kdialog                                 *
 * 2 = X11 xmessage                                *
 ***************************************************/

int get_dialogprog(void) {
    char path[2048];
    char file[1024];
    int i;
    struct split sp;

    strcpy(path,getenv("PATH"));
    fprintf(stderr,"Check for Dialog-Program\n");
    if(path!=NULL) {
      // extract every path and check for zenity or kdialog
      sp = split(path, ':');
      for (i=0; i<sp.count; i++) {
        snprintf(file,sizeof(file),"%s/%s",sp.pointers[i],"zenity");
        if(file_exists(file)) {
            fprintf(stderr,"Dialog Program zenity found\n");
            return(0);
        }
        snprintf(file,sizeof(file),"%s/%s",sp.pointers[i],"kdialog");
        if(file_exists(file)) {
            fprintf(stderr,"Dialog Program kdialog found\n");
            return(1);
        }
      }
      // extract every path and check last for xmessage
      sp = split(path, ':');
      for (i=0; i<sp.count; i++) {
        snprintf(file,sizeof(file),"%s/%s",sp.pointers[i],"xmessage");
        if(file_exists(file)) {
            fprintf(stderr,"Only Dialog Program xmessage found\n");
            return(2);
        }
      }
    }
    return(-1);
}

/***************************************************
 *  print an error string (max. 2048 Bytes long)   *
 ***************************************************/

void error_print(char *error_message, char *error_extend) {

char message[2048];

  if(error_extend) {
    snprintf(message,sizeof(message),error_message,error_extend);
  } else {
    snprintf(message,sizeof(message),"%s",error_message);
  }
  fprintf(stderr,"%s\n",message); // print error to stderr every time
#ifdef USE_WIN
  MessageBox(0,message,"Foobillard++ Error",MB_OK);
#else
#ifdef __APPLE__
  // needs -framework CoreFoundation
  SInt32 nRes = 0;
  CFUserNotificationRef pDlg = NULL;
  const void* keys[] = { kCFUserNotificationAlertHeaderKey,
  kCFUserNotificationAlertMessageKey };
  const void* vals[] = {
  CFSTR("Foobillard++ Error"),
  CFStringCreateWithCString(NULL, message, kCFStringEncodingMacRoman)
  };
  if(!sys_get_fullscreen()) {
  // display a dialog window only if fullscreen is not active
  CFDictionaryRef dict = CFDictionaryCreate(0, keys, vals,
                  sizeof(keys)/sizeof(*keys),
                  &kCFTypeDictionaryKeyCallBacks,
                  &kCFTypeDictionaryValueCallBacks);
  pDlg = CFUserNotificationCreate(kCFAllocatorDefault, 0,
                       kCFUserNotificationPlainAlertLevel,
                       &nRes, dict);
  }
#else
  char *dialog_prog[] = {"zenity --error --text=\"%s\"","kdialog --error \"%s\"","xmessage -center %s"};
  char newmessage[2048];
  // display a dialog window only if fullscreen is not active
  if(dialog>=0 && dialog < 2 && !sys_get_fullscreen()) {
    snprintf(newmessage,sizeof(newmessage),dialog_prog[dialog],message);
    system(newmessage);
  }
#endif
#endif
}

/***********************************************************************
 *                    copy binary a file                               *
 ***********************************************************************/

int filecopy(char *filefrom,char *fileto)
{
  FILE *from, *to;
  char ch;

  /* open source file */
  if((from = fopen(filefrom, "rb"))==NULL) {
  	 fprintf(stderr,"Error: open source file (%s) for copy\n",filefrom);
    return(0);
  }

  /* open destination file */
  if((to = fopen(fileto, "wb"))==NULL) {
  	 fprintf(stderr,"Error: open destination file (%s) for copy\n",fileto);
    return(0);
  }

  /* copy the file */
  while(!feof(from)) {
    ch = fgetc(from);
    if(ferror(from)) {
      return(0);
    }
    if(!feof(from)) fputc(ch, to);
    if(ferror(to)) {
      return(0);
    }
  }

  fclose(from);

  if(fclose(to)==EOF) {
    return(0);
  }
  return(1);
}

/***********************************************************************
 *          Transparent mousecursor for touch-devices (WETAB)          *
 *    We don't use SDL_Showcursor which is not really on function      *
 *                          on some devices                            *
 ***********************************************************************/

#ifdef TOUCH
  static Uint8 cursorMask[16] = { 0 };
  static Uint8 cursorData[16] = { 0 };
  static SDL_Cursor* cursor;
#endif

/***********************************************************************
 *                         Exit SDL-Support                            *
 ***********************************************************************/

void sdl_exit()
{
    if (sdl_on) {
      /*
       * Quit SDL so we can release the fullscreen
       * mode and restore the previous video settings,
       * etc.
       */
       save_config(); //save the config (must!!!)
  #ifdef USE_SOUND
       exit_sound();
  #endif
  #ifdef NETWORKING
        SDLNet_Quit();  //in case of open Netgame
  #endif
        if(glContext)
            SDL_GL_DeleteContext(glContext);
        glContext = NULL;
        if(glWindow)
            SDL_DestroyWindow(glWindow);
        glWindow = NULL;

        SDL_Quit();
        sdl_on = 0;
    }
  }
/***********************************************************************
 *                      Exit with SDL-Support                          *
 ***********************************************************************/

void sys_exit( int code )
{
 	sdl_exit();
  exit( code );
}

/***********************************************************************
 *      Initialize SDL and make a SDL-Window / Fullscreen              *
 ***********************************************************************/

void sys_create_display(int width,int height,int _fullscreen)
{
  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;

  fullscreen = _fullscreen;

  if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0 ) {
    fprintf( stderr, "Video or Audio initialization failed: %s\n",

    SDL_GetError( ) );
    sys_exit(1);
  }

  sdl_on = 1 ; 

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if(options_antialiasing) {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, options_maxfsaa);
  } else {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
  }

  if(fullscreen == 1) {
#ifndef __APPLE__
        flags |= SDL_WINDOW_FULLSCREEN;
#else
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
        glWindow = SDL_CreateWindow("Foobillard++", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
  } else if (fullscreen == 0) { 
        glWindow = SDL_CreateWindow("Foobillard++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags | SDL_WINDOW_RESIZABLE);
  };

  glContext = SDL_GL_CreateContext(glWindow);
  SDL_GL_MakeCurrent(glWindow, glContext);
  try_set_vsync();

  SDL_SetWindowTitle(glWindow, "Foobillard++");

  SDL_GetWindowSize(glWindow, &win_width, &win_height);
  SDL_GL_GetDrawableSize(glWindow, &native_width, &native_height);
  ResizeWindow(win_width, win_height);

  glPolygonMode(GL_FRONT,GL_FILL);  // fill the front of the polygons
  glPolygonMode(GL_BACK,GL_LINE);   // only lines for back (better seeing on zooming)
  glCullFace(GL_BACK);              // Standards for rendering only front of textures
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
}

/***********************************************************************
 *                          Fullscreen active ?                        *
 ***********************************************************************/

int sys_get_fullscreen(void)
{
    return fullscreen;
}

/**************************************************************************
 *            Set a fullscreen(1) or window(0) window                     *
 * SDL_WM_ToggleFullScreen(screen) works only on X11 and there not stable *
 **************************************************************************/

void sys_fullscreen( int fullscr )
{
    int result;

    if ( fullscr!=0){
#ifndef __APPLE__
            result = SDL_SetWindowFullscreen(glWindow,SDL_WINDOW_FULLSCREEN);
#else
            result = SDL_SetWindowFullscreen(glWindow,SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
            if (result == 0) { /*printf ("switch to FULLSCREEN mode fine!\n");*/
                fullscreen = 1;
            } else {
                printf("Switch to FULLSCREEN mode failed: %s\n", SDL_GetError());
            }
    } else if( fullscr==0){
            result = SDL_SetWindowFullscreen(glWindow,0);
            if (result == 0) { /*printf ("switch to WINDOW mode fine!\n"*);*/
                fullscreen = 0;
            } else {
                printf("Switch to WINDOW mode failed: %s\n", SDL_GetError());
            }
    }
}

/***********************************************************************
 *          Toggle between Fullscreen and windowed mode                *
 ***********************************************************************/

void sys_toggle_fullscreen( void )
{
    if (fullscreen){
        sys_fullscreen(0);
    } else {
        sys_fullscreen(1);
    }
}

/***********************************************************************
 *        Update the keystroke modifiers (alt, strg etc.)              *
 ***********************************************************************/

static void update_key_modifiers(void)
{
  SDL_Keymod m;
  m=SDL_GetModState();
  keymodif=0 ;
  if (KMOD_CTRL  & m) keymodif |= KEY_MODIFIER_CTRL ;
  if (KMOD_SHIFT & m) keymodif |= KEY_MODIFIER_SHIFT ;
  if (KMOD_ALT   & m) keymodif |= KEY_MODIFIER_ALT ;
   
}

/***********************************************************************
 *                    handle for the mouse buttons                     *
 ***********************************************************************/

static void handle_button_event(SDL_MouseButtonEvent *e)
{
  MouseButtonEnum b ;
  MouseButtonState s ;

  update_key_modifiers() ;

  /* then the mouse buttons */
  switch(e->button) {
  case SDL_BUTTON_LEFT:   
    b = MOUSE_LEFT_BUTTON; 
    break ;
  case SDL_BUTTON_RIGHT: 
    b = MOUSE_RIGHT_BUTTON;
    break ;
  case SDL_BUTTON_MIDDLE: 
    b = MOUSE_MIDDLE_BUTTON;
    break ;
  case 4:
    b = MOUSE_WHEEL_UP_BUTTON;
    break ;
  case 5:
    b = MOUSE_WHEEL_DOWN_BUTTON;
    break ;
  default:
    /* Unknown button: ignore */
    return ; 
  }

  s = -1;
  if(e->state==SDL_PRESSED)  s=MOUSE_DOWN;
  if(e->state==SDL_RELEASED) s=MOUSE_UP;
  
  MouseEvent(b,s,e->x,e->y) ;
}

/***********************************************************************
 *        Translate the keystrokes from SDL for foobillard++           *
 ***********************************************************************/

static int translate_key(SDL_KeyboardEvent* e)
{
  int keysym=0;

  switch (e->keysym.sym) {
  case SDLK_PAGEUP:
    keysym = KSYM_PAGE_UP ;
    break;
  case SDLK_UP:
    keysym = KSYM_UP ;
    break;
  case SDLK_PAGEDOWN:
    keysym = KSYM_PAGE_DOWN ;
    break;
  case SDLK_DOWN:
    keysym = KSYM_DOWN ;
    break;
  case SDLK_LEFT:
    keysym = KSYM_LEFT ;
    break;
  case SDLK_RIGHT:
    keysym = KSYM_RIGHT ;
    break;
  case SDLK_F1:
    keysym = KSYM_F1 ;    
    break;

  case SDLK_F2:
    keysym = KSYM_F2 ;
    break;
  case SDLK_F3:
    keysym = KSYM_F3 ;
    break;
  case SDLK_F4:
    keysym = KSYM_F4 ;
    break;
  case SDLK_F5:
    keysym = KSYM_F5 ;
    break;
  case SDLK_F6:
    keysym = KSYM_F6 ;
    break;
  case SDLK_F7:
    keysym = KSYM_F7 ;
    break;
  case SDLK_F8:
    keysym = KSYM_F8 ;
    break;
  case SDLK_F9:
    keysym = KSYM_F9 ;
    break;
  case SDLK_F10:
    keysym = KSYM_F10 ;
    break;
  case SDLK_F11:
    keysym = KSYM_F11 ;
    break;
  case SDLK_F12:
    keysym = KSYM_F12 ;
    break;
  case SDLK_KP_ENTER:
    keysym = KSYM_KP_ENTER ;
    break;
  default:
    //fprintf(stderr,"%i\n",e->keysym.sym);
    if (e->keysym.sym>0 && e->keysym.sym<=127) {
      keysym = (int) e->keysym.sym ;
      if((e->keysym.mod & KMOD_LSHIFT) || (e->keysym.mod & KMOD_RSHIFT) || (e->keysym.mod & KMOD_CAPS)) {
        if(keysym >= SDLK_a && keysym <= SDLK_z) {
           keysym = keysym-32;
        }
      }
    } else {
      /* ignore */
      return -1;
    }
  }
  return keysym;
}

/***********************************************************************
 *                  Handle for keystroke down                          *
 ***********************************************************************/

static void handle_key_down(SDL_KeyboardEvent* e)
{
  int keysym;

  update_key_modifiers();
  keysym = translate_key(e);
  if(keysym!=-1){
      Key(keysym, keymodif);
  }
}

/***********************************************************************
 *                         Handle for key up                           *
 ***********************************************************************/

static void handle_key_up(SDL_KeyboardEvent* e)
{
  int keysym;

  update_key_modifiers();
  keysym = translate_key(e);
  if(keysym!=-1){
      KeyUp(keysym);
  }
}

/***********************************************************************
 *                Resize the SDL Surface handle                        *
 ***********************************************************************/

void sys_resize( int width, int height, int callfrom )
{
    if(width < 958) width = 958;      // don't resize below this
    if(height < 750) height = 750;

    SDL_SetWindowSize(glWindow, width,height);
    SDL_GetWindowSize(glWindow, &win_width, &win_height);
    SDL_GL_GetDrawableSize(glWindow, &native_width, &native_height);
    ResizeWindow(win_width, win_height);
}

/***********************************************************************
 *              Handle for the reshape event of SDL                    *
 ***********************************************************************/

static void handle_reshape_event( int width, int height ) 
{
	  if(!ignore) {
     sys_resize( width, height, 0 );
	  }
	  ignore = 0;
}

/***********************************************************************
 *                 work for the SDL mousemotion event                  *
 ***********************************************************************/

void handle_motion_event(SDL_MouseMotionEvent *e) 
{
  update_key_modifiers();
  MouseMotion(e->x,e->y);
}

/***********************************************************************
 *                     Process all the SDL events                      *
 ***********************************************************************/

static void  process_events( void )
{
  /* Our SDL event placeholder. */
  SDL_Event event;

    /* Grab all the events off the queue. */
  while( SDL_PollEvent( &event ) ) 
   {
    switch( event.type ) {
      case SDL_KEYUP:
        handle_key_up( &event.key );
	       break;
      case SDL_KEYDOWN:
        /* Handle key presses. */
        handle_key_down( &event.key );
	       break;
      case SDL_QUIT:
	       /* Handle quit requests (like Ctrl-c). */
	       sys_exit(0);
	       break;
      case SDL_MOUSEMOTION:
        handle_motion_event(&(event.motion)) ;
	       break ;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	       handle_button_event(&(event.button)) ;
	       check_SDL = 0;
        break ;

      case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    //printf("Window %d resized to %dx%d\n", event.window.windowID, event.window.data1, event.window.data2);
                    handle_reshape_event(event.window.data1,event.window.data2);
            }
            break;
        break;
      default:
        //	fprintf( stderr,"EVENT: %d\n", (int) event.type ) ;
        break;
    }
   }
}

/***********************************************************************
 *   set and return SDL_Event status for manual from fullscreen        *
 ***********************************************************************/

void set_checkkey(void) {
    check_SDL = 1;
}

int checkkey(void) {
	   process_events();
    return(check_SDL);
}

/***********************************************************************
 *           get all resolution modes for SDL/OpenGL                   *
 ***********************************************************************/

sysResolution *sys_list_modes( void ) {
    int i;
    SDL_DisplayMode current;
    sysResolution * sysmodes = NULL;
    int display_count = SDL_GetNumVideoDisplays();
    int mode_index =0;
    int decrement=0;


    // Get current display mode of all displays.
    for(i = 0; i < SDL_GetNumVideoDisplays(); ++i){

        int should_be_zero = SDL_GetCurrentDisplayMode(i, &current);

        if(should_be_zero != 0) {
          // In case of error...
          printf("Could not get display mode for video display #%d: %s", i, SDL_GetError());
        } else {
          // On success, print the current display mode.
          printf("Display #%d: current display mode is %dx%dpx @ %dhz.\n", i, current.w, current.h, current.refresh_rate);
        }

    }

    for (int display_index = 0; display_index <= display_count; display_index++)
    {
        printf("Display %i:\n", display_index);

        int modes_count = SDL_GetNumDisplayModes(display_index);

        sysmodes = (sysResolution *) malloc((modes_count+1)*sizeof(sysResolution));

        for (mode_index = 0; mode_index <= modes_count; mode_index++)
        {
            SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

            if (SDL_GetDisplayMode(display_index, mode_index, &mode) == 0)
            {

                //we need only modes of current screenmode depth only
                if(mode.format == current.format)
                {
                    printf(" %i bpp\t%i x %i @ %iHz\n", SDL_BITSPERPIXEL(mode.format), mode.w, mode.h, mode.refresh_rate); 
                    sysmodes[mode_index-decrement].w = mode.w;
                    sysmodes[mode_index-decrement].h = mode.h;
                } else {
                    decrement++;
                }
            }
        }

    sysmodes[mode_index].w=0;  /* terminator */
    sysmodes[mode_index].h=0;  /* terminator */

    return( sysmodes );

    }
}

/***********************************************************************
 *                            SDL main loop                            *
 ***********************************************************************/

void sys_main_loop(void) {
  // we want a good smooth scrolling
  GLint old_t, t;
  GLint sleeptime;

  old_t = SDL_GetTicks();
  while(1) {
    process_events();
    DisplayFunc();
    SDL_GL_SwapWindow(glWindow);
    if(options_vsync && !vsync_available) {
       t = SDL_GetTicks();
       sleeptime = 15-(t-old_t); //wish sleeptime is 15 milliseconds
       old_t = t;
       if(sleeptime > 0) {
         SDL_Delay(sleeptime);
       }
    }
  }

}

/***********************************************************************
 *      Find the program's "data" directory and chdir into it          *
 *      and the program executable and directory to it
 ***********************************************************************/

static char data_dir[512];
static char exe_prog[512];

void enter_data_dir() {
    int success = 1;

#ifdef POSIX
    char proc_exe[20];
    char *slash_pos;
#endif

    do {
        success = 0;
#ifdef USE_WIN
        GetModuleFileName(NULL,exe_prog,sizeof(exe_prog));
#endif
#ifdef __APPLE__
        char *get_mac_data_directory();
        char *data_directory = get_mac_data_directory();

        strncpy(data_dir, data_directory, sizeof(data_dir));
        strncpy(exe_prog, data_directory, sizeof(exe_prog));
        free(data_directory);
#elif defined(POSIX)
        snprintf(proc_exe, sizeof(proc_exe), "/proc/%d/exe", getpid());
        if (readlink(proc_exe, data_dir, sizeof(data_dir)) < 0) {
            perror("readlink failed");
            break;
        }
        strncpy(exe_prog, data_dir, sizeof(exe_prog));
        // Remove program name
        slash_pos = strrchr(data_dir, '/');
        if (!slash_pos) break;
        *slash_pos = '\0';

        // Go one dir up
        slash_pos = strrchr(data_dir, '/');
        if (!slash_pos) break;

        // Add "/data"
        strncpy(slash_pos, "/data", sizeof(data_dir) - (slash_pos - data_dir));
#else
        /* ### TODO ### Get the working directory of the program
         * Solaris: getexecname()
         * FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
         * BSD with procfs: readlink /proc/curproc/file
         * Windows: GetModuleFileName() with hModule = NULL
         */
        strncpy(data_dir, "data", sizeof(data_dir));
#endif

        if (chdir(data_dir) < 0) {
            break;
        }

        success = 1;
    } while (0);
    if (!success) {
        //check for Linux Default Directory if possible
#ifdef USE_DEBIAN
        if(!chdir(DATA_DIRECTORY)){
           strncpy(data_dir, DATA_DIRECTORY, sizeof(data_dir));
        } else {
#endif
           fprintf(stderr,
            "Foobillard++ seems not to be correctly installed\n"
            "Cannot find valid data directory\n"
            "(assuming the current directory contains the data)\n");
#ifdef USE_DEBIAN
        }
#endif
    }
}

/***********************************************************************
 *           returns the "data" directory and chdir into it            *
 ***********************************************************************/

const char *get_data_dir() {
#ifdef POSIX
    return data_dir;
#else
    return ".";
#endif
}
/***********************************************************************
 *           returns the "exe" directory and applicationname           *
 ***********************************************************************/

const char *get_prog() {
    return exe_prog;
}
/***********************************************************************
 *      Check whether a given file exists                              *
 ***********************************************************************/

int file_exists(const char *path) {
#ifdef POSIX
    struct stat buf;
    return stat(path, &buf) == 0;
#else
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    fclose(fp);
    return 1;
#endif
}

/***********************************************************************
 *      Launch an external command                                     *
 ***********************************************************************/

int launch_command(const char *command) {
#ifdef USE_WIN
	   ShellExecute(NULL,"open",command,NULL,NULL,SW_SHOWNORMAL);
	   return (0);
#else
    return system(command);
#endif
}

void try_set_vsync() {
    if(SDL_GL_SetSwapInterval(options_vsync)<0)
      vsync_available = 0;
}
