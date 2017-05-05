/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Startup and main event loop

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
    890301 : first
             about button & off button
             define 0-9, =, +, -, x, / buttons
    890302 : handling of digits
             define c, ce buttons
             handilng of c button
             start on =, +, -, x, / handling
    890303 : operator stack
    890307 : basic functions seems to work
             main function push button array
             DisplayToXreg()
             XregToDisplay()
             base conversion
    890308 : a-f for hex entry
             change sign
             do you really want to quit
             ascii conversion
    890309 : one memory
             factorial
             division with zero requester
    890328 : DrawKeys()
    890329 : GetKey(),
             HandleEditor -> HandleKeys()
             remove all Xr references, rewrite Requester()
    890330 : draw text in DrawKeys()
             box around display
    890331 : priority[]
             ()
    890403 : lnx, sqrt, y^x, SUM, 1/x, x^2
    890404 : key bitmap bitmap for sqrt, sqr, pow
             local mouse pointer
    890405 : 2nd, log, pi
    890407 : deg, rad, sin, cos, tan, asin, acos, atan
             ConvAngle()
    890410 : ConvFromRad(), ConvToRad()
             enter exponent
    890412 : access standby when quit
             trap floating point exceptions
             local matherr function
    890413 : cut and paste to display
    890414 : Exc, Prd
             No inverse defined requester
             inv lnx, inv log, inv prd, inv sum, CMs
    890420 : engineering exponent
    890428 : x<->t
             bitmap to about key
    890529 : paste filters according to base, MyIsDigit()
    890712 : bug1 in MyIsDigit() when base >= 10
    891003 : convert to X11
    891004 :   calcdisp, screen
               BlackPixel & WhitePixel changed to macro
               XCreateWindow() -> XCreateSimpleWindow()
	       RootWindow changed to macro
	       calcfont changed to Font
	       get font with XLoadFont()
	       create a GC
	       XText() -> XDrawImageString()
    891005 :   change various event names
               XQueryWindow() -> XGetWindowAttributes
	       XErrorHandler() -> XSeterrorHandler
	       use cursor font
	       XErrDescrip -> XGetErrorText
	       use XDrawLine() & XDrawRectangle(), no bmline
	       pass display to all X routies
	       no local error handler
    891006 :   adjust for XDrawImageString()'s use of baseline
    891208 :   Replace bitmap keys with font keys,
               abt, xst, sqrt, pow, sqr, pi & nul
	       XSetStandardProperties in Requester()
	       temp fix in Requester(), NULL routine
     891220 :   extend temp fix in Requester() to but msg is display
     900103 : Mod
     900301 : paste convert tolower
     900306 : convert to X11 (the sequel)
              magic with XChangeWindowAttributes() (to make images work)
	      convert bitmaps to X11 format
	      keyabt, keynul, keypi, keypow, keysqrt, keysqr, keyxst
	      display key bitmaps
     900307 : remove magic with XChangeWindowAttributes()
              further refinement of XImage init
     900314 : refresh graphics only in event loop
     900402 : workaround XNextEvent-return problem; move event handling
              from Requester() to main loop
	      handle all windows in same loop
     900406 : XEvent bug finally solved
              OpenAscii() when PBASCII is pressed
	      RefreshAscii()
     900409 : HandleAscii()
     900411 : RefreshAsciiDisplay()
              RefreshDisplay()
              HandleKeys() - key as input parameter
	      HandleAscii() - ascii table key press to calc key press
	      bug in display ascii - when base==10 and display>0xffffffff
	      off key in ascii table
	      only one requester window at a time
     900419 : OpenBin() when PBBIN is pressed
              RefreshBin()
	      HandleBin()
	      RefreshAsciiDisplay()
	      DrawKeys() - draw rectangle if no string
	      HandleBin() - off, shift left, shift right
     900420 : HandleBin() - rotate left & right
     900504 : XregToDisplay() - remove base==2 code
                                error handling if conversion overflow
				exp notation if loss of decimals
				better handling of rounding errors
	      ingnore keypress other than requester if requester window
	      is active
     900515 : three registers in binary window, binregs[]
              HandleBin() - select calculator display echo register
     900516 : HandleBin() - and or xor inv
     900601 : SaveHistory()
              PBHST
     900608 : further refinements of history
              store internal window identifier instead of window handle
     900614 : more than one memory
              HandleKeys() - change handling of digit 0-9 and handling
                             of sto rcl sum exc prd
	      RefreshMemAddr()
     900615 : PBDISPM - Memory window
              OpenMem()
	      skeleton RefreshMem() & RefreshMemDisplay() & HandleMem()
     900628 : RefreshMemDisplay() - display first memory locations
     900629 : HandleMem() - PBMEMOFF PBMEMSUB PBMEMADD
              RefreshMem() - display keys
     900703 : RefreshMem() - display frame around memory value
     900711 : PBLRN - start learn mode
     900914 : learn keycode in keycoord
     900925 : handle learn in HandleKeys()
              PBSST PBBST
	      GetKey() - new parameter : int *index
	      XregToDisp() - handle learn modes
     901023 : bases != 10 are unsigned, clean up somewhat
              PBRS PBRST
     901024 : run program
     901101 : keymaptxt[] & keymaptxt2nd[]
              save keycode when learn, use TI-59 only in display
     901108 : PBCP
     901116 : #defines to Main.h
     901212 : ascii window parts to ascii.c
     901213 : binary window parts to binary.c
              memory window parts to memory.c
     901220 : key and requester handling to keys.c
     910109 : RefreshDisplay() - input argument
              InitMemEntry()
     910110 : generic address entry
              PBGTO
	      save state in file at off in $HOME/.xcalcrc
	      restore state from file at on in $HOME/.xcalcrc
	      remove history
     910221 : use XCreateWindow() instead of XCreateSimpleWindow()
     910318 : set min/max width/height in XSizeHints to get non-sticky
              windows
     910430 : invgc
     941106 : port to linux
               change exception handling
	       trap FPE manually
	       rewrite matherr()
     950506 : move calc stuff to calc.c
              rename rc file to $HOME/.ycalcrc
     960426 : move running of program to calc.c
     970923 : get key press/release events from calcwin
     970927 : move AboutWin() and assciated stuff to calc.c
     980123 : rename matherr() to mymatherr()
     980205 : stepping
     980213 : (friday)
              open calcwin with XCreateSimpleWindow(), make it impossible, or
	       at least hard, to change size of calcwin
     980215 : .ycalcrc for settings and .ycalcmem for memory
              handle old .ycalcrc (version 1)
              .ycalcmem human readable
     980225 : realized that save program memory hasn't worked since program[]
               became an array of struct
     980226 : font sizes and name are variables
              read font name from rcfile
     980317 : read back and foreground color from rcfile
     980331 : renamed .ycalcrc to .ycalc to avoid it being overwritten by older
               versions of ycalc (found this the hard way)
              possible to set signed base conversion in SETFILE
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "Calc.h"
//#include "Keys.h"
#include "Main.h"
#include "Memory.h"

