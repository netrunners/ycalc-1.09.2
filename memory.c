/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Memory window handling

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
    901213 : moved from main.c
    910110 : InitMem();
    910318 : set min/max width/height in XSizeHints to get non-sticky
             windows
    960421 : DrawKeys() - new input parameter: map
    960617 : new key definition structs (major rewrite)
    970923 : do XRaiseWindow() if window is already open
    980226 : font sizes are variables
	     OpenMem() - since font sizes are variables now kd[] sizes
	                  and positions must be initialized by hand
    980317 : use forecolor & backcolor
*/

#include "Calc.h"
//#include "Keys.h"
#include "Main.h"
#include "Memory.h"

/* size and pos of memory

__________________________________________________________________
            ^                                                    |
         MEMYOFF                                                 |MEMENTH
            v                                                    |v
<- MEMXOFF ->[ memory  0 ]<- MEMXBTW ->[ memory 10 ]<- MEMXOFF ->|
             <- MEMENTW ->                                       |^
            ^
         MEMYBTW
            v
             [ memory  1 ]
*/

#define MEMCOLS      2               /* columns in window */
#define MEMROWS      10              /* rows in window */
#define MEMXOFF      (4*fontw)
#define MEMXBTW      (6*fontw)
#define MEMYOFF      fonth
#define MEMYBTW      (fonth/2)
#define MEMENTH      fonth
#define MEMENTW      (DISPSIZE*fontw)
#define MEMFRAMEADDX (fontw/3)       /* amount frame is outside memory entry */
#define MEMFRAMEADDY (fonth/3)       /* amount frame is outside memory entry */

/* size and pos of keys */

#define MEMKEYOFFX MEMXOFF         /* x offset to first key */
#define MEMKEYOFFY (2*MEMYOFF+MEMROWS*MEMENTH+(MEMROWS-1)*MEMYBTW+MEMFRAMEADDY)
#define MEMKW      ksx             /* width */
#define MEMKH      ksy             /* height */

/* size and pos */

#define MEMB       CALCB           /* border */
#define MEMH       (MEMKEYOFFY+MEMKH+MEMYOFF) /* height */
#define MEMW       (2*MEMXOFF+MEMCOLS*MEMENTW+(MEMCOLS-1)*MEMXBTW) /* width */
#define MEMTITLE   "Memory"        /* title */

/* key coords and text */
/* ident defines */

#define MXABS(N)   (N*(kdx+MEMKW)+kdx) /* x for button in row N */
#define MYABS(N)   (N*(kdy+MEMKW)+MEMKEYOFFY) /* y for button in line N */
#define MEMKEYS    3               /* number of keys */

#define PBMEMOFF   0
#define PBMEMSUB   1
#define PBMEMADD   2

static struct keyhandler kh;

/*  x coord   y coord   xsz    ysz      ident & index     flags   learn code
    -- upper left --                    primary   2nd
   init
    col       row        ignored
 */
static struct keydef kd[MEMKEYS+1]={
 0    ,0,0, 0,   PBMEMOFF, NOKEY, 0,            0,
 "Off",NULL,NULL,NULL,NULL,NULL,&kh,
 1    ,0,0, 0,   PBMEMSUB, NOKEY, 0,            0,
 " < ",NULL,NULL,NULL,NULL,NULL,&kh,
 2    ,0,0, 0,   PBMEMADD, NOKEY, 0,            0,
 " > ",NULL,NULL,NULL,NULL,NULL,&kh,};

static int memstart;
static int initialized=0;

double memory[MEMSIZE];

InitMem()
{
   int i;

   for(i=0;i<MEMSIZE;i++)
      memory[i]=0;
}

