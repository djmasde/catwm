/* catwm-0.0.5.c
 *
 *   /\___/\
 *  ( o   o )  Made by cat...
 *  (  =^=  )
 *  (        )            ... for cat!
 *  (         )
 *  (          ))))))________________ Cute And Tiny Window Manager
 *  ______________________________________________________________________________
 *
 *  Copyright (c) 2014-2015, Dj_Dexter, Helmuth.Schmelzer@gmail.com
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

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/XF86keysym.h> //for multimedia keys
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
// imported from dminiwm.c and dwm.c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define TABLENGTH(X)    (sizeof(X)/sizeof(*X))
// for ignoring the numlock mask
#define CLEANMASK(mask) (mask & ~(numlockmask | LockMask))

enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };

typedef union {
    const char** com;
    const int i;
} Arg;

// Structs
struct key {
    unsigned int mod;
    KeySym keysym;
    void (*function)(const Arg arg);
    const Arg arg;
};

typedef struct client client;
struct client{
    // Prev and next client
    client *next;
    client *prev;

    // The window
    Window win;
};

typedef struct desktop desktop;
struct desktop{
    int master_size;
    int mode;
    client *head;
    client *current;
};

// Functions
static void add_window(Window w);
static void change_desktop(const Arg arg);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void decrease();
static void destroynotify(XEvent *e);
static void die(const char* e);
static void unmapnotify(XEvent *e);    // Thunderbird's write window just unmaps...
static unsigned long getcolor(const char* color);
static void grabkeys();
static void increase();
static void keypress(XEvent *e);
static void kill_client();
static void maprequest(XEvent *e);
static void move_down();
static void move_up();
static void next_win();
static void prev_win();
static void catkill(); //kill the cat, xD
static void remove_window(Window w);
static void save_desktop(int i);
static void select_desktop(int i);
static void send_kill_signal(Window w);
static void setup();
static void sigchld(int unused);
static void spawn(const Arg arg);
static void start();
static void swap_master();
static void toggle_fullscreen();
static void switch_vertical();
static void switch_horizontal();
static void tile();
static void update_current();
static int xerror(Display *dis, XErrorEvent *ee), (*xerrorxlib)(Display *, XErrorEvent *); //for error handler
static void logger(const char* e); //logger
static void cleanup(void); //imported from monsterwm.c
static void deletewindow(Window w); //imported from monsterwm.c
static Bool running = True;
static void wea(); //stupid quick and dirty anti not focus
static void client_to_desktop(const Arg arg); // to move window to any desktop, especified.

// Include configuration file (need struct key)
#include "config.h"

// Variable
static Display *dis;
unsigned int numlockmask;		/* dynamic key lock mask */
static int current_desktop;
static int master_size;
static int mode;
static int sh;
static int sw;
static int screen;
static unsigned int win_focus;
static unsigned int win_unfocus;
static Window root;
static client *head;
static client *current;
static int holder;    // for coming out of fullscreen mode back to what mode it was

// Events array
static void (*events[LASTEvent])(XEvent *e) = {
    [KeyPress] = keypress,
    [MapRequest] = maprequest,
    [UnmapNotify] = unmapnotify,
    [DestroyNotify] = destroynotify,
    [ConfigureNotify] = configurenotify,
    [ConfigureRequest] = configurerequest
};

// Desktop array
static desktop desktops[10];

/* ***************************** Window Management ******************************* */
void add_window(Window w) {
    client *c,*t;

    if(!(c = (client *)calloc(1,sizeof(client))))
           die("Error calloc!");

    if(head == NULL) {
        c->next = NULL;
        c->prev = NULL;
        c->win = w;
        head = c;
    }
    else {
        if(ATTACH_ASIDE == 0) {
            for(t=head;t->next;t=t->next);
                c->next = NULL;
                c->prev = t;
                c->win = w;
                t->next = c;
        }
        else {
            for(t=head;t->prev;t=t->prev);
                c->prev = NULL;
                c->next = t;
                c->win = w;
                t->prev = c;

            head = c;
        }
    }

    current = c;
    save_desktop(current_desktop);
}

