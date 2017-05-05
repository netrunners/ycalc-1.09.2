/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Requester window handling

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
    960619 : requester code from keys.c
    970922 : malloc reqsbdy
    970927 : handle multiline requester
    980226 : font sizes are variables
    980317 : use forecolor & backcolor
*/

#include <malloc.h>

#include "Keys.h"
#include "Main.h"

/* for the two buttons */

static struct keydef kd[2];
static struct keyhandler kh;

/* body text */

static int reqbtx,reqbty;    /* body top left */
static char *reqbdy;         /* body text */
static int reqlcnt;          /* lines of text in body */
static char **reqlines;      /* array of line pointers */

/* return button pressed, 1 or 2 or 0 if none */

int
HandleReq(XButtonEvent *event)
{
   struct keydef *lkdp;
   int i,key;

   if((!kh.win)||(event->window!=kh.win))
      return 0;

   /* requester is open */

   key=0;
   switch(event->type)
   {
      case Expose:
	 DrawKeys(&kh,1);
	 for(i=0;i<reqlcnt;i++)
	    XDrawImageString(calcdisp,kh.win,gc,reqbtx,reqbty+i*fonth,
			     *(reqlines+i),strlen(*(reqlines+i)));
	 break;
      case ButtonPress:
      case ButtonRelease:
	 lkdp=HandleKeys(event,&kh,1);
	 if(lkdp)
	 {
	    key=lkdp->i1;
	    XDestroyWindow(calcdisp,kh.win);
	    kh.win=(Window)NULL;
	    free(reqlines);
	    reqlines=NULL;
	    free(reqbdy);
	    reqbdy=NULL;
	 }
   }
   return key;
}

/* default texts */

#define REQTITLE   "Calc Req"
#define REQDEFBDY  "Waiting !" /* defult body text */
#define REQDEFBUT  "Continue"  /* default button zero text */

/* display a requester box
   with 1 or 2 buttons
   x and y is offset relative win
*/

void
Requester(char *bdytxt, char *b0txt, char *b1txt, int x, int y, Window win)
{
   int ccnt,lwid,rx,ry,w;
   XWindowAttributes info;
   XSizeHints hints;
   Window child;
   char *cp;

   if(kh.win)
      return;

   /* setup keydefs */

   if(b0txt==NULL)
      kd[0].txt1=REQDEFBUT;
   else
      kd[0].txt1=b0txt;
   kd[0].w=(strlen(kd[0].txt1)+1)*fontw;
   kd[0].h=fonth+fonth/2;
   kd[0].i1=1;
   kd[0].i2=1;
   kd[0].flags=INV1ST+INV2ND;
   kd[0].handler= &kh;
   kh.noc=1;
   if(b1txt!=NULL)
   {
      kd[1].w=(strlen(b1txt)+1)*fontw;
      kd[1].h=kd[0].h;
      kd[1].i1=2;
      kd[1].i2=2;
      kd[1].flags=INV1ST+INV2ND;
      kd[1].handler= &kh;
      kd[1].txt1=b1txt;
      kh.noc=2;
   }

   /* setup body text */

   if(!bdytxt)
   {
      if(reqbdy=malloc(strlen(REQDEFBDY)))
	 strcpy(reqbdy,REQDEFBDY);
      else
	 return;
   }
   else
   {
      if(reqbdy=malloc(strlen(bdytxt)))
	 strcpy(reqbdy,bdytxt);
      else
	 return;
   }

   /* determine number of lines and number of chars in widest line */

   cp=reqbdy;
   ccnt=reqlcnt=lwid=0;
   reqlines=NULL;
   while(*cp)
   {
      if(!ccnt)
      {
	 char *tmp;

	 /* start of line */

	 if(!(tmp=realloc(reqlines,sizeof(char*)*(reqlcnt+1))))
	 {
	    free(reqlines);
	    return;
	 }
	 reqlines=(char**)tmp;

	 /* remember where it started */

	 *(reqlines+reqlcnt)=cp;
	 reqlcnt++;
      }
      if((*cp)=='\n')
      {
	 /* end of line, check length */

	 if(ccnt>lwid)
	    lwid=ccnt;
	 ccnt=0;
	 *cp='\0';
      }
      else
	 ccnt++;
      cp++;
   }

   /* handle no '\n' */

   if(!lwid)
   {
      lwid=ccnt;
      reqlcnt=1;
   }

   /*   reqlcnt++;*/

   /* calculate window size */

   hints.width=(lwid+2)*fontw;
   hints.height=(reqlcnt+3)*fonth;

   /* calulate text position */

   reqbtx=fontw;
   reqbty=fonth;

   /* calulate button position */

   kd[0].x=reqbtx;
   kd[0].y=hints.height-2*fonth;
   w=kd[0].y+kd[0].w;
   if(kh.noc>1)
   {
      kd[1].x=hints.width-kd[1].w-fontw;
      kd[1].y=kd[0].y;
      w=w+kd[0].w+2*fontw;
   }

   /* make sure that buttons fit in the window */

   if(w>hints.width)
   {
      hints.width=w;
      kd[1].x=hints.width-kd[1].w-fontw;
   }

   /* open window */

   XGetWindowAttributes(calcdisp,win,&info);
   XTranslateCoordinates(calcdisp,calcwin,RootWindow(calcdisp,screen),
			 info.x,info.y,&rx,&ry,&child);
   hints.x=rx;
   hints.y=ry;
   if((kh.win=XCreateSimpleWindow(calcdisp,RootWindow(calcdisp,screen),
				  hints.x,hints.y,hints.width,hints.height,
				  CALCB,
				  forecolor.pixel,backcolor.pixel))==0)
      MyExit("Cannot open requester window");
   hints.flags=USPosition|USSize;
   XSetStandardProperties(calcdisp,kh.win,REQTITLE,NULL,None,NULL,0,&hints);
   XDefineCursor(calcdisp,kh.win,XCreateFontCursor(calcdisp,XC_coffee_mug));
   XSelectInput(calcdisp,kh.win,
		ExposureMask|ButtonReleaseMask|ButtonPressMask);
   XMapWindow(calcdisp,kh.win);
   kh.def=kd;
}
