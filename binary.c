/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Binary window handling

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
    901213 : moved from Main.c
    910318 : set min/max width/height in XSizeHints to get non-sticky
             windows
    960421 : DrawKeys() - new input parameter: map
    960617 : new key definition structs (major rewrite)
    970919 : continue major rewrite
    970921 : HandleBin(event) - use HandleKeys()
    970923 : do XRaiseWindow() if window is already open
    971109 : RefreshBinDisplay() - put number at register displays
    980226 : font sizes are variables
	     OpenBin() - since font sizes are variables now initkd[] sizes
	                  and positions must be initialized by hand
    980317 : use forecolor & backcolor
*/

#include <limits.h>

#include "Keys.h"
#include "Main.h"

/* bit keys */

#define BINBH      fonth           /* bit height */
#define BINBW      BINBH           /* bit width */
#define BINBOW     2               /* x offset to bit filling */
#define BINBOH     2               /* y offset to bit filling */
#define BINBX      BINBW           /* x offset for first bit */
#define BINBY      (2*BINBH)       /* y offset for first bit */
#define BINREGBITS 32              /* bits in one register */
#define BINREGS    3               /* total number of registers */
#define BINREGOPR1 2               /* binary register first operand */
#define BINREGOPR2 1               /* binary register second operand */
#define BINREGRES  0               /* binary register result */
#define BINBITS    (BINREGS*BINREGBITS) /* total number of bits */

/* other keys, size & pos */

#define BINOKEYS   14               /* total number of other keys */
#define BINKEYS    (BINBITS+BINOKEYS) /* total number of keys */
#define BINKW      ksx             /* key width */
#define BINKH      ksy             /* key height */
#define BXABS(N)   (N*(kdx+BINKW)+kdx) /* x for button in row N */
#define BYABS(N)   (N*(kdy+BINKW)+kdy) /* y for button in line N */
#define BINSX      (BINBX+(BINREGBITS+1)*BINBW) /* x for register select */
#define BINSY      BINBY           /* y for register select */

/* ident defines */

#define PBBINOFF   (BINBITS+0)
#define PBBINSHL   (BINBITS+1)
#define PBBINSHR   (BINBITS+2)
#define PBBINROL   (BINBITS+3)
#define PBBINROR   (BINBITS+4)
#define PBBINASL   (BINBITS+5)
#define PBBINASR   (BINBITS+6)
#define PBBINOPR1  (BINBITS+7)
#define PBBINOPR2  (BINBITS+8)
#define PBBINRES   (BINBITS+9)
#define PBBINAND   (BINBITS+10)
#define PBBINOR    (BINBITS+11)
#define PBBINXOR   (BINBITS+12)
#define PBBININV   (BINBITS+13)

/* size and pos */

#define BINXOFF    0                     /* x offset from calculator window */
#define BINYOFF    5                     /* y offset from calculator window */
#define BINB       CALCB                 /* border */
#define BINW       (BINSX+BINBW+BINBW/4) /* width */
#define BINH       BYABS(4)              /* height */
#define BINTITLE   "Binary"              /* title */

/* fixed text */

#define BTXT1      "Shift"
#define BTXT1X     (BXABS(1)+BINKW/2+3)
/*#define BTXT1Y     (BYABS(3)-4)*/
#define BTXT1Y     (BYABS(3))
#define BTXT2      "Rotate"
#define BTXT2X     (BXABS(3)+BINKW/2)
#define BTXT2Y     BTXT1Y
#define BTXT3      "A Shift"
#define BTXT3X     (BXABS(5)+BINKW/2-8)
#define BTXT3Y     BTXT1Y

static struct keydef kd[BINKEYS];
static unsigned long binregs[BINREGS];  /* registers in binary window */
static int curbinreg;                /* echo register for calculator display */

/* other keys init values */

/*  x coord   y coord   xsz    ysz      ident & index     flags   learn code
    -- upper left --                    primary   2nd
   init
    col       row        ignored
 */

