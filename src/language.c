/* language.c
**
**    code for Language / Localisation
**    Copyright (C) 2010 - 2013  Holger Schaekel
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include "options.h"
#include "sys_stuff.h"
#include "language.h"
#ifdef USE_WIN
  #include <windows.h>
  #include <shellapi.h>
#endif

char localeText[MAX_TEXT_ENTRIES][MAX_TEXT_ENTRY_LEN];

static char charlang[3]; // system locale code

/***********************************************************************
 *          Find the systems locale and store it in charlang           *
 ***********************************************************************/

static void find_system_locale()
{
    char *locale_str;
    char *locale_code_pos;

    if(strcmp(options_language, "00")) {
    	strncpy(charlang, options_language, sizeof(charlang));
    	return;
    }

    setlocale(LC_ALL,"");
#ifdef LC_MESSAGES
    locale_str = setlocale(LC_MESSAGES, NULL);
#else
    locale_str = setlocale(LC_ALL, NULL);
#endif

    locale_code_pos = strstr(locale_str, "LC_MESSAGES");
    if (locale_code_pos) {
        locale_code_pos += strlen("LC_MESSAGES") + 1;
    } else {
        locale_code_pos = locale_str;
    }

    if (locale_code_pos[0] && locale_code_pos[1]) {
        snprintf(charlang, sizeof(charlang), "%c%c",
            tolower(locale_code_pos[0]), tolower(locale_code_pos[1]));
    } else {
        strncpy(charlang, "en", sizeof(charlang));
    }

    if ((arch == ARCH_WIN32 || arch == ARCH_WIN64) && !strcmp(charlang, "ge")) {
        //windows have other locales than Linux/Unix
        strncpy(charlang, "de", sizeof(charlang));
    }
    strncpy(options_language,charlang, sizeof(charlang));
}

/***********************************************************************
 *      Find the program's locale file and return the path to it       *
 ***********************************************************************/

static char *find_localized_file(const char *base_name)
{
    static char full_path[512];
    char exe_path[512] = "locale/";

#ifdef USE_WIN
    char *cp;
    GetModuleFileName(NULL,exe_path,sizeof(exe_path));
    if((cp = strrchr(exe_path,'\\'))) { //extract the program name from path
      cp[0] = 0;
    }
    strcat(exe_path,"\\data\\locale\\");
#endif

    snprintf(full_path, sizeof(full_path), "%s%s/%s", exe_path, charlang, base_name);
    if (file_exists(full_path)) return full_path;

    snprintf(full_path, sizeof(full_path), "%sen/%s", exe_path, base_name);
    if (file_exists(full_path)) return full_path;

    error_print("Locale file not found: %s\n",full_path);

    return NULL;
}

/***********************************************************************
 *      load the entire localized file for the program output          *
 ***********************************************************************/

static void load_language_file()
{
    char *language_file = find_localized_file("language.txt");
    FILE *fp;
    int index = 0;
    char *cp;

    if (language_file) fp = fopen(language_file, "r");
    if (!language_file || !fp) {
        error_print("ERROR: Cannot open language file - terminating.",NULL);
        exit(1);
    }

    while ((fgets(localeText[index], MAX_TEXT_ENTRY_LEN, fp) != NULL) &&
           (index < MAX_TEXT_ENTRIES)) {
        cp = strrchr(localeText[index], '\r');
        if (cp) *cp = '\0';
        cp = strrchr(localeText[index], '\n');
        if (cp) *cp = '\0';

        index++;
    }

    fclose(fp);
}

/***********************************************************************
 *                   init the language locale system                   *
 ***********************************************************************/

void init_language()
{
    find_system_locale();
    load_language_file();
}
