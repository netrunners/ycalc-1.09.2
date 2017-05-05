#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* flags for keys */

#define INV1ST            0x0001    /* valid inverse for 1:st */
#define INV2ND            0x0002    /* valid inverse for 2:nd */
#define ACT1ST            0x0004    /* special action during learn for 1:st */
#define ACT2ND            0x0008    /* special action during learn for 2:nd */
#define MRG1ST            0x0010    /* merge next key(s) (number) for 1:st */
#define MRG2ND            0x0020    /* merge next key(s) (number) for 2:nd*/
#define DRAWINV           0x0040    /* draw inverse */
#define LNUMMASK          0xf000    /* number mask */
#define LNUMGET(x)        ((x&LNUMMASK)>>24) /* extract number */
#define LNUM(x)           (x<<24)   /* put number in correct place */

struct keyhandler
{
   Window win;           /* associated window */
   struct keydef *def;   /* pointer to key coordinate array */
   int    noc;           /* number of coordinates in array */
   struct keydef *last;  /* last pressed key */
};

struct keydef
{
   int           x;            /* 0 coordinate upper left corner*/
   int           y;            /* 1 */
   int           w;            /* 2 width */
   int           h;            /* 3 height */
   int           i1;           /* 4 ident & index 1st */
   int           i2;           /* 5 ident & index 2nd */
   unsigned long flags;        /* 6 */
   int           lrn;          /* 7 learn code */
   char          *txt1;        /* text 1st */
   char          *txt2;        /* text 2nd */
   char          *map1;        /* map 1st */
   char          *map2;        /* map 2nd */
   XImage        *img1;        /* image 1st */
   XImage        *img2;        /* image 2nd */
   struct keyhandler *handler; /* associated keyhandler */
};

struct keydef *GetKey(XButtonEvent *, struct keyhandler *);
struct keydef *HandleKeys(XButtonEvent *, struct keyhandler *, int);
void DrawKeys(struct keyhandler *, int);
void DrawOneKey(struct keydef *, int);
void InitMaps(struct keyhandler *);