static struct keydef initkd[BINOKEYS]={
 0,   2, 0, 0,   PBBINAND, NOKEY, 0,            0,
 "And",NULL,NULL,NULL,NULL,NULL,NULL,
 1,   2, 0, 0,   PBBINOR,  NOKEY, 0,            0,
 "Or ",NULL,NULL,NULL,NULL,NULL,NULL,
 2,   2, 0, 0,   PBBINXOR, NOKEY, 0,            0,
 "Xor",NULL,NULL,NULL,NULL,NULL,NULL,
 0,   3, 0, 0,   PBBINOFF, NOKEY, 0,            0,
 "Off",NULL,NULL,NULL,NULL,NULL,NULL,
 1,   3, 0, 0,   PBBINSHL, NOKEY, 0,            0,
 " < ",NULL,NULL,NULL,NULL,NULL,NULL,
 2,   3, 0, 0,   PBBINSHR, NOKEY, 0,            0,
 " > ",NULL,NULL,NULL,NULL,NULL,NULL,
 3,   3, 0, 0,   PBBINROL, NOKEY, 0,            0,
 " < ",NULL,NULL,NULL,NULL,NULL,NULL,
 4,   3, 0, 0,   PBBINROR, NOKEY, 0,            0,
 " > ",NULL,NULL,NULL,NULL,NULL,NULL,
 5,   3, 0, 0,   PBBINASL, NOKEY, 0,            0,
 " < ",NULL,NULL,NULL,NULL,NULL,NULL,
 6,   3, 0, 0,   PBBINASR, NOKEY, 0,            0,
 " > ",NULL,NULL,NULL,NULL,NULL,NULL,
 7,   3, 0, 0,   PBBININV, NOKEY, 0,            0,
 "Inv",NULL,NULL,NULL,NULL,NULL,NULL,
 0,      0,    0, 0,   PBBINOPR1,NOKEY, 0,            0,
 NULL,NULL,NULL,NULL,NULL,NULL,NULL,
 0,      0,    0, 0,   PBBINOPR2,NOKEY, 0,            0,
 NULL,NULL,NULL,NULL,NULL,NULL,NULL,
 0,      0,    0, 0,   PBBINRES, NOKEY, 0,            0,
 NULL,NULL,NULL,NULL,NULL,NULL,NULL,};

static struct keyhandler kh;
static int initialized=0;

