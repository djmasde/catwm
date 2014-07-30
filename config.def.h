 /* config.h for catwm-0.0.4.c
 *
 *  ( o   o )  Made by cat...
 *  (  =^=  )
 *  (        )            ... for cat!
 *  (         )
 *  (          ))))))________________ Cute And Tiny Window Manager
 *  ______________________________________________________________________________
 *
 *  Copyright (c) 2014, Dj_Dexter, Helmuth.Schmelzer@gmail.com 
 *  Copyright (c) 2010, Rinaldini Julien, julien.rinaldini@heig-vd.ch
 *  Based in moetunes fork
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Some changes and additions by P. Newman 24.4.11
 *     - best not to email julien if you use this... :)
 *  Some changes and bugfixes by Dj_Dexter/djmasde 21.7.14
 *  Bug with desktop numbers:
 *  -before, only show 3 desktops
 *  -after, show to 9 desktops, configured with config.h
 *  -removed the next_desktop, and prev_desktop functions
 *  Now, numlock on, not affects the keyboard shortcuts
 *  Unmapnotify function for rare windows, Ex: thunderbird, firefox, etc...
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Mod (Mod1 == alt) and master size
   and I added panel size and  the windows key (Mod4 == Super)
   added shortcuts for different tiling modes
   added shortcuts for moving the window to the next workspace and back
   */
//#define MOD1            Mod1Mask
#define MOD4        Mod4Mask
#define MASTER_SIZE     0.6
#define PANEL_HEIGHT    20
#define BORDER_WIDTH    4
#define ATTACH_ASIDE    1 /* 0=TRUE, 1=New window is master */
#define DEFAULT_MODE    2 /* 0 = Horizontal, 1 = Fullscreen, 2 = Vertical */

// Colors
#define FOCUS           "#664422" // dkorange
#define UNFOCUS         "#004050" // blueish

//if have gmrun...
//const char* dmenucmd[] = {"gmrun",NULL};
const char* dmenucmd[] = {"dmenu_run","-b",NULL};
const char* urxvtcmd[] = {"urxvt",NULL};
const char* lockcmd[]  = {"xlock",NULL};
const char* next[]     = {"ncmpcpp","next",NULL};
const char* prev[]     = {"ncmpcpp","prev",NULL};
const char* toggle[]   = {"ncmpcpp","toggle",NULL };
const char* voldown[]  = {"amixer","set","PCM","5\%-",NULL};
const char* volup[]    = {"amixer","set","PCM","5\%+",NULL};
const char* firecmd[]  = {"qupzilla",NULL};
const char* leafpad[]  = {"leafpad",NULL};
const char* paharo[]   = {"psi",NULL};

// for reboot and shutdown
const char* rebootcmd[]     = {"sudo","reboot",NULL};
const char* shutdowncmd[]   = {"sudo","shutdown","-h","now",NULL};

// Avoid multiple paste
#define DESKTOPCHANGE(K,N) \
    {  MOD4,             K,                          change_desktop, {.i = N}}, \

// Shortcuts
static struct key keys[] = {
    // MOD               KEY                         FUNCTION        ARGS
    {  MOD4,             XK_h,                       increase,       {NULL}},
    {  MOD4,             XK_l,                       decrease,       {NULL}},
    {  MOD4|ShiftMask,   XK_c,                       kill_client,    {NULL}},
    {  MOD4,             XK_j,                       next_win,       {NULL}},
    {  MOD4,             XK_Tab,                     next_win,       {NULL}},
    {  MOD4,             XK_k,                       prev_win,       {NULL}},
    {  MOD4,             XK_c,                       spawn,          {.com = lockcmd}},
    {  MOD4,             XK_Right,                   spawn,          {.com = next}},
    {  MOD4,             XK_Left,                    spawn,          {.com = prev}},
    {  MOD4,             XK_Down,                    spawn,          {.com = toggle}},
    {  MOD4,             XK_F1,                      spawn,          {.com = voldown}},
    {  MOD4,             XK_F2,                      spawn,          {.com = volup}},
    {  MOD4,             XK_p,                       spawn,          {.com = dmenucmd}},
    {  MOD4,             XK_o,                       spawn,          {.com = firecmd}},
    {  MOD4|ShiftMask,   XK_Return,                  spawn,          {.com = urxvtcmd}},
    {  MOD4,             XK_z,                       spawn,          {.com = leafpad}},
    {  MOD4,             XK_x,                       spawn,          {.com = paharo}},
// Windows Key + shift + shortcut
    {  MOD4|ShiftMask,   XK_j,                       move_up,        {NULL}},
    {  MOD4|ShiftMask,   XK_k,                       move_down,      {NULL}},
    {  MOD4|ShiftMask,   XK_Return,                  swap_master,    {NULL}},
    {  MOD4|ShiftMask,   XK_m,                       toggle_fullscreen,{NULL}},
    {  MOD4|ShiftMask,   XK_v,                       switch_vertical,{NULL}},
    {  MOD4|ShiftMask,   XK_h,                       switch_horizontal,{NULL}},
// Windows Key + shift + shortcut
    // Quit normally 
    {  MOD4|ShiftMask,   XK_q,                       catkill,        {NULL}},
//    {  MOD1|ControlMask, XK_r,                       spawn,          {.com = rebootcmd}},
//    {  MOD1|ControlMask, XK_s,                       spawn,          {.com = shutdowncmd}},
       DESKTOPCHANGE(   XK_1,                                       0)
       DESKTOPCHANGE(   XK_2,                                       1)
       DESKTOPCHANGE(   XK_3,                                       2)
       DESKTOPCHANGE(   XK_4,                                       3)
       DESKTOPCHANGE(   XK_5,                                       4)
       DESKTOPCHANGE(   XK_6,                                       5)
       DESKTOPCHANGE(   XK_7,                                       6)
       DESKTOPCHANGE(   XK_8,                                       7)
       DESKTOPCHANGE(   XK_9,                                       8)
       DESKTOPCHANGE(   XK_0,                                       9)
};

#endif