#define FPEMSG        "Floating point exception"
#define MATHERRMSG    "Math library error : "

#define SETFILE       ".ycalc"        /* file to read settings from */
#define MEMFILE       ".ycalcmem"     /* file to save/restore memory from */
#define LEGACYFILE    ".ycalcrc"      /* legacy rcfile */
#define LEGACYVERSION 1               /* legacy version of rcfile */

#define MEMVERSION    2               /* version of memfile */

#define DEFAULTFONT   "6x10"
/*#define FONT       "/users/un/src/c/x/my6x10.onx"*/

#define SETFILEDELIM   " \t\n"         /* delimiters for SETFILE */

/* global variables */

Display *calcdisp;
GC gc,invgc;
Window calcwin;
char *prg;
int power,screen;
int useimages;           /* use key images if available */
XColor backcolor,forecolor;
int signconv;            /* base conversion is signed */

/* font */

static char fontname[500];  /* font points here */

static char *font;    /* name/file */
int fontb;     /* */
int fonth;     /* font height */
int fontw;     /* font width */
int kdx;       /* x between keys */
int kdy;       /* y between keys */
int ksx;       /* x key size */
int ksy;       /* y key size */

#if 0
#define FONTB      10
#define FONTH      10
#define FONTW      6

/* keys */

#define KDX        FONTW           /* x between keys */
#define KDY        (FONTH/2)       /* y between keys */
#define KSX        (FONTW*4)       /* x key size, 24 */
#define KSY        (FONTH+4)       /* y key size, 14 */
#endif