OpenBin()
{
   XWindowAttributes info;
   XSizeHints hints;
   int i,j,x,y;

   if(!initialized)
   {
      for(i=0;i<(BINOKEYS-BINREGS);i++)
      {
	 initkd[i].x=BXABS(initkd[i].x);
	 if(initkd[i].y==3)
	    initkd[i].y=BYABS(initkd[i].y)+fonth/2;
	 else
	    initkd[i].y=BYABS(initkd[i].y);
	 initkd[i].w=BINKW;
	 initkd[i].h=BINKH;
      }
      for(j=0;i<BINOKEYS;i++,j++)
      {
	 initkd[i].x=BINSX;
	 initkd[i].y=BINSY+j*BINBH;
	 initkd[i].w=BINBW;
	 initkd[i].h=BINBH;
      }
      initialized=1;
   }
   if(kh.win)
   {
      XRaiseWindow(calcdisp,kh.win);
      /* XIconifyWindow(calcdisp,kh.win,screen); */
      return;
   }
   XGetWindowAttributes(calcdisp,calcwin,&info);
   if((kh.win=XCreateSimpleWindow(calcdisp,RootWindow(calcdisp,screen),
				    info.x+BINXOFF,info.y+info.height+BINYOFF,
				    BINW,BINH,BINB,
				    forecolor.pixel,backcolor.pixel))==0)
      MyExit("Cannot open binary window");
   hints.x=0;
   hints.y=0;
   hints.width=hints.min_width=hints.max_width=BINW;
   hints.height=hints.min_height=hints.max_height=BINH;
   hints.flags=USSize|PMinSize|PMaxSize|PPosition;
   XSetStandardProperties(calcdisp,kh.win,BINTITLE,NULL,None,NULL,0,&hints);
   XDefineCursor(calcdisp,kh.win,XCreateFontCursor(calcdisp,XC_hand2));
   XSelectInput(calcdisp,kh.win,
		ExposureMask|ButtonReleaseMask|ButtonPressMask);
   XMapWindow(calcdisp,kh.win);

   /* set up bit coordinates */

   x=BINBX;
   y=BINBY;
   for(i=0;i<BINBITS;i++)
   {
      kd[i].x=x;
      kd[i].y=y;
      kd[i].w=BINBW;
      kd[i].h=BINBH;
      kd[i].i1=BINBITS-1-i;
      x=x+BINBW;
      if(!((i+1)%BINREGBITS))
      {
	 x=BINBX;
	 y=y+BINBH;
      }
      kd[i].handler= &kh;
   }

   /* set up other key coordinates */

   for(;i<BINKEYS;i++)
   {
      kd[i].x=initkd[i-BINBITS].x;
      kd[i].y=initkd[i-BINBITS].y;
      kd[i].w=initkd[i-BINBITS].w;
      kd[i].h=initkd[i-BINBITS].h;
      kd[i].i1=initkd[i-BINBITS].i1;
      kd[i].txt1=initkd[i-BINBITS].txt1;
      kd[i].handler= &kh;
   }

   /* clear binary registers */

   for(i=0;i<BINREGS;i++)
      binregs[i]=0;
   curbinreg=BINREGRES;
   kh.def=kd;
   kh.noc=BINKEYS;
}

RefreshBin()
{
   int i,x,y;

   if(!kh.win)
      return;
   DrawKeys(&kh,1);
   x=BINBX;
   for(i=0;i<(BINREGBITS+1);i++)
   {
      if(i%8)
      {
	 if(!(i%4))
            XDrawLine(calcdisp,kh.win,gc,x,BINBY,x,BINBY-BINBH/2);
      }
      else
         XDrawLine(calcdisp,kh.win,gc,x,BINBY,x,BINBY-BINBH);
      x=x+BINBW;
   }
   y=BINSY;
   XDrawImageString(calcdisp,kh.win,gc,BTXT1X,BTXT1Y,BTXT1,strlen(BTXT1));
   XDrawImageString(calcdisp,kh.win,gc,BTXT2X,BTXT2Y,BTXT2,strlen(BTXT2));
   XDrawImageString(calcdisp,kh.win,gc,BTXT3X,BTXT3Y,BTXT3,strlen(BTXT3));
   RefreshBinDisplay();
}

RefreshBinDisplay()
{
   int i,reg,x,y;
   unsigned long l;
   char s[10];

   if(!kh.win)
      return;
   DisplayToXreg();
   if(xreg>((double)ULONG_MAX))
      binregs[curbinreg]=ULONG_MAX;
   else
   {
      if(xreg<0.0)
         binregs[curbinreg]=0;
      else
         binregs[curbinreg]=(unsigned long)xreg;
   }
   x=BINBX+BINBOW;
   y=BINBY+BINBOH;
   for(reg=0;reg<BINREGS;reg++)
   {
      l=binregs[BINREGS-reg-1];
      s[0]=0x31+reg;
      s[1]=0;
      XDrawImageString(calcdisp,kh.win,gc,x-fontw-BINBOW-(BINBX-fontw)/2,
		       y+BINBH-BINBOH,s,1);
      for(i=0;i<BINREGBITS;i++)
      {
	 if(l&0x80000000)
            XFillRectangle(calcdisp,kh.win,gc,x,y,BINBW-(2*BINBOW)+1,
			   BINBH-(2*BINBOH)+1);
	 else
            XClearArea(calcdisp,kh.win,x,y,BINBW-(2*BINBOW)+1,
		       BINBH-(2*BINBOH)+1,0);
	 x=x+BINBW;
	 l=l<<1;
      }
      if((BINREGS-reg-1)==curbinreg)
         XFillRectangle(calcdisp,kh.win,gc,BINSX+BINBOW,y,BINBW-(2*BINBOW)+1,
			BINBH-(2*BINBOH)+1);
      else
         XClearArea(calcdisp,kh.win,BINSX+BINBOW,y,BINBW-(2*BINBOW)+1,
		    BINBH-(2*BINBOH)+1,0);
      x=BINBX+BINBOW;
      y=y+BINBH;
   }
}