OpenMem()
{
   XSizeHints hints;
   int i,rx,ry,x,y;
   Window child,parent;
   unsigned int b,d,h,w;

   if(!initialized)
   {
      for(i=0;i<(MEMKEYS+1);i++)
      {
	 kd[i].x=MXABS(kd[i].x);
	 kd[i].y=MYABS(kd[i].y);
	 kd[i].w=MEMKW;
	 kd[i].h=MEMKH;
      }
      initialized=1;
   }
   if(kh.win)
   {
      XRaiseWindow(calcdisp,kh.win);
      return;
   }
   XGetGeometry(calcdisp,calcwin,&parent,&x,&y,&w,&h,&b,&d);
   hints.x=0;
   hints.y=0;
   hints.width=hints.min_width=hints.max_width=MEMW;
   hints.height=hints.min_height=hints.max_height=MEMH;
   hints.flags=USSize|PMinSize|PMaxSize|PPosition;
   if((kh.win=XCreateSimpleWindow(calcdisp,RootWindow(calcdisp,screen),
				  hints.x,hints.y,hints.width,hints.height,
				  MEMB,
				  forecolor.pixel,backcolor.pixel))==0)
      MyExit("Cannot open memory window");
   XSetStandardProperties(calcdisp,kh.win,MEMTITLE,NULL,None,NULL,0,&hints);
   XDefineCursor(calcdisp,kh.win,XCreateFontCursor(calcdisp,XC_hand2));
   XSelectInput(calcdisp,kh.win,
		ExposureMask|ButtonReleaseMask|ButtonPressMask);
   XMapWindow(calcdisp,kh.win);
   memstart=0;
   kh.def=kd;
   kh.noc=MEMKEYS;
}

RefreshMem()
{
   char s[DISPSIZE+1];
   int c,n,r,x,y;

   if(!kh.win)
      return;
   DrawKeys(&kh,1);
   n=memstart; 
   x=MEMXOFF;
   y=MEMYOFF;
   for(c=0;c<MEMCOLS;c++)
   {
      for(r=0;r<MEMROWS;r++)
      {
	 /* memory slot number */

	 sprintf(s,"%2d ",n);
	 XDrawImageString(calcdisp,kh.win,gc,x-3*fontw-MEMFRAMEADDX,
			  y+MEMENTH,s,strlen(s));

	 /* frame */

	 XDrawRectangle(calcdisp,kh.win,gc,x-MEMFRAMEADDX,y-MEMFRAMEADDY,
			MEMENTW+2*MEMFRAMEADDX,MEMENTH+2*MEMFRAMEADDY);
	 y=y+MEMENTH+MEMYBTW;
	 n++;
      }
      x=x+MEMENTW+MEMXBTW;
      y=MEMYOFF;
   }
   RefreshMemDisplay();
}

RefreshMemDisplay()
{
   char s[DISPSIZE+1];
   int c,n,r,x,y;

   if(!kh.win)
      return;
   DrawKeys(&kh,1);
   n=memstart;
   x=MEMXOFF;
   y=MEMYOFF;
   for(c=0;c<MEMCOLS;c++)
   {
      for(r=0;r<MEMROWS;r++)
      {
	 myfta(s,memory[n]);
	 XDrawImageString(calcdisp,kh.win,gc,x,y+MEMENTH,s,strlen(s));
	 y=y+MEMENTH+MEMYBTW;
	 n++;
      }
      x=x+MEMENTW+MEMXBTW;
      y=MEMYOFF;
   }
}

HandleMem(event)
XButtonEvent *event;
{
   int dummy,key;
   long cy,l,mask,reg;
   struct keydef *lkdp;

   if((!kh.win)||(event->window!=kh.win))
      return;

   /* window is open */

   switch(event->type)
   {
      case Expose:
         RefreshMem();
	 break;
      case ButtonPress:
      case ButtonRelease:
	 lkdp=HandleKeys(event,&kh,1);
	 if(lkdp)
	 {
	    key=lkdp->i1;
	    switch(key)
	    {
               case PBMEMOFF:
	          XDestroyWindow(calcdisp,kh.win);
		  kh.win=(Window)NULL;
		  break;
	       case PBMEMSUB:
		  if(memstart<(MEMROWS*MEMCOLS))
		     memstart=MEMSIZE-MEMROWS*MEMCOLS;
		  else
	             memstart=memstart-MEMROWS*MEMCOLS;
		  RefreshMem();
		  break;
	       case PBMEMADD:
		  if(memstart>(MEMSIZE-MEMROWS*MEMCOLS-1))
	             memstart=0;
		  else
	             memstart=memstart+MEMROWS*MEMCOLS;
		  RefreshMem();
		  break;
	       default:
		  break;
	    }
	 }
	 break;
      default:
	 break;
   }
}
