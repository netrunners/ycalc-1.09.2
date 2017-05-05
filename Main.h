/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   header file for ycalc

   Copyright (C) 1989-1998  Ulf Nordquist <un@pobox.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   History (By Ulf Nordquist)
   Ver 1  901108 : defines and global variables from xcalc.c
          901116 : move global variables back to xcalc.c
	  901212 : move ascii #define to ascii.c
	  901213 : move binary #define to binary.c
	           move memory #define to memory.c
	  910109 : RUNSTR
          910110 : key numbers to Keys.h
          910430 : flags for keys - DRAWINV
          970922 : remove confusing define of ON and OFF
                   and FALSE
                   define all sizes relative fontsize
          980225 : remove all defines for font, no fixed font anymore
          980227 : MAX()
*/

/* header files */

#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#define _ABS(x) ((x) >= 0 ? (x) : -(x))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/* for all windows */

#define CALCB      2               /* border */

#define NOKEY      1138

/* variables */

extern Display *calcdisp;
extern GC gc,invgc;
extern Window calcwin;
extern XColor backcolor,forecolor;
extern double xreg;
extern int fontb;
extern int fonth;
extern int fontw;
extern int kdx;
extern int kdy;
extern int ksx;
extern int ksy;
extern int newentry,screen;
extern int useimages;
extern int signconv;
