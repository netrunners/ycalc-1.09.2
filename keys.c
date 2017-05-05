/*
   ycalc, an X Window Calculator, Based on Texas Instruments TI-59
   Common key handling

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
    901220 : moved from xcalc.c
             InitKeys()
    910110 : PBGTO
    910128 : PBLIST
    910320 : Requester() - make window follow calculator window
    910430 : HandleCalc() - invert key image during key press
             DrawOneKey()
             Handle button released in other key than pressed
    941105 : porting to linux
             DrawOneKey() - bug: accessing txt when it was NULL
    941106 :   Requester() - bug: accessing b1txt when it was NULL
    950506 : move calc stuff to calc.c
    960418 : DrawKeys() DrawOneKey() from calc.c
    960421 : DrawKeys() DrawOneKey() - new input parameter: map
             InitKeys() from calc.c rename to InitMaps()
    960619 : requester code to req.c
    970920 : InitMaps() - initialize each map for each keydef
    970921 : HandleKeys() - code from calc.c
    980226 : font sizes are variables
             DrawOneKey() - if no text image do a XClearArea before
	                    XDrawRectangle()
*/

#include "Keys.h"
#include "Main.h"

#define NOINVMSG      "No inverse defined"

void
DrawKeys(struct keyhandler *kh, int no)
{
   int i;

   for(i=0;i<kh->noc;i++)
      DrawOneKey(&(kh->def[i]),no);
}

/*

NAME

SYNOPSIS
 void DrawOneKey(coord,pos,txt,win,map)
 int *coord,pos;
 char *txt[];
 Window win;
 char *map[];

DESCRIPTION

ENTRY PARAMETERS
 int *coord  : Pointer to array of int with key data.
 int pos     : Key position in coord.
 char *txt[] : Pointer to array of keytexts.
 Window win  : Window the key resides in.
 char *map[] : Pointer to array of key ximages. May be NULL.

RETURN VALUES
 Unknown.

EXAMPLES

WARNINGS

AUTHOR
  Ulf Nordquist

SEE ALSO

LOCATION

*/

void
DrawOneKey(struct keydef *kdp, int no)
{
   GC lgc;
   char *ltp;
   XImage *lxip;

   if((kdp->flags)&DRAWINV)
      lgc=invgc;
   else
      lgc=gc;
   if(no==1)
   {
      ltp=kdp->txt1;
      lxip=kdp->img1;
   }
   else
   {
      ltp=kdp->txt2;
      lxip=kdp->img2;
   }
   if(ltp)
   {
      XDrawImageString(calcdisp,kdp->handler->win,lgc,kdp->x+fontw/3+1,
		       kdp->y+fonth/5+fontb-1,ltp,strlen(ltp));
      XDrawRectangle(calcdisp,kdp->handler->win,lgc,kdp->x,kdp->y,kdp->w,
		     kdp->h);
   }
   else
   {
      if(lxip)
      {
	 XPutImage(calcdisp,kdp->handler->win,lgc,lxip,0,0,
		   kdp->x,kdp->y,
		   ksx+1,ksy+1);
	 if(!ltp)
	    XDrawRectangle(calcdisp,kdp->handler->win,lgc,kdp->x,kdp->y,kdp->w,
			   kdp->h);
      }
      else
      {
	 XClearArea(calcdisp,kdp->handler->win,kdp->x,kdp->y,kdp->w,kdp->h,0);
	 XDrawRectangle(calcdisp,kdp->handler->win,lgc,kdp->x,kdp->y,kdp->w,
			kdp->h);
      }
   }
}

struct keydef *
GetKey(XButtonEvent *event, struct keyhandler *kh)
{
   int i;
   short x,y;
   struct keydef *kdp;

   x=event->x;
   y=event->y;
   for(i=0;i<kh->noc;i++)
   {
      kdp= &(kh->def[i]);
      if((x>=(kdp->x))&&(y>=(kdp->y)))
      {
	 if((x<=(kdp->x+(kdp->w)))&&(y<=(kdp->y+(kdp->h))))
	    return kdp;
      }
   }
   return NULL;
}

/*

NAME
 InitMaps - Initializes an array of images for keys.

SYNOPSIS
 InitMaps(map,n)
 char *map[];
 int n;

DESCRIPTION
 Initializes an array of images for keys. When called map[] should contain
 pointers to bitmaps for each key (pointers may be NULL). These pointers
 will be converted to pointers to ximages for use by DrawOneKey().

ENTRY PARAMETERS
 char *map[] : Pointers to bitmaps.
 int n       : Number of pointers.

RETURN VALUES
 Unknown.

EXAMPLES

WARNINGS

AUTHOR
  Ulf Nordquist

SEE ALSO

LOCATION

*/

void
InitMaps(struct keyhandler *kh)
{
   int i;
   XImage *image;
   struct keydef *lkdp;

   for(i=0;i<kh->noc;i++)
   {
      if(kh->def[i].map1)
      {
	 image=XCreateImage(calcdisp,DefaultVisual(calcdisp,screen),1,
			    XYBitmap,0,kh->def[i].map1,ksx+1,ksy+1,
			    BitmapPad(calcdisp),0);
	 image->bitmap_unit=8;
	 image->bitmap_pad=8;
	 image->bitmap_bit_order=LSBFirst;
	 image->bytes_per_line=(image->width/8)+(image->width%8?1:0);
	 kh->def[i].img1=image;
      }
      if(kh->def[i].map2)
      {
	 image=XCreateImage(calcdisp,DefaultVisual(calcdisp,screen),1,
			    XYBitmap,0,kh->def[i].map2,ksx+1,ksy+1,
			    BitmapPad(calcdisp),0);
	 image->bitmap_unit=8;
	 image->bitmap_pad=8;
	 image->bitmap_bit_order=LSBFirst;
	 image->bytes_per_line=(image->width/8)+(image->width%8?1:0);
	 kh->def[i].img2=image;
      }
   }
}

struct keydef*
HandleKeys(XButtonEvent *event, struct keyhandler *kh, int no)
{
   int index;
   struct keydef *lkdp;

   if((!kh->win)||(event->window!=kh->win))
      return;

   /* window is open and event is for this window */
   lkdp=NULL;
   switch(event->type)
   {
	   
      case ButtonPress:
	 if(lkdp=GetKey(event,kh))
	 {
	    lkdp->flags=lkdp->flags|DRAWINV;
	    DrawOneKey(lkdp,no);
	    kh->last=lkdp;
	    lkdp=NULL;
	 }
	 break;
      case ButtonRelease:
	 if(kh->last)
	 {
	    kh->last->flags=kh->last->flags&(~DRAWINV);
	    DrawOneKey(kh->last,no);
	    lkdp=GetKey(event,kh);
	    if(lkdp)
	    {
	       if(kh->last!=lkdp)
		  lkdp=NULL;
	    }
	    kh->last=NULL;
	 }
	 break;
   }
   return lkdp;
}
