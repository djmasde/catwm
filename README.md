CATWM
=====

     /\___/\
    ( o   o )  Made by cat...
    (  =^=  )
    (        )            ... for cat!
    (         )
    (          ))))))________________ Cute And Tiny Window Manager

Summary
-------

catwm is a very simple and lightweight tiling window manager.
I will try to stay under 1000 SLOC.

Status
------
 * 21.07.14 -> v0.4  
   -Numlock not affects keyboard shotcurts. 

   -Multiple desktops works now ok,

   -Added space to use a toolbar, ex: dzen2 

   -unmapnotify function, for, rares windows, ex: thunderbird, firefox...

    Before this, catwm crashes, with about dialogs or new window, from these windows..

 * 22.07.14

   -About: catwm -v,

   -Removed a obsolete header,

   -changed volume keybindings.

 * 05.07.19 -> v0.3. Multiple desktops and correct some bugs
 * 30.06.10 -> v0.2. Back again \o/
 * 15.03.10 -> v0.2. The wm is functional -> I only use this wm!
 * 24.02.10 -> First release, v0.1. In this release 0.1, the wm is almost functional

Modes
-----

It allow the "normal" modes of tiling window managers:

    --------------
    |        |___|
    |        |___|
    | Master |___|
    |        |___|
    |        |___|
    --------------

Horizontal stack

and fullscreen mode

Installation
------------

Need Xlib, then:

    $ nano config.h
    
    $ make
    
    # make install

    $ make clean

Bugs
----
 * No bugs for the moment ;) (I mean, no importants bugs ;)

Todo
----
 * Add multiple view ("desktop") >> "cutre" multihead supported, see not mine screenshot: http://a.pomf.se/ggylig.png
 * Switch to XCB

If you have some particular request, just send me an e-mail, and I will see for it!

Here a screenshot: http://a.pomf.se/qxwuno.png