void client_to_desktop(const Arg arg) {
    client *tmp = current;
    int tmp2 = current_desktop;
    
    if(arg.i == current_desktop || current == NULL)
        return;

    // Add client to desktop
    select_desktop(arg.i);
    add_window(tmp->win);
    save_desktop(arg.i);

    // Remove client from current desktop
    select_desktop(tmp2);
    remove_window(current->win);

    tile();
    update_current();
    change_desktop(arg);
}

void unmapnotify(XEvent *e) { // for thunderbird's write window and maybe others
    XUnmapEvent *ev = &e->xunmap;
    client *c;

    if(ev->send_event == 1) {
        for(c=head;c;c=c->next)
            if(ev->window == c->win) {
               remove_window(ev->window);
               update_current(); 
               //wtf! without this, in tiled mode, not get all size of the window
               increase();
               //Quick and dirty hack, for molest resize, if have more dialogs...
               decrease();
               //without this, windows like GnuStep style or others, catwm retakes the window
               //and disappear the bug "ghost" window.
               update_current();
               return;
            }
    }
}

void swap_master() {
    Window tmp;

    if(head != NULL && current != NULL && current != head && mode != 1) {
        tmp = head->win;
        head->win = current->win;
        current->win = tmp;
        current = head;

        save_desktop(current_desktop);
        tile();
    }
}

void move_down() {
    Window tmp;
    if(current == NULL || current->next == NULL || current->win == head->win || current->prev == NULL)
        return;

    tmp = current->win;
    current->win = current->next->win;
    current->next->win = tmp;
    //keep the moved window activated
    next_win();
    save_desktop(current_desktop);
    tile();
}

void move_up() {
    Window tmp;
    if(current == NULL || current->prev == head || current->win == head->win) {
        fprintf(stdout, "%s\n","move_up not gonna be used");
        return;
    }
    tmp = current->win;
    current->win = current->prev->win;
    current->prev->win = tmp;
    prev_win();
    save_desktop(current_desktop);
    tile();
}

void next_win() {
    client *c;

    if(current != NULL && head != NULL) {
        if(current->next == NULL)
            c = head;
        else
            c = current->next;

        current = c;
        update_current();
    }
}

void prev_win() {
    client *c;

    if(current != NULL && head != NULL) {
        if(current->prev == NULL)
            for(c=head;c->next;c=c->next);
        else
            c = current->prev;

        current = c;
        update_current();
    }
}

void decrease() {
    if(master_size > 50) {
        master_size -= 10;
        tile();
    }
}

void increase() {
    // if mode 2 = vertical stack prevents catwm crash with badsize X error.
    if(mode != 2) {
    if(master_size < sh-50) {
        master_size += 10;
        tile();
    }
    }
    else {
    // if mode 1 = horizontal stack resize, but no all, prevents catwm crash with badsize X error.
        if(master_size < sw-50) {
        master_size += 10;
        tile();
    }
    }   
}

void remove_window(Window w) {
    client *c;

    // CHANGE THIS UGLY CODE
    for(c=head;c;c=c->next) {

        if(c->win == w) {
            if(c->prev == NULL && c->next == NULL) {
//                free(head);
                head = NULL;
                current = NULL;
                return;
            }

            if(c->prev == NULL) {
                head = c->next;
                c->next->prev = NULL;
                current = c->next;
            }
            else if(c->next == NULL) {
                c->prev->next = NULL;
                current = c->prev;
            }
            else {
                c->prev->next = c->next;
                c->next->prev = c->prev;
                current = c->prev;
            }
            //removed this, possible "steal the memory"
//            free(c);
            update_current();
            return;
        }
    }
}

