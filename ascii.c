/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Ascii window handling

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
    901212 : moved from xcalc.c
    910318 : set min/max width/height in XSizeHints to get non-sticky
             windows
    920402 : HandleAscii() - check newentry flag
    960421 : DrawKeys() - new input parameter: map
    960617 : new key definition structs (major rewrite)
    970919 : continue major rewrite
    970921 : HandleAscii() - use HandleKeys()
    970923 : do XRaiseWindow() if window is already open
    980226 : font sizes are variables
    980317 : use forecolor & backcolor
*/

#include <limits.h>

#include "Keys.h"
#include "Main.h"

/* table keys */

#define ASCTKX     1               /* x offset for first key */
#define ASCTKY     (fonth*4)       /* y offset for first key */
#define ASCTKW     ksx             /* key width */
#define ASCTKH     ksy             /* key height */
#define ASCTKEYS   129             /* total number of keys */
#define ASCTROWS   16              /* number of key rows */
#define ASCTCOLS   8               /* number of key columns */
#define ASCTCHARS  6               /* number of chars per key incl '\0' */

/* size and pos */

#define ASCTXOFF   5               /* x offset from calculator window */
#define ASCTYOFF   0               /* y offset from calculator window */
#define ASCTB      CALCB           /* border */
#define ASCTW      (ASCTCOLS*ASCTKW+ASCTKY) /* width */
#define ASCTH      ((ASCTROWS+1)*ASCTKH+ASCTKY) /* height */
#define ASCTTITLE  "ASCII Table"   /* title */

/* fixed text */

#define ATXT1X     (ASCTKX+3*ASCTKW/2) /* horizontal table numbers */
#define ATXT1Y     (3*ASCTKY/4)
#define ATXT2X     (ASCTKX+ASCTKW/2) /* vertical table numbers */
#define ATXT2Y     (ASCTKY+ASCTKH)
#define ATXT3X     (ASCTKX+ASCTKW) /* ascii echo of display */
#define ATXT3Y     (ASCTKY-2*ASCTKH)

/* special keys */

#define PBASCTOFF  128
#define ASCTOFFX   (ASCTKX+ASCTKW*ASCTCOLS)
#define ASCTOFFY   (ATXT3Y-ASCTKH/2)
#define ASCTOFFT   "Off"

static struct keydef kd[ASCTKEYS];
static char *asciitxtptr;
static char *asciictrltxt[0x20]=
{
   "NUL","SOH","STX","ETX",
   "EOT","ENQ","ACK","BEL",
   " BS"," HT"," LF"," VT",
   " FF"," CR"," SO"," SI",
   "DLE","DC1","DC2","DC3",
   "DC4","NAK","SYN","ETB",
   "CAN"," EM","SUB","ESC",
   " FS"," GS"," RS"," US"
};
static char asciidisplay[5];
static struct keyhandler kh;

