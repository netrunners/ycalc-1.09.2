/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Calculator window handling

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
    950506 : collect calc stuff here from keys.c and main.c
             put all of keys.h here
    960418 : DrawKeys() DrawOneKey() to keys.c
    960418 : InitKeys() to keys.c
    960426 : RunProgram()
    960617 : new key definition structs (major rewrite)
    970919 : continue major rewrite
    970920 : draw line direct around display
    970921 : HandleCalc() - use HandleKeys()
    970922 : change standby algorithm by use of standby variable
             HandleCalc() - move second/inverse handling to HandleCalcKeys()
    970923 :                handle key press/release
    980123 : Bug: When digits overflow the display, you get a core dump
                  PushDigit() - i was not initialized when the display was full
             Bug: Core dump on overflow, reported by Uwe Bonnes
                  <bon@elektron.ikp.physik.tu-darmstadt.de>
	          myfta() - handle inf and nan
             rename matherr() to mymatherr(), reported by Leiming Qian
               <lqian@jabba.csl.uiuc.edu>
    980130 : RefreshBase() - do RefreshMemDisplay()
    980205 : do not destroy display after running
             stepping
    980209 : HandleCalcKeys() - case PBPASTE: do XFree() on ptr returned by
                                XFetchBytes()
    980211 : myfta() - force non-exponent display if base!=10
    980213 : (friday)
             handle address entry and base checking in PushDigit() and use it
              instead of HandleDecDigit(), which is removed
             Bug: PushDigit() - only allow entry of digits which are
                  consistent with the base
    980225 : globalize calckd[]
             HandleCalcKeys() - handle second and invers flags for learn mode
	                        also
	     InitCalc() - since font sizes are variables now, calckd[] sizes
	                  and positions must be initialized by hand
			  slight hack to get base coversion column keys in
                          place
    980226 : calckd[] - replace keynul with NULL
	     InitCalc() - remove texts from calckd[] if images are available
    980331 : myfta() - use signconv
*/

#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/keysymdef.h>

#include <limits.h>
#include <math.h>

#include "Calc.h"
#include "Main.h"
#include "Memory.h"

                   /*  01234567890123456789 */
#define RUNSTR        "    - Running -     "
#define STANDBYSTR    " - Access standby - "

/* error strings */

#define CONVNAME      "Base conversion "    /* the space is magic */
#define DIVNAME       "Division by zero"
#define FACTNAME      "Factorial"
#define LNXNAME       "Base e logarithm"
#define LOGNAME       "Base 10 logarithm"
#define SQRTNAME      "Square root"
#define NEGINFNAME    "Negative infinity"
#define POSINFNAME    "Positive infinity"
#define NANNAME       "Not a number"

#define NOINVMSG      "No inverse defined"

#define STKOVFMSG     "Stack overflow"

/* calculator window */
/* fixed text */

#define TXT1       "Base:"
#define TXT1X      (CALCDISPX-3)
#define TXT1Y      (CALCDISPY+DISPH+5+fontb)
#define TXT1X1     (TXT1X+5*fontw)           /*x position for base number */
#define TXT2       "Mode:"
#define TXT2D      "Deg"
#define TXT2R      "Rad"
#define TXT2X      (TXT1X1+3*fontw)
#define TXT2Y      TXT1Y
#define TXT2X1     (TXT2X+5*fontw)           /*x position for mode string */
#define TXT3I      "INV"
#define TXT3N      "   "
#define TXT3X      (TXT2X1+4*fontw)
#define TXT3Y      TXT1Y
#define TXT4N      "    "
#define TXT4X      (TXT3X+4*fontw)
#define TXT4Y      TXT1Y

/* display in learn mode */

#define PGMDISPSTEP (DISPSIZE-7)  /* index in display for program step */
#define PGMDISPSEP  "  "          /* separator between step and code */

/* key identifiers */

#define PBABOUT           0
#define PBINV             1
#define PBOFF             2
#define PBCE              3
#define PB7               4
#define PB8               5
#define PB9               6
#define PBDIV             7
#define PBCLR             8
#define PB4               9
#define PB5               10
#define PB6               11
#define PBMUL             12
#define PBCHS             13
#define PB1               14
#define PB2               15
#define PB3               16
#define PBSUB             17
#define PBA               18
#define PB0               19
#define PBPOINT           20
#define PBEQUAL           21
#define PBADD             22
#define PBB               23
#define PBC               24
#define PBD               25
#define PBE               26
#define PBF               27
#define PBBIN             28
#define PBLRN             29
#define PBDEC             30
#define PBHEX             31
#define PBSTO             32
#define PBRCL             33
#define PBXST             34
#define PBLPAR            35
#define PBRPAR            36
#define PBLNX             37
#define PBSQRT            38
#define PBPOW             39
#define PBSUM             40
#define PBSQR             41
#define PB1SX             42
#define PB2ND             43
#define PBEE              44
#define PBSST             45
#define PBBST             46
#define PBRS              47
#define PBRST             48
#define PBGTO             49

#define PB1ST             100
#define PBLOG             101
#define PBPI              102
#define PBOCT             103
#define PBASCII           104
#define PBFACT            105
#define PBRAD             106
#define PBDEG             107
#define PBSIN             108
#define PBCOS             109
#define PBTAN             110
#define PBCUT             111
#define PBPASTE           112
#define PBEXC             113
#define PBPRD             114
#define PBCMS             115
#define PBENG             116
#define PBMOD             117
#define PBLIST            118
#define PBDISPM           119
#define PBCP              120

static struct keyhandler kh;

/* bitmaps */
/* keys */

/*#define _width 25
#define _height 15
static char _bits[] = {*/