HandleBin(event)
XButtonEvent *event;
{
   int bkey,breg,key;
   long cy,mask,reg;
   unsigned long l;
   struct keydef *lkdp;

   if((!kh.win)||(event->window!=kh.win))
      return;

   /* window is open */

   switch(event->type)
   {
      case Expose:
         RefreshBin();
	 break;
      case ButtonPress:
      case ButtonRelease:
	 lkdp=HandleKeys(event,&kh,1);
	 if(lkdp)
	 {
	    key=lkdp->i1;
	    if(key>=BINBITS)
	    {
	       /* other keys */

	       l=binregs[curbinreg];
	       breg=curbinreg;
	       switch(key)
	       {
	          case PBBINOFF:
		     XDestroyWindow(calcdisp,kh.win);
		     kh.win=(Window)NULL;
		     break;
		  case PBBINSHL:
		     l=l<<1;
		     l=l&0xfffffffe;
		     break;
		  case PBBINSHR:
		     l=l>>1;
		     l=l&0x7fffffff;
		     break;
		  case PBBINROL:
		  case PBBINASL:
		     cy=l&0x80000000;
		     l=l<<1;
		     l=l&0xfffffffe;
		     if(cy)
		     l=l|1;
		     break;
		  case PBBINROR:
		     cy=l&1;
		     l=l>>1;
		     l=l&0x7fffffff;
		     if(cy)
		     l=l|0x80000000;
		     break;
		  case PBBINASR:
		     cy=l&0x80000000;
		     l=l>>1;
		     l=l&0x7fffffff;
		     if(cy)
		        l=l|0x80000000;
		     break;
		  case PBBININV:
		     l=(~l);
		     break;
		  case PBBINOPR1:
		     breg=curbinreg=BINREGOPR1;
		     l=binregs[curbinreg];
		     break;
		  case PBBINOPR2:
		     breg=curbinreg=BINREGOPR2;
		     l=binregs[curbinreg];
		     break;
		  case PBBINRES:
		     breg=curbinreg=BINREGRES;
		     l=binregs[curbinreg];
		     break;
		  case PBBINAND:
		     breg=curbinreg=BINREGRES;
		     binregs[BINREGRES]=binregs[BINREGOPR1]&
		                        binregs[BINREGOPR2];
		     l=binregs[curbinreg];
		     break;
		  case PBBINOR:
		     breg=curbinreg=BINREGRES;
		     binregs[BINREGRES]=binregs[BINREGOPR1]|
		                        binregs[BINREGOPR2];
		     l=binregs[curbinreg];
		     break;
		  case PBBINXOR:
		     breg=curbinreg=BINREGRES;
		     binregs[BINREGRES]=binregs[BINREGOPR1]^
		                        binregs[BINREGOPR2];
		     l=binregs[curbinreg];
		     break;
		  default:
		     break;
	       }
	    }
	    else
	    {
	       /* bit keys */

	       breg=key/BINREGBITS;
	       bkey=key%BINREGBITS;
	       l=binregs[breg];
	       mask=1<<bkey;
	       if(mask&l)
	          l=l&(~mask);
	       else
	          l=l|mask;
	       binregs[breg]=l;
	    }
	    if(breg==curbinreg)
	    {
	       xreg=(double)l;
	       XregToDisplay();
	    }
	       else
	       RefreshBinDisplay();
	 }
	 break;
      default:
	 break;
   }
}