void
OpenAscii(void)
{
   XWindowAttributes info;
   XSizeHints hints;
   int i,x,y;
   char *cptr;

   if(kh.win)
   {
      XRaiseWindow(calcdisp,kh.win);
      return;
   }
   XGetWindowAttributes(calcdisp,calcwin,&info);
   if((kh.win=XCreateSimpleWindow(calcdisp,RootWindow(calcdisp,screen),
				  info.x+info.width+ASCTXOFF,info.y+ASCTYOFF,
				  ASCTW,ASCTH,ASCTB,
				  forecolor.pixel,backcolor.pixel))==0)
      MyExit("Cannot open Ascii table window");
   hints.x=0;
   hints.y=0;
   hints.width=hints.min_width=hints.max_width=ASCTW;
   hints.height=hints.min_height=hints.max_height=ASCTH;
   hints.flags=USSize|PMinSize|PMaxSize|PPosition;
   XSetStandardProperties(calcdisp,kh.win,ASCTTITLE,NULL,None,NULL,0,&hints);
   XDefineCursor(calcdisp,kh.win,XCreateFontCursor(calcdisp,XC_hand2));
   XSelectInput(calcdisp,kh.win,
		ExposureMask|ButtonReleaseMask|ButtonPressMask);
   XMapWindow(calcdisp,kh.win);

   /* set up key coordinates and texts */

   cptr=asciitxtptr=malloc(ASCTCHARS*ASCTKEYS);
   x=ASCTKX;
   for(i=0;i<(ASCTROWS*ASCTCOLS);i++)
   {
      if(!(i%ASCTROWS))
      {
	 y=ASCTKY;
	 x=x+ASCTKW;
      }

      kd[i].x=x;
      kd[i].y=y;
      kd[i].w=ASCTKW;
      kd[i].h=ASCTKH;
      kd[i].i1=i;
      kd[i].handler= &kh;
      y=y+ASCTKH;
      kd[i].txt1=cptr;
      if(i>=0x20)
      {
	 *cptr=' ';
	 *(cptr+1)=i;
	 *(cptr+2)=' ';
	 *(cptr+3)='\0';
      }
      else
	 strcpy(cptr,asciictrltxt[i]);
      cptr=cptr+ASCTCHARS;
   }

   /* off key */

   kd[i].x=ASCTOFFX;
   kd[i].y=ASCTOFFY;
   kd[i].w=ASCTKW;
   kd[i].h=ASCTKH;
   kd[i].i1=i;
   kd[i].handler= &kh;
   kd[i].txt1=cptr;
   strcpy(cptr,ASCTOFFT);
   kh.def=kd;
   kh.noc=ASCTKEYS;
}

RefreshAscii()
{
   int i;
   char s[10];

   if(!kh.win)
      return;
   RefreshAsciiDisplay();
   for(i=0;i<(ASCTCOLS);i++)
   {
      s[0]='0'+i;
      XDrawImageString(calcdisp,kh.win,gc,ATXT1X+i*ASCTKW,ATXT1Y,s,1);
   }
   for(i=0;i<(ASCTROWS);i++)
   {
      s[0]='0'+i;
      if(i>9)
         s[0]=s[0]+7;
      XDrawImageString(calcdisp,kh.win,gc,ATXT2X,ATXT2Y+i*ASCTKH,s,1);
   }
   DrawKeys(&kh,1);
}

RefreshAsciiDisplay()
{
   char s[10];
   unsigned long l;
   unsigned char c;
   int i;

   if(!kh.win)
      return;
   DisplayToXreg();
   if(xreg>((double)ULONG_MAX))
      l=ULONG_MAX;
   else
   {
      if(xreg<0.0)
         l=0;
      else
         l=(unsigned long)xreg;
   }
   s[sizeof(l)]='\0';
   for(i=0;i<sizeof(l);i++)
   {
      c=(l&0xff000000)>>((sizeof(l)-1)*8);
      if((c>=' ')&&(c<=0x7e))
         s[i]=c;
      else
         s[i]='.';
      l=l<<8;
   }
   strcpy(asciidisplay,s);
   XDrawImageString(calcdisp,kh.win,gc,ATXT3X,ATXT3Y,asciidisplay,
		    strlen(asciidisplay));
}

HandleAscii(event)
XButtonEvent *event;
{
   unsigned long l;
   struct keydef *lkdp;
   int key;

   if((!kh.win)||(event->window!=kh.win))
      return;

   /* window is open and event is for this window */

   switch(event->type)
   {
      case Expose:
         RefreshAscii();
	 break;
      case ButtonPress:
      case ButtonRelease:
	 lkdp=HandleKeys(event,&kh,1);
	 if(lkdp)
	 {
	    key=lkdp->i1;
	    switch(key)
	    {
	       case PBASCTOFF:
	          XDestroyWindow(calcdisp,kh.win);
		  kh.win=(Window)NULL;
		  break;
	       default:
		  if(newentry)
		  {
		     ClearDisplay();
		     newentry=0;
		  }
		  DisplayToXreg();
		  if(xreg>((double)ULONG_MAX))
		     l=ULONG_MAX;
		  else
		  {
		     if(xreg<0.0)
		        l=0;
		     else
		        l=(unsigned long)xreg;
		  }
		  if(l>0xffffff)
		     l=l&0xffffff;
		  l=l*0x100+key;
		  xreg=(double)l;
		  XregToDisplay();
		  break;
	    }
	 }
	 break;
      default:
	 break;
   }
}