void kill_client() {
    if(current != NULL) {
        //send delete signal to window
        XEvent ke;
        ke.type = ClientMessage;
        ke.xclient.window = current->win;
        ke.xclient.message_type = XInternAtom(dis, "WM_PROTOCOLS", True);
        ke.xclient.format = 32;
        ke.xclient.data.l[0] = XInternAtom(dis, "WM_DELETE_WINDOW", True);
        ke.xclient.data.l[1] = CurrentTime;
        XSendEvent(dis, current->win, False, NoEventMask, &ke);
        send_kill_signal(current->win);
    }
}

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(dis,screen);

    if(!XAllocNamedColor(dis,map,color,&c,&c))
        die("Error parsing color!");

    return c.pixel;
}

void update_current() {
    client *c;

    for(c=head;c;c=c->next)
        if(current == c) {
            // "Enable" current window
            XSetWindowBorderWidth(dis,c->win,BORDER_WIDTH);
            XSetWindowBorder(dis,c->win,win_focus);
            XSetInputFocus(dis,c->win,RevertToParent,CurrentTime);
            XRaiseWindow(dis,c->win);
        }
        else
            XSetWindowBorder(dis,c->win,win_unfocus);
            XSync(dis, False);
}

/* **************************** Desktop Management ************************************* */
void change_desktop(const Arg arg) {
    client *c;

    if(arg.i == current_desktop)
        return;

    // Save current "properties"
    save_desktop(current_desktop);

    // Unmap all window
    if(head != NULL)
        for(c=head;c;c=c->next)
            XUnmapWindow(dis,c->win);

    // Take "properties" from the new desktop
    select_desktop(arg.i);

    // Map all windows
    if(head != NULL)
        for(c=head;c;c=c->next)
            XMapWindow(dis,c->win);

    tile();
    update_current();

}

void save_desktop(int i) {
    desktops[i].master_size = master_size;
    desktops[i].mode = mode;
    desktops[i].head = head;
    desktops[i].current = current;
}

void select_desktop(int i) {
    head = desktops[i].head;
    current = desktops[i].current;
    master_size = desktops[i].master_size;
    mode = desktops[i].mode;
    current_desktop = i;
}

void tile() {
    client *c;
    int n = 0;
    int x = 0;

    // If only one window
    if(head != NULL && head->next == NULL) {
        XMoveResizeWindow(dis,head->win,0,0,sw-4,sh-4);
    }
    else if(head != NULL) {
        switch(mode) {
            case 0: /* Horizontal */
                // Master window
                XMoveResizeWindow(dis,head->win,0,0,sw-4,master_size - BORDER_WIDTH);

                // Stack
                for(c=head->next;c;c=c->next) ++n;
                for(c=head->next;c;c=c->next) {
                    XMoveResizeWindow(dis,c->win,x,master_size + BORDER_WIDTH,(sw/n) - BORDER_WIDTH,sh-master_size - (2 * BORDER_WIDTH));
                    x += sw/n;
                }
                break;
            case 1: /* Fullscreen */
                for(c=head;c;c=c->next) {
                    XMoveResizeWindow(dis,c->win,0,0,sw,sh);
                }
                break;
            case 2: /* Vertical */
                // Master window
                XMoveResizeWindow(dis,head->win,0,0,master_size - BORDER_WIDTH,sh - BORDER_WIDTH);

                // Stack
                for(c=head->next;c;c=c->next) ++n;
                for(c=head->next;c;c=c->next) {
                    XMoveResizeWindow(dis,c->win,master_size + BORDER_WIDTH,x,sw-master_size-(2*BORDER_WIDTH),(sh/n) - BORDER_WIDTH);
                    x += sh/n;
                }
                break;
            default:
                break;
        }
    }
}

void toggle_fullscreen() {
    if(mode != 1) {
        holder = mode;
        mode = 1;
    }
    else {
        mode = holder;
    }

    tile();
    update_current();
}

void switch_vertical() {
    mode = 2;
        master_size = sw * MASTER_SIZE;
    tile();
    update_current();
}

void switch_horizontal() {
    mode = 0;
        master_size = sh * MASTER_SIZE;
    tile();
    update_current();
}

