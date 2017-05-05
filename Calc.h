/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   header file for calc.c

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


   History

   970922 : break out from Main.h
   980205 : stepping
   980225 : calckd[]
            use variables for font dependent sizes
*/

/* calculator window */
/* size and pos */

#include "Keys.h"

#define CALCW      (BCX+ksx+kdx)
#define CALCH      (YABS(8)+ksy+kdy)

#define CALCTITLE  "Calculator"    /* title */

/* display */

#define DISPSIZE     20           /* no of chrs in disp */
#define DISPWIDTH    (DISPSIZE*fontw) /* pixel width of display */
#define EXPSIZE      3            /* max length of exponent, excl sign */
#define EXPZERO      "+000"
#define DISPH        (fonth+4)    /* height of display */
#define CALCDISPX    (fontw/2)    /* x pos for display */
#define CALCDISPY    (fonth/2)    /* y pos for display */
#define DISPFRAMEADDX (fontw/5)   /* amount frame is outside display string */
#define DISPFRAMEADDY (fonth/5)   /* amount frame is outside display string */
#define ZEROSTR      "                   0"
#define DISPCONV     "%20.10f"    /* sprintf to display */
#define EXPCONV      "%03d"       /* sprintf exp to display */
#define DISPCONVOCT  "%20o"       /* sprintf to display octal */
#define DISPCONVHEX  "%20x"       /* sprintf to display hex */
#define DISPCONVEXP  "%20.10e"    /* sprintf to display with exp */
#define EXPHILIMIT   (1e9)        /* high limit for exp output */
#define EXPLOLIMIT   (1e-9)       /* low limit for exp output */
#define MAXBIN       1048576      /* max binary value 2^20 */
#define MAXBASE      16           /* max numeric base */
#define MAXINTDIGS   10           /* max number of digits in integer */
#define ACTDISPSIZE  (MAXINTDIGS+EXPSIZE+3) /* actual max chrs in display */

/* keys */

#define CKY        (CALCDISPY+DISPH+2*fonth) /* y top left for keys */

#define XABS(N)    ((N)*(kdx+ksx)+kdx)       /* x for button in row N */
#define YABS(N)    (CKY+(N)*(kdy+ksy)+kdy)   /* y for button in line N */
#define BCX        (XABS(5)+kdx)             /* x pos for base conv col */

#define NOOFKEYS   50

//extern struct keydef calckd[NOOFKEYS];
struct keydef calckd[NOOFKEYS];

/* history */

#define HISTORYSIZE 100       /* number of events remembered */
#define HNW         0         /* identifier for no window */
#define HAW         1         /* identifier for asciiwin */
#define HBW         2         /* identifier for binwin */
#define HCW         3         /* identifier for calcwin */
#define HMW         4         /* identifier for memwin */

struct historyentry
{
   int window;   /* where it happened */
   int key;      /* what happened */
};

/* operator and operand stack */

#define STACKSIZE  20

/* priority */

#define OPRMUL     0
#define OPRDIV     1
#define OPRADD     2
#define OPRSUB     3
#define OPRPAR     4
#define OPRPOW     5
#define OPRMOD     6

#define PRISIZE    7
#define MAXPRI     (PRISIZE+1)
#define MINPRI     (-1)

/* ASCII control */

#define CR        0x0d
#define LF        0x0a

/* Various magic numbers */

#define DEG        42
#define RAD        1138
#define STRLEN     100             /* generic string length */
#define TRUE       (!FALSE)

/* program memory */

/* learn */

#define LEARNSIZE    960          /* program steps */
#define SECONDOFFSET 5            /* offset to 2:nd learn code */
#define PGMNODEF     (-1)         /* not defined for program */

#define INCPGM pgmindex=(pgmindex+1)%LEARNSIZE /* inc to next step */

struct pgmentry
{
   struct keydef *kdp;
   int           second;
};

extern int running,stepping;
extern struct pgmentry program[LEARNSIZE];

/* misc */

extern int inverse,second;