static char keyabt[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xbc, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x10, 0x00,
   0xa4, 0x00, 0x10, 0x00, 0xbc, 0xb9, 0x3a, 0x00, 0xa4, 0xaa, 0x12, 0x00,
   0xa4, 0xaa, 0x12, 0x00, 0xa4, 0xbb, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keypi[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
   0x80, 0x25, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00,
   0x00, 0x12, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keypow[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x00,
   0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x88, 0x02, 0x00,
   0x80, 0x48, 0x04, 0x00, 0x80, 0x0c, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x80, 0x08, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keysqrt[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xff, 0x7f, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x21, 0x02, 0x00,
   0x88, 0x40, 0x01, 0x00, 0x88, 0x80, 0x00, 0x00, 0x50, 0x40, 0x01, 0x00,
   0x50, 0x20, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keysqr[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xc0, 0x00, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x40, 0x44, 0x00, 0x00, 0x80, 0xe2, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
   0x80, 0x02, 0x00, 0x00, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keyxst[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x04, 0x00, 0x00, 0x02, 0x04, 0x00, 0x88, 0x06, 0x1e, 0x00,
   0x50, 0x9e, 0x04, 0x00, 0x20, 0xc6, 0x04, 0x00, 0x50, 0xf2, 0x24, 0x00,
   0x88, 0xc0, 0x18, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char keylst[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x01, 0x02, 0x00,
   0x08, 0x38, 0x07, 0x00, 0x08, 0x05, 0x02, 0x00, 0x08, 0x39, 0x02, 0x00,
   0x08, 0x41, 0x12, 0x00, 0x78, 0x3d, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* key coordinates */

struct keydef calckd[NOOFKEYS]={

/* coordinate   xsz ysz ident & index            flags            learn code
    x      y             1st     2nd
    0      1     2   3    4       5                6                   7
   init
   col    row   ignored
*/

'b'    ,-2      ,0,0,PBOFF  ,PBOFF  ,0,             PGMNODEF,
"Off","Off",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,-1,0,0,PBABOUT,NOKEY  ,0,                   PGMNODEF,
"Abt" ,NULL ,keyabt,NULL,NULL,NULL,&kh,
0,1,0,0,PB2ND  ,PB1ST  ,INV1ST+INV2ND+ACT1ST+ACT2ND,PGMNODEF,
"2nd","1st",NULL  ,NULL,NULL,NULL,&kh,
1,1,0,0,PBINV  ,PBINV  ,INV1ST+INV2ND,              22,
"INV","INV",NULL  ,NULL,NULL,NULL,&kh,
2,1,0,0,PBLNX  ,PBLOG  ,INV1ST+INV2ND,              23,
"lnx","log",NULL  ,NULL,NULL,NULL,&kh,
3,1,0,0,PBCE   ,PBCP   ,0,                          24,
" CE"," CP",NULL  ,NULL,NULL,NULL,&kh,
4,1,0,0,PBCLR  ,NOKEY  ,0,                          25,
"CLR",NULL ,NULL  ,NULL,NULL,NULL,&kh,
0,2,0,0,PBLRN  ,NOKEY  ,ACT1ST,                     PGMNODEF,
"LRN",NULL ,NULL  ,NULL,NULL,NULL,&kh,
1,2,0,0,PBXST  ,NOKEY  ,0,                          32,
"x:t" ,NULL ,keyxst,NULL,NULL,NULL,&kh,
2,2,0,0,PBSQR  ,PBSIN  ,INV2ND,                     33,
"x^2" ,"sin",keysqr,NULL,NULL,NULL,&kh,
3,2,0,0,PBSQRT ,PBCOS  ,INV2ND,                     34,
"sqr" ,"cos",keysqrt,NULL,NULL,NULL,&kh,
4,2,0,0,PB1SX  ,PBTAN  ,INV2ND,                     35,
"1/x","tan",NULL  ,NULL,NULL,NULL,&kh,
0,3,0,0,PBSST  ,NOKEY  ,ACT1ST,                     PGMNODEF,
"SST", NULL ,NULL  ,NULL,NULL,NULL,&kh,
1,3,0,0,PBSTO  ,PBCMS  ,MRG1ST,                     42,
"STO", "CMs",NULL  ,NULL,NULL,NULL,&kh,
2,3,0,0,PBRCL  ,PBEXC  ,MRG1ST+MRG2ND,              43,
"RCL", "Exc",NULL  ,NULL,NULL,NULL,&kh,
3,3,0,0,PBSUM  ,PBPRD  ,INV1ST+INV2ND+MRG1ST+MRG2ND,44,
"SUM", "Prd",NULL  ,NULL,NULL,NULL,&kh,
4,3,0,0,PBPOW  ,NOKEY  ,0,                          45,
"y^x" ,NULL,keypow,NULL,NULL,NULL,&kh,
0,4,0,0,PBBST  ,NOKEY  ,ACT1ST,                     PGMNODEF,
"BST", NULL ,NULL  ,NULL,NULL,NULL,&kh,
1,4,0,0,PBEE   ,PBENG  ,INV2ND,                     52,
" EE", "Eng",NULL  ,NULL,NULL,NULL,&kh,
2,4,0,0,PBLPAR ,NOKEY  ,0,                          53,
" ( ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
3,4,0,0,PBRPAR ,NOKEY  ,0,                          54,
" ) ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
4,4,0,0,PBDIV  ,NOKEY  ,0,                          55,
" / ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
0,5,0,0,PBGTO  ,NOKEY  ,MRG1ST,                     61,
"GTO", NULL ,NULL  ,NULL,NULL,NULL,&kh,
1,5,0,0,PB7    ,NOKEY  ,0,                          7,
" 7 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
2,5,0,0,PB8    ,NOKEY  ,0,                          8,
" 8 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
3,5,0,0,PB9    ,NOKEY  ,0,                          9,
" 9 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
4,5,0,0,PBMUL  ,PBDEG  ,0,                          65,
" x ", "Deg",NULL  ,NULL,NULL,NULL,&kh,
1,6,0,0,PB4    ,NOKEY  ,0,                          4,
" 4 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
2,6,0,0,PB5    ,NOKEY  ,0,                          5,
" 5 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
3,6,0,0,PB6    ,NOKEY  ,0,                          6,
" 6 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
4,6,0,0,PBSUB  ,PBRAD  ,0,                          75,
" - ", "Rad",NULL  ,NULL,NULL,NULL,&kh,
0,7,0,0,PBRST  ,NOKEY  ,0,                          81,
"RST", NULL ,NULL  ,NULL,NULL,NULL,&kh,
1,7,0,0,PB1    ,NOKEY  ,0,                          1,
" 1 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
2,7,0,0,PB2    ,NOKEY  ,0,                          2,
" 2 ", NULL ,NULL  ,NULL,NULL,NULL,&kh,
3,7,0,0,PB3    ,PBPI   ,0,                          3,
" 3 ", "Pi" ,NULL  ,keypi,NULL,NULL,&kh,
4,7,0,0,PBADD  ,NOKEY  ,0,                          85,
" + ", NULL,NULL  ,NULL,NULL,NULL,&kh,
0,8,0,0,PBRS   ,NOKEY  ,0,                          91,
"R/S", NULL,NULL  ,NULL,NULL,NULL,&kh,
1,8,0,0,PB0    ,NOKEY  ,0,                          0,
" 0 ", NULL,NULL  ,NULL,NULL,NULL,&kh,
2,8,0,0,PBPOINT,NOKEY  ,0,                          93,
" . " , NULL,NULL  ,NULL,NULL,NULL,&kh,
3,8,0,0,PBCHS  ,NOKEY  ,0,                          94,
"+/-", NULL,NULL  ,NULL,NULL,NULL,&kh,
4,8,0,0,PBEQUAL,PBLIST ,0,                          95,
" = ", "Lst",NULL  ,keylst,NULL,NULL,&kh,
'b'    ,0,0,0,PBHEX  ,PBASCII,0,                    PGMNODEF,
"Hex","Asc",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,1,0,0,PBDEC  ,PBOCT  ,0,                    PGMNODEF,
"Dec","Oct",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,2,0,0,PBBIN  ,NOKEY  ,0,                    PGMNODEF,
"Bin",NULL,NULL  ,NULL,NULL,NULL,&kh,
'b'    ,3,0,0,PBA    ,PBFACT ,0,                    PGMNODEF,
" A "," x!",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,4,0,0,PBB    ,PBCUT  ,0,                    PGMNODEF,
" B ","Cut",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,5,0,0,PBC    ,PBPASTE,0,                    PGMNODEF,
" C ","Pas",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,6,0,0,PBD    ,PBMOD  ,0,                    PGMNODEF,
" D ","Mod",NULL  ,NULL,NULL,NULL,&kh,
'b'    ,7,0,0,PBE    ,NOKEY  ,0,                    PGMNODEF,
" E ",NULL,NULL  ,NULL,NULL,NULL,&kh,
'b'    ,8,0,0,PBF    ,PBDISPM,0,                    PGMNODEF,
" F ","DMs",NULL  ,NULL,NULL,NULL,&kh};

/* keysym to key identifier translation table */

struct ks2kientry
{
   int sym;   /* X keysym define */
   int id;    /* ycalc key id */
};

static struct ks2kientry ksxlate[]=
{
   /* X keysym define      ycalc key id */

      XK_Help          ,   PBABOUT,
      'I'              ,   PBINV,
      'i'              ,   PBINV,
      'Q'              ,   PBOFF,
      'q'              ,   PBOFF,
      0x08             ,   PBCE,
      '7'              ,   PB7,
      '8'              ,   PB8,
      '9'              ,   PB9,
      '/'              ,   PBDIV,
      0x1b             ,   PBCLR,
      0x7f             ,   PBCLR,
      '4'              ,   PB4,
      '5'              ,   PB5,
      '6'              ,   PB6,
      '*'              ,   PBMUL,
      0                ,   PBCHS,
      '1'              ,   PB1,
      '2'              ,   PB2,
      '3'              ,   PB3,
      '-'              ,   PBSUB,
      'A'              ,   PBA,
      'a'              ,   PBA,
      '0'              ,   PB0,
      '.'              ,   PBPOINT,
      '='              ,   PBEQUAL,
      0x0d             ,   PBEQUAL,
      '+'              ,   PBADD,
      'B'              ,   PBB,
      'b'              ,   PBB,
      'C'              ,   PBC,
      'c'              ,   PBC,
      'D'              ,   PBD,
      'd'              ,   PBD,
      'E'              ,   PBE,
      'e'              ,   PBE,
      'F'              ,   PBF,
      'f'              ,   PBF,
      0                ,   PBBIN,
      'L'              ,   PBLRN,
      'l'              ,   PBLRN,
      0                ,   PBDEC,
      'H'              ,   PBHEX,
      'h'              ,   PBHEX,
      'S'              ,   PBSTO,
      's'              ,   PBSTO,
      'R'              ,   PBRCL,
      'r'              ,   PBRCL,
      'X'              ,   PBXST,
      '('              ,   PBLPAR,
      ')'              ,   PBRPAR,
      0                ,   PBLNX,
      0                ,   PBSQRT,
      '^'              ,   PBPOW,
      0                ,   PBSUM,
      0                ,   PBSQR,
      0                ,   PB1SX,
      0                ,   PB2ND,
      0                ,   PBEE,
      XK_Up            ,   PBSST,
      XK_KP_Up         ,   PBSST,
      XK_Down          ,   PBBST,
      XK_KP_Down       ,   PBBST,
      0                ,   PBRS,
      0                ,   PBRST,
      0                ,   PBGTO,
      0                ,   PB1ST,
      0                ,   PBLOG,
      0                ,   PBPI,
      0                ,   PBOCT,
      0                ,   PBASCII,
      '!'              ,   PBFACT,
      0                ,   PBRAD,
      0                ,   PBDEG,
      0                ,   PBSIN,
      0                ,   PBCOS,
      0                ,   PBTAN,
      0                ,   PBCUT,
      0                ,   PBPASTE,
      0                ,   PBEXC,
      0                ,   PBPRD,
      0                ,   PBCMS,
      0                ,   PBENG,
      0                ,   PBMOD,
      0                ,   PBLIST,
      0                ,   PBDISPM,
      0                ,   PBCP,
};

/* address entry */

#define MAXADDRESSENTRY   10
#define MEMADDRESSDIGITS   2
#define PGMADDRESSDIGITS   3
#define ATMEMORY           1
#define ATPROGRAM          2

static int addrent;         /* address entry in progress FALSE or key no */
static int address;         /* address accumulator */
static int addrcnt;         /* number of address digits entered */
static int addrmaxcnt;      /* max number of address digits entered to enter */
static int addrtype;        /* type of address */
static int addrinv;         /* inverse of address functions */

static char oprstack[STACKSIZE];
static double opndstack[STACKSIZE];
static int oprptr;          /* ptr to next free in stack */

/*                             MUL DIV ADD SUB   PAR   POW MOD */

static char priority[PRISIZE]={ 1 , 1 , 0 , 0 , MINPRI, 2,  1};

/* learn */

static int learn;                 /* true if in learn mode */

extern int power;

static double treg;               /* calculator register */
static char display[DISPSIZE+1];  /* current calculator display content */
static int base;                  /* current base for calculator display */
static int engexp;                /* use engineering exponent mode */
static int enterexp;              /* enter exponent in progress */
static int point;                 /* digits are entered after decimal point */
static int standby;               /* if standby, next off _is_ off */
static int trigmode;              /* current angle mode */

/* global variables */

int running;                           /* true if running a program */
int stepping;                          /* true if stepping a program */
int pgmindex;                          /* next position in program memory */
struct pgmentry program[LEARNSIZE];    /* program memory */
double xreg;                           /* calculator register */
int inverse,newentry,second;

/* prototypes */

static ClearAll();
static DisplayChk();
static HandleCalcKeys(struct keydef *);
static HandleDecDigit(char);
static MyIsDigit(char, int);
static PopOperator();
static RefreshAddr();
static RefreshBase();
static RefreshDisplay(char *);
static RefreshGadgets();
static RefreshInverse();
static RefreshText();
static RefreshTrigMode();
static double ConvFromRad(double);
static double ConvToRad(double);
static void PushDigit(char);
static void PushOperator(char);

ClearDisplay()
{
   int i;

   newentry=1;
   enterexp=point=0;
   for(i=0;i<(DISPSIZE-1);i++)
      display[i]=' ';
   display[i]='0';
   display[i+1]='\0';
   RefreshDisplay(NULL);
}

ClrPgm()
{
   int i;

   for(i=0;i<LEARNSIZE;i++)
      program[i].kdp=NULL;
   pgmindex=0;
}

DisplayToXreg()
{
   char s[DISPSIZE+2];
   int i;

   if(base==10)
   {
      if((i=DisplayChk())!=(-1))
      {
      
	 /* insert 'e' if exp notation */

	 strncpy(s,display,i);
	 s[i]='\0';
	 strcat(s,"e");
	 strcat(s,&display[i]);
	 xreg=atof(s);
      }
      else
         xreg=atof(display);
   }
   else
   {
      /* other base */

      xreg=(double)strtoul(display,NULL,base);
   }
}

EndEntry()
{
   newentry=1;
   enterexp=point=0;
   DisplayToXreg();
}

HandleCalc(XButtonEvent *event)
{
   int i,j,r;
   struct keydef *lkdp;
   XKeyEvent *keyevent;
   KeySym ks;
   char buf[10];

   if((!kh.win)||(event->window!=kh.win))
      return;

   /* window is open */

   switch(event->type)
   {
      case Expose:
         RefreshGadgets();
	 RefreshText();
	 break;
      case KeyPress:
	 break;
      case KeyRelease:
	 keyevent=(XKeyEvent*)event;
	 ks=XLookupKeysym(keyevent,0);
	 r=XLookupString(keyevent,buf,10,&ks,NULL);

	 //handle esc and unused keys (instead of neg by defaukt)
	 switch(  ks ) {
		case XK_Escape : exit(0);
		case XK_Control_L:
		case XK_Control_R:
		case XK_Shift_L:
		case XK_Shift_R:return;
	 }

	 /* translate keysym to struct keydef */
	 
	 lkdp=NULL;
	 for(i=0;i<(sizeof(ksxlate)/sizeof(struct ks2kientry));i++)
	    if(ksxlate[i].sym==buf[0])
	    {
	       for(j=0;j<NOOFKEYS;j++)
		  if((calckd[j].i1==ksxlate[i].id)||
		     (calckd[j].i2==ksxlate[i].id))
		  {
		     lkdp= &calckd[j];
		     if(calckd[j].i1==ksxlate[i].id)
			second=0;
		     else
			second=1;
		     break;
		  }
	       break;
	    }
	 HandleCalcKeys(lkdp);
	 break;
      case ButtonPress:
      case ButtonRelease:
	 HandleCalcKeys(HandleKeys(event,&kh,!second?1:2));
	 break;
   }
}

InitCalc()
{
   int i;

   for(i=0;i<NOOFKEYS;i++)
   {
      if(calckd[i].x=='b')
      {
	 calckd[i].x=BCX;
	 if(calckd[i].y<0)
	    /*	    calckd[i].y=YABS(calckd[i].y)-(kdy+3);*/
	    calckd[i].y=YABS(calckd[i].y)-(kdy);
	 else
	    calckd[i].y=YABS(calckd[i].y);
      }
      else
      {
	 calckd[i].x=XABS(calckd[i].x);
	 calckd[i].y=YABS(calckd[i].y);
      }
      calckd[i].w=ksx;
      calckd[i].h=ksy;
      if((calckd[i].map1)&&useimages)
	 calckd[i].txt1=NULL;
      if((calckd[i].map2)&&useimages)
	 calckd[i].txt2=NULL;
   }
   engexp=inverse=learn=running=second=standby=0;
   trigmode=DEG;
   addrent=0;
   addrcnt=MAXADDRESSENTRY;
   base=10;
   treg=0;
   kh.win=calcwin;
   kh.def=calckd;
   kh.noc=NOOFKEYS;
   InitMaps(&kh);
   ClearAll();
}

/* 
   convert number to an ascii string in string
   return 1 if successful 0 otherwise
*/

int
myfta(char *string, double number)
{
   char e[DISPSIZE+1],s[DISPSIZE+1],s1[DISPSIZE+1];
   int dp,exp,i,j,k,ovf,sd;
   long l;

   if((!(ovf=isinf(number)))&&(!isnan(number)))
   {
      /* valid number */

      sprintf(s,DISPCONVEXP,number);

      /* remove trailing zeroes between '.' and 'e'
	 also remove 'e' & pad exp to three digits
	 */

      for(i=0;s[i]!='.';i++)
	 ;
      sd=1;
      dp=i;
      for(;s[i]!='e';i++,sd++)
	 ;
      sd--;
      k=i+1;
      if(k==(DISPSIZE-EXPSIZE))
      {
	 s[k-1]=s[k];
	 s[k]='0';
	 k--;
	 i--;
      }
      i--;
      for(j=0;s[i]=='0';i--,j++,sd--)
	 ;
      sd--;
      j++;
      s[++i]='\0';
      strcpy(s1,&s[k]);
      strcat(s,s1);
      s1[0]='\0';
      for(i=0;i<j;i++)
      {
	 dp++;
	 strcat(s1," ");
      }
      strcat(s1,s);
      strcpy(e,s1);
      exp=atoi(&e[DISPSIZE-EXPSIZE-1]);
      if(((((number<EXPHILIMIT)&&(number>EXPLOLIMIT))||
	   ((number>-(EXPHILIMIT/10))&&(number<-(EXPLOLIMIT/10)))||
	   (number==0))&&
	  (!engexp)&&
	  (((sd+_ABS(exp))<MAXINTDIGS)||(_ABS(number)>=1)))||
	 (base!=10))

	 /* non exponent format output */
   
      {
	 if(((((number>((double)ULONG_MAX))||(number<0))&&(!signconv))||
	     ((number>((double)LONG_MAX))||(number<((double)LONG_MIN))&&
	      (signconv)))
	    &&(base!=10))
	 {
	    return 0;
	 }
	 else
	 {
	    sd=0;
	    if(base==8)
	       sprintf(s,DISPCONVOCT,(unsigned int)number);
	    if(base==10)
	       sprintf(s,DISPCONV,number);
	    if(base==16)
	       sprintf(s,DISPCONVHEX,(unsigned int)number);
	    for(i=0;(s[i]!='\0')&&(s[i]!='.');i++)
	       if(isdigit(s[i]))
		  sd++;
	    if(s[i]=='.')
	    {

	       /* remove trailing zeroes after '.' */

	       j=strlen(s);
	       for(;s[i]!='\0';i++)
		  if((sd++)==MAXINTDIGS)
		     break;
	       s[i]='\0';
	       i--;
	       for(;s[i]=='0';i--)
		  ;
	       if(s[i]=='.')
		  i--;
	       s[++i]='\0';
	       j=j-strlen(s);
	       s1[0]='\0';
	       for(i=0;i<j;i++)
		  strcat(s1," ");
	       strcat(s1,s);
	       strcpy(s,s1);
	    }
	 }
      }
      else
      {
	 /* exponent format output */

	 strcpy(s,e);
	 if(engexp)
	 {
	    /* engineering exponent */

	    k=exp;

                          /*      +4                -4     */
	                  /*    j    k            j    k   */

	    j=3-(k%3);    /*    2    4            4    -4  */
	    if(j!=3)
	    {
	       if(k>=0)
		  j=3-j;  /*    1    4                     */
	       else
		  j=6-j;  /*                      2    -4  */
	       k=k-j;     /*    1    3            2    -6  */

	       /*  1  +  1  2  3
		  -5 -4 -3 -2 -1  DISPSIZE  EXPSIZE=3
		         1.23+
		*/

	       if(((DISPSIZE-EXPSIZE-3)-dp)<j)
	       {

		  /* fill with zeroes */

		  for(i=dp-1;i<(DISPSIZE-EXPSIZE-1);i++)
		     s[i-j]=s[i];
		  for(;i<(DISPSIZE-EXPSIZE-1+j);i++)
		     s[i-j]='0';
		  dp=dp-j;
	       }
	       for(i=0;i<j;i++)
	       {
		  s[dp]=s[dp+1];
		  s[++dp]='.';
	       }
	       if(k<0)
		  k=(-k);
	       sprintf(&s[DISPSIZE-EXPSIZE],EXPCONV,k);
	    }
	 }
      }
      strcpy(string,s);
      for(i=0;i<DISPSIZE;i++)
	 string[i]=toupper(string[i]);
      return 1;
   }
   else
   {
      /* infinity or not-a-number */

      if(isnan(number))
	 mymatherr(NANNAME);
      else
      {
	 if(ovf>0)
	    mymatherr(POSINFNAME);
	 else
	    mymatherr(NEGINFNAME);
      }
   }
   return 0;
}

RunProgram()
{
   HandleCalcKeys(program[pgmindex].kdp);
   INCPGM;
}

XregToDisplay()
{
   char str1[DISPSIZE+1],str2[DISPSIZE+1];
   int i,lcode;

   if(!learn)
   {
      if(myfta(display,xreg))
         RefreshDisplay(NULL);
      else
      {
	 RefreshDisplay(ZEROSTR);
	 mymatherr(CONVNAME);
	 /*	 mymatherr(STANDBYSTR);*/
      }
   }
   else
   {
      lcode=0;
      for(i=0;i<NOOFKEYS;i++)
      {
	 lcode=0;
	 if(program[pgmindex].kdp)
	 {
	    if(calckd[i].i1==program[pgmindex].kdp->i1)
	    {
	       if(program[pgmindex].second)
	       {
		  lcode=calckd[i].lrn+SECONDOFFSET;
		  if(!(lcode%10))
		     lcode=lcode-10;
		  break;
	       }
	       else
	       {
		  lcode=calckd[i].lrn;
		  break;
	       }
	    }
	 }
      }
      sprintf(str1,"%d",pgmindex);
      sprintf(str2,"%d",lcode);
      for(i=0;i<(PGMDISPSTEP-strlen(str1));i++)
         display[i]=' ';
      display[i]='\0';
      strcat(display,str1);
      strcat(display,PGMDISPSEP);
      strcat(display,str2);
      for(i=strlen(display);i<(DISPSIZE);i++)
         display[i]=' ';
      display[i]='\0';
      RefreshDisplay(NULL);
   }
}

/* locals */

static char *aboutmsg=
"ycalc version %s - %s\n\
Copyright (C) 1989-1998  Ulf Nordquist <un@pobox.com>\n\
This is free software, and you are welcome to redistribute\n\
it under certain conditions. ycalc comes with ABSOLUTELY\n\
NO WARRANTY. For details about redistribution and warranty\n\
see the file COPYING which you should have received a copy\n\
of along with this program, or it can be retrieved at\n\
ftp://prep.ai.mit.edu/pub/gnu/COPYING.\n\
Latest version can be found at www.pobox.com/~un/ycalc.html";

static void
AboutWin()
{
   char *buf;

   if(!(buf=malloc(strlen(aboutmsg)+100)))
      return;
   sprintf(buf,aboutmsg,VERSION,DATE);
   Requester(buf,NULL,NULL,0,0,calcwin);
   free(buf);
}

static
ClearAll()
{
   oprptr=xreg=0;
   ClearDisplay();
}

static double
ConvFromRad(double a)
{
   if(trigmode==RAD)
      return(a);
   else
      return(a*180/M_PI);
}
   
static double
ConvToRad(double a)
{
   if(trigmode==RAD)
      return(a);
   else
      return(a*M_PI/180);
}
   
/* check if display contains exponent
   return pointer to eponent sign (index)
   or     -1 if no exp
*/

static
DisplayChk()
{
   int i;

   for(i=DISPSIZE-1;(display[i]!='+')&&(display[i]!='-')&&(i>=0);i--)
      ;
   if((i!=(-1))&&(isdigit(display[i-1])||(display[i-1]=='.')))
      return(i);
   else
      return(-1);
}

static
HandleCalcKeys(struct keydef *lkdp)
{
   int entryinverse,entryaddrcnt,entrysecond,flag,i,j,k,key,lcode;
   char c,*cp,str1[STRLEN],str2[STRLEN];
   long l;
   double f;

   if(!lkdp)
      return;

   /* valid keydef ptr */

   key=NOKEY;
   if(!inverse)
   {
      if(!second)
	 key=lkdp->i1;
      else
	 key=lkdp->i2;
   }
   else
   {
      if(!second)
      {
	 if((lkdp->flags)&INV1ST)
	    key=lkdp->i1;
      }
      else
      {
	 if((lkdp->flags)&INV2ND)
	    key=lkdp->i2;
      }
      if(key==NOKEY)
      {
	 Requester(NOINVMSG,NULL,NULL,0,0,calcwin);
	 EndEntry();
      }
   }
   
   if(key!=PBOFF)
      standby=0;
   entrysecond=second;
   entryinverse=inverse;
   entryaddrcnt=addrcnt;
   lcode=lkdp->lrn;
   flag=lkdp->flags;
   if(learn&&(lcode!=PGMNODEF)&&(!((flag&ACT1ST)&&(!second)))&&
      (!((flag&ACT2ND)&&(second)))&&(lcode!=PGMNODEF))
   {
      /* learning, save key */

      if(key==PBLRN)
	 learn=0;
      else
      {
	 program[pgmindex].kdp=lkdp;
	 program[pgmindex].second=second;
	 INCPGM;
      }
      XregToDisplay();
   }
   else
   {
      switch(key)
      {
	 /* mode setters */

         case PBOFF:
            if(power)
	    {
	       EndEntry();
	       if(!standby)
	       {
		  standby=1;
		  RefreshDisplay(STANDBYSTR);
	       }
	       else
		  power=0;
	    }
	    break;
	 case PB2ND:
	    second=1;
	    DrawKeys(&kh,2);
	    break;
	 case PBINV:
	    inverse=1;
	    RefreshInverse();
	    break;
	 case PBEE:
	    if(newentry)
	       PushDigit('1');
	    if((DisplayChk()==(-1))&&(base==10))
	    {
	       /* if no exp notation, force it */

	       i=DISPSIZE-1;
	       while(i>=0)
	          if(display[i--]==' ')
	             break;
	       if(i>=2)
	       {
		  strcpy(str1,&display[EXPSIZE+1]);
		  strcat(str1,EXPZERO);
		  strcpy(display,str1);
		  enterexp=1;
		  RefreshDisplay(NULL);
	       }
	    }
	    break;
	 case PB1ST:
	    break;
	 case PBDEG:
	    trigmode=DEG;
	    RefreshTrigMode();
	    break;
	 case PBRAD:
	    trigmode=RAD;
	    RefreshTrigMode();
	    break;
	 case PBENG:
	    if(inverse)
	       engexp=0;
	    else
	       engexp=1;
	    EndEntry();
	    XregToDisplay();
	    break;
	 case PBLRN:
	    if(learn)
	       learn=0;
	    else
	    {
	       learn=1;
	       running=0;
	    }
	    XregToDisplay();

	    break;

	    /* enter one digit */

	 case PB0:
	    PushDigit('0');
	    break;
	 case PB1:
	    PushDigit('1');
	    break;
	 case PB2:
	    PushDigit('2');
	    break;
	 case PB3:
	    PushDigit('3');
	    break;
	 case PB4:
	    PushDigit('4');
	    break;
	 case PB5:
	    PushDigit('5');
	    break;
	 case PB6:
	    PushDigit('6');
	    break;
	 case PB7:
	    PushDigit('7');
	    break;
	 case PB8:
	    PushDigit('8');
	    break;
	 case PB9:
	    PushDigit('9');
	    break;
	 case PBA:
	    PushDigit('A');
	    break;
	 case PBB:
	    PushDigit('B');
	    break;
	 case PBC:
	    PushDigit('C');
	    break;
	 case PBD:
	    PushDigit('D');
	    break;
	 case PBE:
	    PushDigit('E');
	    break;
	 case PBF:
	    PushDigit('F');
	    break;
	 case PBPOINT:
	    if((base==10)&&(!point))
	    {
	       PushDigit('.');
	       point=1;
	    }
	    break;

	    /* operators */

	 case PBDIV:
	    PushOperator(OPRDIV);
	    break;
	 case PBMUL:
	    PushOperator(OPRMUL);
	    break;
	 case PBSUB:
	    PushOperator(OPRSUB);
	    break;
	 case PBADD:
	    PushOperator(OPRADD);
	    break;
	 case PBPOW:
	    PushOperator(OPRPOW);
	    break;
	 case PBMOD:
	    PushOperator(OPRMOD);
	    break;
	 case PBLPAR:
	    PushOperator(OPRPAR);
	    break;
	 case PBRPAR:
	    PopOperator();
	    break;
	 case PBEQUAL:
	    while(oprptr!=0)
               PopOperator();
	    break;

	    /* functions entering a value to the display */

	 case PBRCL:
	    InitMemEntry(key);
	    break;
	 case PBPI:
	    newentry=1;
	    enterexp=point=0;
	    xreg=M_PI;
	    XregToDisplay();
	    break;
	 case PBCE:
	    ClearDisplay();
	    break;
	 case PBCLR:
	    ClearAll();
	    break;
	 case PBCUT:
	    for(i=0;!isalnum(display[i]);i++)
	       ;
	    j=i;
	    while(display[j])
	    {
	       display[j]=tolower(display[j]);
	       j++;
	    }
	    XStoreBytes(calcdisp,&display[i],strlen(&display[i]));
	    break;
	 case PBPASTE:
	    cp=XFetchBytes(calcdisp,&i);
	    if(i>0)
	    {
	       for(j=0,k=0;(j<i)&&(k<DISPSIZE);j++)
	          if(MyIsDigit(str1[k]=toupper(*(cp+j)),base))
	             k++;
	       XFree(cp);
	       str1[k]='\0';
	       ClearDisplay();
	       strcpy(&display[DISPSIZE-k],str1);
	       RefreshDisplay(NULL);
	    }
	    break;

	    /* functions operating on display value */

	 case PBCHS:            /* does not EndEntry() */
	    if(!enterexp)
	    {
	       DisplayToXreg();
	       xreg=(-xreg);
	       XregToDisplay();
	    }
	    else
	    {
	       if(display[DISPSIZE-EXPSIZE-1]=='-')
	          display[DISPSIZE-EXPSIZE-1]='+';
	       else
	          display[DISPSIZE-EXPSIZE-1]='-';
	       RefreshDisplay(NULL);
	    }
	    break;
	 case PBOCT:
	    EndEntry();
	    base=8;
	    XregToDisplay();
	    RefreshBase();
	    break;
	 case PBDEC:
	    EndEntry();
	    base=10;
	    XregToDisplay();
	    RefreshBase();
	    break;
	 case PBHEX:
	    EndEntry();
	    base=16;
	    XregToDisplay();
	    RefreshBase();
	    break;
	 case PBFACT:
	    EndEntry();
	    if((xreg>170)||(xreg<0))
	       mymatherr(FACTNAME);
	    else
	    {
	       l=(long)xreg;
	       xreg=1;
	       for(i=1;i<=l;i++)
	          xreg=xreg*i;
	    }
	    XregToDisplay();
	    break;
	 case PBLNX:
	    EndEntry();
	    if(!inverse)
	    {
	       if(xreg<=0)
		  mymatherr(LNXNAME);
	       else
		  xreg=log(xreg);
	    }
	    else
	       xreg=exp(xreg);
	    XregToDisplay();
	    break;
	 case PBLOG:
	    EndEntry();
	    if(!inverse)
	    {
	       if(xreg<=0)
		  mymatherr(LOGNAME);
	       else
		  xreg=log10(xreg);
	    }
	    else
	       xreg=pow(10.0,xreg);
	    XregToDisplay();
	    break;
	 case PBSQRT:
	    EndEntry();
	    if(xreg<0)
	       mymatherr(SQRTNAME);
	    else
	       xreg=sqrt(xreg);
	    XregToDisplay();
	    break;
	 case PB1SX:
	    EndEntry();
	    if(xreg==0)
	       mymatherr(DIVNAME);
	    else
	       xreg=1/xreg;
	    XregToDisplay();
	    break;
	 case PBSQR:
	    EndEntry();
	    xreg=xreg*xreg;
	    XregToDisplay();
	    break;
	 case PBSIN:
	    EndEntry();
	    if(!inverse)
	       xreg=sin(ConvToRad(xreg));
	    else
	       xreg=ConvFromRad(asin(xreg));
	    XregToDisplay();
	    break;
	 case PBCOS:
            EndEntry();
	    if(!inverse)
               xreg=cos(ConvToRad(xreg));
	    else
	       xreg=ConvFromRad(acos(xreg));
	    XregToDisplay();
	    break;
	 case PBTAN:
	    EndEntry();
	    if(!inverse)
               xreg=tan(ConvToRad(xreg));
	    else
	       xreg=ConvFromRad(atan(xreg));
	    XregToDisplay();
	    break;
	 case PBXST:
	    EndEntry();
	    f=treg;
	    treg=xreg;
	    xreg=f;
	    XregToDisplay();
	    break;

	    /* enter value to memory */

	 case PBSTO:
	    EndEntry();
	    InitMemEntry(key);
	    break;
	 case PBSUM:
	    EndEntry();
	    InitMemEntry(key);
	    addrinv=inverse;
	    break;
	 case PBPRD:
	    EndEntry();
	    InitMemEntry(key);
	    addrinv=inverse;
	    break;
	 case PBEXC:
	    EndEntry();
	    InitMemEntry(key);
	    break;
	 case PBCMS:
	    InitMem();
	    RefreshMemDisplay();
	    break;

	    /* programming keys */

	 case PBSST:
	    if(learn)
	    {
	       pgmindex=(pgmindex+1)%LEARNSIZE;
	       XregToDisplay();
	    }
	    else
	       stepping=1;
	    break;
	 case PBBST:
	    if(learn)
	    {
	       if(pgmindex)
	          pgmindex--;
	       else
	          pgmindex=LEARNSIZE-1;
	       XregToDisplay();
	    }
	    break;

	    /* special buttons */

	 case PBABOUT:
	    AboutWin();
	    break;
	 case PBASCII:
	    OpenAscii();
	    break;
	 case PBDISPM:
	    OpenMem();
	    break;
	 case PBBIN:
	    OpenBin();
	    break;
	 case PBRST:
	    if(running||stepping)
	       pgmindex=LEARNSIZE-1;
	    else
	       pgmindex=0;
	    break;
	 case PBRS:
	    if(running)
	    {
	       running=0;
	       RefreshDisplay(NULL);
	    }
	    else
	    {
	       if(!stepping)
		  running=1;
	       RefreshDisplay(NULL);
	    }
	    break;
	 case PBCP:
	    if(!running)
	       ClrPgm();
	    break;
	 case PBGTO:
	    EndEntry();
	    InitPgmEntry(key);
	    break;
	 default:
	    break;
      }
      if(entryaddrcnt==addrcnt)
      {
	 addrent=0;
	 RefreshAddr();
	 addrcnt=MAXADDRESSENTRY;
      }
      if(addrent&&(addrcnt>=addrmaxcnt))
      {
	 switch(addrent)
	 {
            case PBSTO:
	       memory[address]=xreg;
	       RefreshMemDisplay();
	       break;
	    case PBRCL:
	       enterexp=point=0;
	       xreg=memory[address];
	       XregToDisplay();
	       break;
	    case PBSUM:
	       if(!addrinv)
                  memory[address]=memory[address]+xreg;
	       else
                  memory[address]=memory[address]-xreg;
	       RefreshMemDisplay();
	       break;
	    case PBPRD:
	       if(!addrinv)
                  memory[address]=memory[address]*xreg;
	       else
	       {
		  if(xreg==0)
		     mymatherr(DIVNAME);
		  else
		     memory[address]=memory[address]/xreg;
	       }
	       RefreshMemDisplay();
	       break;
	    case PBEXC:
	       enterexp=point=0;
	       f=memory[address];
	       memory[address]=xreg;
	       xreg=f;
	       XregToDisplay();
	       RefreshMemDisplay();
	       break;
	    case PBGTO:
	       if(running)
	       {
		  if(!address)
		     address=LEARNSIZE;
	          pgmindex=address-1;
	       }
	       else
	          pgmindex=address;
	       break;
	    default:
	       break;
	 }
	 addrent=0;
	 RefreshAddr();
      }
   }
   if((entrysecond)&&(key!=PBINV))
   {
      second=0;
      DrawKeys(&kh,1);
   }
   if((entryinverse)&&(key!=PB2ND))
   {
      inverse=0;
      RefreshInverse();
   }
}

InitMemEntry(key)
int key;
{
   newentry=1;
   addrent=key;
   address=addrcnt=0;
   addrtype=ATMEMORY;
   addrmaxcnt=MEMADDRESSDIGITS;
   RefreshAddr();
}

InitPgmEntry(key)
int key;
{
   newentry=1;
   addrent=key;
   address=addrcnt=0;
   addrtype=ATPROGRAM;
   addrmaxcnt=PGMADDRESSDIGITS;
   RefreshAddr();
}

static
MyIsDigit(char c,int b)
{
   int r;

   if(b<=10)
      r=((c>='0')&&(c<('0'+b)));
   else
      r=(((c>='0')&&(c<='9'))||((c>='A')&&(c<('A'+b-10))));
   return(r);
}

/* convert and copy display to xreg
   pop operator and operand from stack
   perform yreg .operator. xreg
   put result in display
*/

static
PopOperator()
{

   double yreg;
   long xl,yl;

   EndEntry();
   if(oprptr!=0)
   {
      yreg=opndstack[--oprptr];
      switch(oprstack[oprptr])
      {
         case OPRMUL:
	    xreg=yreg*xreg;
	 break;
         case OPRDIV:
	    if(xreg==0)
	       mymatherr(DIVNAME);
	    else
	       xreg=yreg/xreg;
	 break;
         case OPRADD:
	    xreg=yreg+xreg;
	 break;
         case OPRSUB:
	    xreg=yreg-xreg;
	 break;
         case OPRPOW:
	    xreg=pow(yreg,xreg);
	 break;
         case OPRMOD:
	    xl=xreg;
	    yl=yreg;
	    xreg=yl%xl;
	 break;
	 default:
	 break;
      }
      XregToDisplay();
   }
}

/* push a digit into display register from right 
   and refresh display
*/

static void
PushDigit(char d)
{
   int b,i;

   b='0';
   if(d>'9')
      b='A'-10;
   if((d-b)>=base)
      return;
   if((d>='0')&&(d<='9'))
   {
      if(addrent)
      {
	 address=(d-'0')+10*address;
	 addrcnt++;
	 RefreshAddr();
	 return;
      }
   }

   /* move string one step left */

   if(newentry)
   {
      ClearDisplay();
      newentry=0;
   }
   if(!enterexp)
   {
      if(display[0]==' ')
      {
	 for(i=0;display[i]==' ';i++)
            ;
	 if((i==DISPSIZE-1)&&(display[i]=='0'))
            display[i++]=' ';
      }
      else
	 i=0;
   }
   else
      i=DISPSIZE-EXPSIZE+1;
   for(;i<DISPSIZE;i++)
      display[i-1]=display[i];
   display[i-1]=d;
   RefreshDisplay(NULL);
}

/* if not '('
      while operator on stack and lower priority
         PopOperator()
   push operator and operand on stack
   convert and copy display to xreg
*/

static void
PushOperator(char f)
{
   newentry=1;
   enterexp=point=0;
   if(f!=OPRPAR)
      while((oprptr!=0)&&(priority[f]<=priority[oprstack[oprptr-1]]))
         PopOperator();
   if(oprptr==STACKSIZE)
   {
      Requester(STKOVFMSG,NULL,NULL,0,0,kh.win);
      ClearAll();
   }
   else
   {
      oprstack[oprptr]=f;
      DisplayToXreg();
      opndstack[oprptr++]=xreg;
   }
}

static
RefreshAddr()
{
   char s[MAXADDRESSENTRY+2];

   if(addrent&&(!running))
   {
      switch(addrtype)
      {
         case ATMEMORY:
	    s[0]='M';
	    break;
	 case ATPROGRAM:
	    s[0]='A';
	    break;
	 default:
	    break;
      }
      if(addrcnt)
         sprintf(&s[1],"%1d",address);
      else
         s[1]='\0';
      XDrawImageString(calcdisp,kh.win,gc,TXT4X,TXT4Y,s,strlen(s));
   }
   else
      XDrawImageString(calcdisp,kh.win,gc,TXT4X,TXT4Y,TXT4N,strlen(TXT4N));
}

static
RefreshBase()
{
   char s[4];

   sprintf(s,"%2d",base);
   XDrawImageString(calcdisp,kh.win,gc,TXT1X1,TXT1Y,s,strlen(s));
   RefreshMemDisplay();
}

static
RefreshDisplay(char *str)
{
   if(running)
   {
      XDrawImageString(calcdisp,kh.win,gc,CALCDISPX,CALCDISPY+fontb,
		       RUNSTR,strlen(RUNSTR));
   }
   else
   {
      if(str)
	 XDrawImageString(calcdisp,kh.win,gc,CALCDISPX,CALCDISPY+fontb,
			  str,strlen(str));
      else
	 XDrawImageString(calcdisp,kh.win,gc,CALCDISPX,CALCDISPY+fontb,
			  display,strlen(display));
   }
   RefreshAsciiDisplay();
   RefreshBinDisplay();
}

static
RefreshGadgets()
{
   DrawKeys(&kh,!second?1:2);
   XDrawLine(calcdisp,kh.win,gc,0,TXT1Y+(fonth/2-1),CALCW,TXT1Y+(fonth/2-1));
   XDrawLine(calcdisp,kh.win,gc,0,TXT1Y+(fonth/2+1),CALCW,TXT1Y+(fonth/2+1));
   XDrawLine(calcdisp,kh.win,gc,BCX-kdx-1,TXT1Y+(fonth/2+1),BCX-kdx-1,CALCH);
   XDrawLine(calcdisp,kh.win,gc,BCX-kdx+1,TXT1Y+(fonth/2+1),BCX-kdx+1,CALCH);
   XDrawRectangle(calcdisp,calcwin,gc,CALCDISPX-DISPFRAMEADDX,
		  CALCDISPY-DISPFRAMEADDY,DISPWIDTH+2*DISPFRAMEADDX,
		  fonth+fonth/2);
}

static
RefreshInverse()
{

   if(inverse)
      XDrawImageString(calcdisp,kh.win,gc,TXT3X,TXT3Y,TXT3I,strlen(TXT3I));
   else
      XDrawImageString(calcdisp,kh.win,gc,TXT3X,TXT3Y,TXT3N,strlen(TXT3N));
}

static
RefreshText()
{
   XDrawImageString(calcdisp,kh.win,gc,TXT1X,TXT1Y,TXT1,strlen(TXT1));
   RefreshBase();
   XDrawImageString(calcdisp,kh.win,gc,TXT2X,TXT2Y,TXT2,strlen(TXT2));
   RefreshTrigMode();
   RefreshDisplay(NULL);
   RefreshInverse();
   RefreshAddr();
}

static
RefreshTrigMode()
{

   if(trigmode==RAD)
      XDrawImageString(calcdisp,kh.win,gc,TXT2X1,TXT2Y,TXT2R,strlen(TXT2R));
   else
      XDrawImageString(calcdisp,kh.win,gc,TXT2X1,TXT2Y,TXT2D,strlen(TXT2D));
}