/* ********************** Keyboard Management ********************** */
void grabkeys() {
    unsigned int i,j;
    KeyCode code;

    // numlock workaround
    XModifierKeymap *modmap;
    numlockmask = 0;
    modmap = XGetModifierMapping(dis);
    for (i=0;i<8;++i) {
        for (j=0;j<modmap->max_keypermod;++j) {
            if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dis, XK_Num_Lock))
                numlockmask = (1 << i);
        }
    }
    XFreeModifiermap(modmap);

    XUngrabKey(dis, AnyKey, AnyModifier, root);
    // For each shortcuts
    for(i=0;i<TABLENGTH(keys);++i) {
        code = XKeysymToKeycode(dis,keys[i].keysym);
        XGrabKey(dis, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dis, code, keys[i].mod | LockMask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dis, code, keys[i].mod | numlockmask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dis, code, keys[i].mod | numlockmask | LockMask, root, True, GrabModeAsync, GrabModeAsync);
    }
}

void keypress(XEvent *e) {
    unsigned int i;
    KeySym keysym;
    XKeyEvent *ev = &e->xkey;

    keysym = XkbKeycodeToKeysym(dis, (KeyCode)ev->keycode, 0, 0);
    //fprintf(stderr, "pressed key %s\n", XKeysymToString(keysym));
    for(i=0;i<TABLENGTH(keys); ++i) {
        if(keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)) {
            if(keys[i].function)
                keys[i].function(keys[i].arg);
        }
    }
}

void configurenotify(XEvent *e) {
    // Do nothing for the moment
}

/* ********************** Signal Management ************************** */
void configurerequest(XEvent *e) {
    // Paste from DWM, thx again \o/
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow(dis, ev->window, ev->value_mask, &wc);
}

void destroynotify(XEvent *e) {
    int i=0;
    client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    // Uber (and ugly) hack ;)
    for(c=head;c;c=c->next)
        if(ev->window == c->win)
            i++;
    
    // End of the hack
    if(i == 0)
        return;

    remove_window(ev->window);
    tile();
    update_current();
}

void die(const char* e) {
    fprintf(stdout,"catwm-0.0.5: %s\n",e);
    exit(1);
}
 
void maprequest(XEvent *e) {
    XMapRequestEvent *ev = &e->xmaprequest;

    // For fullscreen mplayer (and maybe some other program)
    client *c;
    for(c=head;c;c=c->next)
        if(ev->window == c->win) {
            XMapWindow(dis,ev->window);
            return;
        }

    add_window(ev->window);
    XMapWindow(dis,ev->window);
    tile();
    update_current();
}

void catkill() {
        //Close connection to X, dwm style :)
        XCloseDisplay(dis);
        fprintf(stdout, "catwm-0.0.5: You are killed me!\n");
        die("forced shutdown");
}

void send_kill_signal(Window w) { 
    XEvent ke;
    ke.type = ClientMessage;
    ke.xclient.window = w;
    ke.xclient.message_type = XInternAtom(dis, "WM_PROTOCOLS", True);
    ke.xclient.format = 32;
    ke.xclient.data.l[0] = XInternAtom(dis, "WM_DELETE_WINDOW", True);
    ke.xclient.data.l[1] = CurrentTime;
    XSendEvent(dis, w, False, NoEventMask, &ke);
}

void setup() {
    unsigned int i;

    // Install a signal
    sigchld(0);

    // Screen and root window
    screen = DefaultScreen(dis);
    root = RootWindow(dis,screen);

    // Screen width and height
    sw = XDisplayWidth(dis,screen) - BORDER_WIDTH;
    sh = XDisplayHeight(dis,screen) - PANEL_HEIGHT - BORDER_WIDTH;

    // Colors
    win_focus = getcolor(FOCUS);
    win_unfocus = getcolor(UNFOCUS);

    // Shortcuts
    grabkeys();

    // Default stack
    mode = DEFAULT_MODE;

    // For exiting
//    bool_quit = 0;

    // List of client
    head = NULL;
    current = NULL;

    // Master size
    if(mode == 0)
        master_size = sh*MASTER_SIZE;
    else
        master_size = sw*MASTER_SIZE;

    // Set up all desktop
//    int i;
    for(i=0;i<TABLENGTH(desktops);++i) {
        desktops[i].master_size = master_size;
        desktops[i].mode = mode;
        desktops[i].head = head;
        desktops[i].current = current;
    }

    // Select first dekstop by default
    const Arg arg = {.i = 0};
    current_desktop = arg.i;
    change_desktop(arg);
    // To catch maprequest and destroynotify (if other wm running)
    XSelectInput(dis,root,SubstructureNotifyMask|SubstructureRedirectMask);
    fprintf(stdout,"\n\n catwm-0.0.5: We're up and running!\n");
    update_current();
}