/* local variables */

static haveoldrcfile;
static char backcolorname[500];  /* background color name */
static char forecolorname[500];  /* foreground color name */

/* function prototypes */

void MySignalHandler();
double ConvFromRad(),ConvToRad();

static FILE *OpenSaveFile(char *, char *);
static void readmemfile();
static void writememfile();
static void oldRestoreFromFile();
static void readsetfile();

main(argc,argv)
int argc;
char *argv[];
{
   int dummy,eventfound,i,runkey,ascret,desret,dirret,pixel,r;
   char buf[10];
   XSetWindowAttributes attr;
   XEvent event;
   unsigned long valuemask;
   XSizeHints hints;
   XGCValues gcval;
   XCharStruct xchar;
   XFontStruct *calcfont;
   Colormap colormap;

   prg=argv[0];

   /* set up local signal handler for floating point exception */

   signal(SIGFPE,MySignalHandler);

   /* connect to server */

   if((calcdisp=XOpenDisplay(NULL))==0)
   {
      MyExit("Cannot open display");
      exit(1);
   }
   screen=DefaultScreen(calcdisp);
   colormap=DefaultColormap(calcdisp,screen);

   /* settings */
   /* defaults */

   strcpy(fontname,DEFAULTFONT);
   font=fontname;
   backcolor.pixel=WhitePixel(calcdisp,screen);
   forecolor.pixel=BlackPixel(calcdisp,screen);
   signconv=0;

   /* from file */

   readsetfile();

   /* font */

   if(!(calcfont=XLoadQueryFont(calcdisp,font)))
   {
      printf("%s: Cannot open font '%s'\n",argv[0],font);
      exit(1);
   }

   /* calculate font dependent values */

   XTextExtents(calcfont,"A",1,&dirret,&ascret,&desret,&xchar);
   fonth=ascret+desret;
   fonth=MAX(ascret,xchar.ascent)+MAX(desret,xchar.descent);
   fontw=xchar.width;
   fontb=fonth;
   kdx=fontw;
   kdy=fonth/2;
   ksx=fontw*4;
   /*   ksy=fonth+4;*/
   ksy=fonth+fonth/2;
   useimages=((ksx==24)&&(ksy==15));

   /* allocate colors */

   backcolor.flags=DoRed|DoGreen|DoBlue;
   pixel=backcolor.pixel;
   r=0;
   if(backcolorname[0]=='#')
   {
      if(sscanf(backcolorname,"#%02x%02x%02x",
		(unsigned int*)&backcolor.red,
		(unsigned int*)&backcolor.green,
		(unsigned int*)&backcolor.blue)==3)
	 r=XAllocColor(calcdisp,colormap,&backcolor);
   }
   else
      r=XAllocNamedColor(calcdisp,colormap,backcolorname,&backcolor,
			 &backcolor);
   if(!r)
      backcolor.pixel=pixel;


   pixel=forecolor.pixel;
   r=0;
   if(forecolorname[0]=='#')
   {
      if(sscanf(forecolorname,"#%02x%02x%02x",
		(unsigned int*)&forecolor.red,
		(unsigned int*)&forecolor.green,
		(unsigned int*)&forecolor.blue)==3)
	 r=XAllocColor(calcdisp,colormap,&forecolor);
   }
   else
      r=XAllocNamedColor(calcdisp,colormap,forecolorname,&forecolor,
			 &forecolor);
   if(!r)
      forecolor.pixel=pixel;

   /* open & set up calculator window */

   if(!(calcwin=XCreateSimpleWindow(calcdisp,RootWindow(calcdisp,screen),
				    0,0,
				    CALCW,CALCH,CALCB,
				    forecolor.pixel,backcolor.pixel)))
      MyExit("Cannot open calculator window");
   hints.x=0;
   hints.y=0;
   hints.width=hints.min_width=hints.max_width=CALCW;
   hints.height=hints.min_height=hints.max_height=CALCH;
   hints.flags=USSize|PMinSize|PMaxSize|PPosition;
   XSetStandardProperties(calcdisp,calcwin,CALCTITLE,CALCTITLE,None,NULL,0,
			  &hints);
   XDefineCursor(calcdisp,calcwin,XCreateFontCursor(calcdisp,XC_hand2));
   XSelectInput(calcdisp,calcwin,ExposureMask|ButtonPressMask|
		ButtonReleaseMask|KeyPressMask|KeyReleaseMask);

   /* set up normal graphical context */

   gcval.foreground=forecolor.pixel;
   gcval.background=backcolor.pixel;
   gcval.function=GXcopy;
   gcval.font=calcfont->fid;
   gc=XCreateGC(calcdisp,RootWindow(calcdisp,screen),
		GCForeground|GCBackground|GCFont|GCFunction,&gcval);

   /* set up inverse graphical context */

   gcval.foreground=backcolor.pixel;
   gcval.background=forecolor.pixel;
   invgc=XCreateGC(calcdisp,RootWindow(calcdisp,screen),
		   GCForeground|GCBackground|GCFont|GCFunction,&gcval);

   /* init variables */

   power=1;
   InitCalc();
   readmemfile();

   /* main loop */

   XSetNormalHints(calcdisp,calcwin,&hints);
   XMapWindow(calcdisp,calcwin);
   while(power)
   {
      if(!running)
      {
	 XNextEvent(calcdisp,&event);
	 eventfound=1;
      }
      else
         eventfound=XCheckMaskEvent(calcdisp,
				    ExposureMask|
				    ButtonReleaseMask|ButtonPressMask|
				    KeyReleaseMask|KeyPressMask,
				    &event);
      if(eventfound)
      {
	 /* an event was found */

	 if(!HandleReq(&event))
	 {
	    /* if no requester */

	    HandleCalc(&event);
	    HandleAscii(&event);
	    HandleBin(&event);
	    HandleMem(&event);
	 }
      }
      if(running||stepping)
      {
	 RunProgram();
	 if(stepping)
	    stepping=0;
      }
   }
   writememfile();
   MyExit(NULL);
}