void sigchld(int unused) {
    // Again, thx to dwm ;)
    if(signal(SIGCHLD, sigchld) == SIG_ERR)
        die("Can't install SIGCHLD handler");
    while(0 < waitpid(-1, NULL, WNOHANG));
}

void spawn(const Arg arg) {
    if(fork() == 0) {
        if(fork() == 0) {
            if(dis)
                close(ConnectionNumber(dis));

            setsid();
            execvp((char*)arg.com[0],(char**)arg.com);
        }
        exit(0);
    }
}

void start() {
    XEvent ev;
    while(running && !XNextEvent(dis, &ev)) if (events[ev.type]) events[ev.type](&ev);
}

void wea() {
    //quick and dirty anti not focus
    decrease();
    increase();
}

//for logging events...
void logger(const char* e) {
    fprintf(stderr,"\n\033[0;34m:: catwm-0.0.5 : %s \033[0;m\n", e);
}

/* There's no way to check accesses to destroyed windows, thus those cases are ignored (especially on UnmapNotify's).  Other types of errors call Xlibs default error handler, which may call exit.  */
int xerror(Display *dis, XErrorEvent *ee) {
    if(ee->error_code == BadWindow || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
    if(ee->error_code == BadAccess) {
        logger("\033[0;31mIs Another Window Manager Running? Exiting!");
        exit(1);
    } else logger("\033[0;31mBad Window Error!");
    return xerrorxlib(dis, ee); /* may call exit */
}

/**
 * clients receiving a WM_DELETE_WINDOW message should behave as if
 * the user selected "delete window" from a hypothetical menu and
 * also perform any confirmation dialog with the user.
 */
void deletewindow(Window w) {
     if(current != NULL) {
        //send delete signal to window
        XEvent ke;
        ke.type = ClientMessage;
        ke.xclient.window = current->win;
        ke.xclient.message_type = XInternAtom(dis, "WM_PROTOCOLS", True);
        ke.xclient.format = 32;
        ke.xclient.data.l[0] = XInternAtom(dis, "WM_DELETE_WINDOW", True);
        ke.xclient.data.l[1] = CurrentTime;
        XSendEvent(dis, current->win, False, NoEventMask, &ke);
        send_kill_signal(current->win);
    }

}

/**
 * remove all windows in all desktops by sending a delete window message
 */
void cleanup(void) {
    Window root_return, parent_return, *children;
    unsigned int nchildren;

    XUngrabKey(dis, AnyKey, AnyModifier, root);
    XQueryTree(dis, root, &root_return, &parent_return, &children, &nchildren);
    for (unsigned int i = 0; i < nchildren; i++) deletewindow(children[i]);
    if (children) XFree(children);
    XSync(dis, False);
}

int main(int argc, char **argv) {
    //exported from dwm.c
    if(argc == 2 && !strcmp("-v", argv[1]))
	die("© 2010 pyknite, © 2014-2015 Dj_Dexter, see LICENSE for details\n");
    else if(argc != 1)
	die("usage: catwm [-v]\n");
    // Open display   
    if(!(dis = XOpenDisplay(NULL))) {
        die("Cannot open display!");
    }
    // X error handler (if other wm is running in the same display)
    XSetErrorHandler(xerror);
    
    // Setup env
    setup();
    // Start wm
    start();
    cleanup();
    //quick and dirty anti not focus
    wea();
    // Close display
//    XCloseDisplay(dis);

    return 0;
}