MyExit(s)
char *s;
{
   if(s==NULL)
      exit(0);
   else
   {
      printf("\n%s: %s\n",prg,s);
      exit(1);
   }
}


/* local handler of signals */

void
MySignalHandler()
{
   signal(SIGFPE,SIG_IGN);
   Requester(FPEMSG,NULL,NULL,0,0,calcwin);
   signal(SIGFPE,MySignalHandler);
}

/* local handler of math lib errors */

mymatherr(str)
char *str;
{
   char msg[STRLEN];

   strcpy(msg,MATHERRMSG);
   strcat(msg,str);
   Requester(msg,NULL,NULL,0,0,calcwin);
}

/* file handling */

static FILE *
OpenSaveFile(char *mode, char *name)
{
   char *cp,s[100];
   FILE *fp;

   if(!(cp=getenv("HOME")))
      return NULL;
   strcpy(s,cp);
   strcat(s,"/");
   strcat(s,name);
   if(!(fp=fopen(s,mode)))
      return NULL;
   return fp;
}

/* mem file */
   
/* file format version 2
   first line : ascii version number
   P<step number> <pgm code>
   M<mem number>  <mem content>
*/

static void
readmemfile()
{
   char buf[500];
   FILE *fp;
   int code,fail,num,ver;
   double cont;

   fail=1;
   InitMem();
   ClrPgm();
   if(fp=OpenSaveFile("r",MEMFILE))
   {
      /* check version of file */

      if(fgets(buf,500,fp))
	 if(sscanf(buf,"%d",&ver)==1)
	    if(ver==MEMVERSION)
	       fail=0;

      /* read body of file */

      while((fgets(buf,500,fp))&&(!fail))
      {
	 *buf=toupper(*buf);
	 if(sscanf(buf,"M%d %lf",&num,&cont)==2)
	 {
	    /* memory */

	    if((num>=0)&&(num<MEMSIZE))
	       memory[num]=cont;
	    continue;
	 }
	 if(sscanf(buf,"P%d %d",&num,&code)==2)
	 {
	    int i;

	    /* program */

	    for(i=0;i<NOOFKEYS;i++)
	       if(calckd[i].lrn==code)
		  break;
	    if((i<NOOFKEYS)&&((num>=0)&&(num<LEARNSIZE)))
	    {
	       program[num].kdp= &calckd[i];
	       if(((code%10)>SECONDOFFSET)||(!(code%10)))
		  program[num].second=1;
	    }
	    continue;
	 }
      }
      fclose(fp);
   }
   if(fail)
   {
      /* only try old type mem file if no new can be loaded */

      oldRestoreFromFile();
   }
}

static void
writememfile()
{
   char *cp,format[50];
   FILE *fp;
   int code,i;

   if(!(fp=OpenSaveFile("w",MEMFILE)))
      return;
   fprintf(fp,"%d # Version number\n",MEMVERSION);

   /* program memory */

   fprintf(fp,"#\n# Program\n#\n");
   for(i=0;i<LEARNSIZE;i++)
      if(program[i].kdp)
      {
	 code=program[i].kdp->lrn;
	 if(program[i].second)
	 {
	    code=code+SECONDOFFSET;
	    if(!(code%10))
	       code=code-10;
	 }
	 fprintf(fp,"P%03d %d\n",i,code);
      }

   /* storage memory */

   fprintf(fp,"#\n# Memory\n#\n");
   sprintf(format,"M%%02d %s\n",DISPCONVEXP);
   for(i=0;i<MEMSIZE;i++)
      if(memory[i]!=0.0)
	 fprintf(fp,format,i,memory[i]);
   fclose(fp);
}
   
/*
  old file format
  one                      byte  : version number
  LEARNSIZE * sizeof(int)  bytes : program      
  MEMSIZE * sizeof(double) bytes : memory
*/

static void
oldRestoreFromFile()
{
   char *cp;
   FILE *fpr;
   int fail,i,j;

   fail=1;
   if(fpr=OpenSaveFile("r",LEGACYFILE))
   {
      fail=0;
      if(getc(fpr)==LEGACYVERSION)
      {
	 cp=(char*)program;
	 for(i=0;i<(LEARNSIZE*sizeof(int));i++)
	 {
	    if((j=getc(fpr))==EOF)
	    {
	       fail=1;
	       break;
	    }
	    /*	    *(cp++)=j;*/
	    *(cp++)=0;
	 }
	 if(!fail)
	 {
	    cp=(char*)memory;
	    for(i=0;i<(MEMSIZE*sizeof(double));i++)
	    {
	       if((j=getc(fpr))==EOF)
	       {
		  fail=1;
		  break;
	       }
	       *(cp++)=j;
	    }
	 }
      }
   }
   if(fail)
   {
      InitMem();
      ClrPgm();
   }
   if(fpr)
      fclose(fpr);
}

/* settings file */

static void
readsetfile()
{
   FILE *fp;
   char buf[500],*cp;

   if(fp=OpenSaveFile("r",SETFILE))
   {
      while(fgets(buf,500,fp))
      {
	 if(cp=strtok(buf,SETFILEDELIM))
	 {
	    if(!strcmp("font",cp))
	    {
	       cp=strtok(NULL,SETFILEDELIM);
	       strcpy(fontname,cp);
	       continue;
	    }
	    if(!strcmp("color",cp))
	    {
	       if(cp=strtok(NULL,SETFILEDELIM))
	       {
		  if(!strcmp("back",cp))
		  {
		     cp=strtok(NULL,SETFILEDELIM);
		     strcpy(backcolorname,cp);
		     continue;
		  }
		  if(!strcmp("fore",cp))
		  {
		     cp=strtok(NULL,SETFILEDELIM);
		     strcpy(forecolorname,cp);
		     continue;
		  }
	       }
	    }
	    if(!strcmp("conversion",cp))
	    {
	       if(cp=strtok(NULL,SETFILEDELIM))
	       {
		  if(!strcmp("signed",cp))
		  {
		     signconv=1;
		     continue;
		  }
	       }
	    }
	 }
      }
      fclose(fp);
   }
}
