/* 
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
*/ 
/* 
 * HISTORY
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: Dog.c /main/7 1997/05/02 10:11:08 dbl $"
#endif
#endif

/*****************************************************************************
*
*  Dog.c - Dog widget source file
*  
******************************************************************************/

#include <stdio.h>
#include <X11/StringDefs.h>
#include <Xm/XmP.h>
#include <Xm/DrawP.h>
#include "DogP.h"

#ifdef USING_UIL
#include <Mrm/MrmAppl.h>
#endif /* USING_UIL */

#include "up.bm"
#include "down.bm"
#include "bark.bm"

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define WagTime(w) XmField(w,offsets,Dog,wag_time, int)
#define BarkTime(w) XmField(w,offsets,Dog,bark_time, int)
#define BarkCallback(w) XmField(w,offsets,Dog,bark_callback,XtCallbackList)
#define UpPixmap(w) XmField(w,offsets,Dog,up_pixmap,Pixmap)
#define DownPixmap(w) XmField(w,offsets,Dog,down_pixmap,Pixmap)
#define BarkPixmap(w) XmField(w,offsets,Dog,bark_pixmap,Pixmap)
#define CurrPixmap(w) XmField(w,offsets,Dog,curr_pixmap,Pixmap)
#define CurrPx(w) XmField(w,offsets,Dog,curr_px,int)
#define Wagging(w) XmField(w,offsets,Dog,wagging,Boolean)
#define Barking(w) XmField(w,offsets,Dog,barking,Boolean)
#define DrawGC(w) XmField(w,offsets,Dog,draw_GC,GC)
#define PixmapX(w) XmField(w,offsets,Dog,pixmap_x,Position)
#define PixmapY(w) XmField(w,offsets,Dog,pixmap_y,Position)
#define DrawX(w) XmField(w,offsets,Dog,draw_x,Position)
#define DrawY(w) XmField(w,offsets,Dog,draw_y,Position)
#define DrawWidth(w) XmField(w,offsets,Dog,draw_width,Dimension)
#define DrawHeight(w) XmField(w,offsets,Dog,draw_height,Dimension)
#define CurrWidth(w) XmField(w,offsets,Dog,curr_width,Dimension)
#define CurrHeight(w) XmField(w,offsets,Dog,curr_height,Dimension)
#define HighlightThickness(w) \
		XmField(w,offsets,XmPrimitive,highlight_thickness,Dimension)
#define ShadowThickness(w) \
		XmField(w,offsets,XmPrimitive,shadow_thickness,Dimension)
#define Foreground(w)	XmField(w,offsets,XmPrimitive,foreground,Pixel)
#define Highlighted(w) XmField(w,offsets,XmPrimitive,highlighted,Boolean)
#define TopShadowGC(w) XmField(w,offsets,XmPrimitive,top_shadow_GC,GC)
#define BottomShadowGC(w) XmField(w,offsets,XmPrimitive,bottom_shadow_GC,GC)
#define BackgroundPixel(w) XmField(w,offsets,Core,background_pixel,Pixel)
#define Width(w) XmField(w,offsets,Core,width,Dimension)
#define Height(w) XmField(w,offsets,Core,height,Dimension)
#define WagId(w) XmField(w,offsets,Dog,wagId,XtIntervalId)
#define BarkId(w) XmField(w,offsets,Dog,barkId,XtIntervalId)

#define SetPixmap(w, px, pixmap, width, height) \
    CurrPx(w) = px; CurrPixmap(w) = pixmap; \
    CurrWidth(w) = width; CurrHeight(w) = height

#define MakePixmap(b,wd,ht) \
    XCreatePixmapFromBitmapData(XtDisplay(w),RootWindowOfScreen(XtScreen(w)), \
    (char *)(b), (wd), (ht), Foreground(w), BackgroundPixel(w), \
    DefaultDepthOfScreen(XtScreen(w)))

static void ClassInitialize();
static void Initialize();
static void Redisplay();
static void Resize();
static void Destroy();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();

static void bark_dog();
static void end_bark();
static void start_wag();
static void stop_wag();
static void do_wag();
static void create_GC();
static void create_pixmaps();
static void destroy_pixmaps();

static char defaultTranslations[] =
   "<Btn1Down>:         Bark()\n\
    ~Shift<Btn2Down>:   StartWag()\n\
    Shift<Btn2Down>:    StopWag()\n\
    <Key>Return:        Bark()\n\
    Ctrl <Key>Return:   Bark()\n\
    <Key>osfActivate:   Bark()\n\
    <Key>space:         Bark()\n\
    Ctrl <Key>space:    Bark()\n\
    <Key>osfSelect:     Bark()\n\
    <Key>W:             StartWag()\n\
    <Key>S:             StopWag()\n\
    <Key>osfHelp:       PrimitiveHelp()";

static XtActionsRec actionsList[] = {
    { "Bark", (XtActionProc) bark_dog},
    { "StartWag", (XtActionProc) start_wag},
    { "StopWag", (XtActionProc) stop_wag}
};

static XmPartResource resources[] = {
    {DogNwagTime, DogCWagTime, XtRInt, sizeof(int),
	XmPartOffset(Dog,wag_time), XmRImmediate, (caddr_t)100},
    {DogNbarkTime, DogCBarkTime, XtRInt, sizeof(int),
	XmPartOffset(Dog,bark_time), XmRImmediate, (caddr_t)1000},
    {DogNbarkCallback, XtCCallback, XtRCallback, sizeof(caddr_t),
	XmPartOffset(Dog,bark_callback), XtRCallback, NULL}
};

DogClassRec dogClassRec = {
    {                                   /* core_class fields    */
    (WidgetClass) &xmPrimitiveClassRec, /* superclass           */
    "Dog",                              /* class_name           */
    sizeof(DogPart),                    /* widget_size          */
    ClassInitialize,                    /* class_initialize     */
    NULL,                               /* class_part_initialize*/
    False,                              /* class_inited         */
    Initialize,                         /* initialize           */
    NULL,                               /* initialize_notify    */
    XtInheritRealize,                   /* realize              */
    actionsList,                        /* actions              */
    XtNumber(actionsList),              /* num_actions          */
    (XtResourceList)resources,          /* resources            */
    XtNumber(resources),                /* num_resources        */
    NULLQUARK,                          /* xrm_class            */
    True,                               /* compress_motion      */
    True,                               /* compress_exposure    */
    True,                               /* compress_enterleave  */
    False,                              /* visible_interest     */
    Destroy,                            /* destroy              */
    Resize,                             /* resize               */
    Redisplay,                          /* expose               */
    SetValues,                          /* set_values           */
    NULL,                               /* set_values_hook      */
    XtInheritSetValuesAlmost,           /* set_values_almost    */
    NULL,                               /* get_values_hook      */
    NULL,                               /* accept_focus         */
    XtVersionDontCheck,                 /* version              */
    NULL,                               /* callback_private     */
    defaultTranslations,                /* tm_table             */
    QueryGeometry,                      /* query_geometry       */
    NULL,                               /* disp accelerator     */
    NULL                                /* extension            */
    },
    {				        /* primitive_class record */
    XmInheritWidgetProc,               	/* border_highlight     */
    XmInheritWidgetProc,               	/* border_unhighlight   */
    XtInheritTranslations,       	/* translations         */
    bark_dog,                           /* arm_and_activate     */
    NULL,   	    			/* syn resources        */
    0,					/* num syn_resources    */
    NULL,                             	/* extension            */
    },
    {					/* dog_class record     */
    NULL,                             	/* extension            */
    }
};

externaldef(dogwidgetclass) WidgetClass dogWidgetClass =
						(WidgetClass) &dogClassRec;

static XmOffsetPtr offsets; /* Part Offset table for XmResolvePartOffsets */

/**********************************************************************
 *
 * DogCreate - Convenience routine, used by Uil/Mrm.
 *
 *********************************************************************/

Widget DogCreate(parent, name, arglist, nargs)
    Widget parent;
    char *name;
    Arg *arglist;
    int nargs;
{
    return(XtCreateWidget (name, dogWidgetClass, parent, arglist, nargs));
}

#ifdef USING_UIL
/**********************************************************************
 *
 * DogMrmInitialize - register Dog widget class with Mrm
 *
 *********************************************************************/

int DogMrmInitialize()
{
    return(MrmRegisterClass (MrmwcUnknown, "Dog" , "DogCreate",	DogCreate,
				(WidgetClass)&dogClassRec));
}
#endif /* USING_UIL */

/**********************************************************************
 *
 * _DogDrawPixmap - draw the current pixmap
 *
 *********************************************************************/

void _DogDrawPixmap(dw)
    DogWidget dw;
{
    Widget w = (Widget) dw;

    if (XtIsRealized(w)) {
	XCopyArea(XtDisplay(w),CurrPixmap(w),
		XtWindow(w),DrawGC(w),
		PixmapX(w), PixmapY(w),
		DrawWidth(w), DrawHeight(w),
		DrawX(w), DrawY(w));
    }
}

/**********************************************************************
 *
 * _DogPosition(w) - position the current pixmap
 *
 *********************************************************************/

void _DogPosition(w)
    DogWidget w;
{
    Dimension margin = ShadowThickness(w) + HighlightThickness(w);

    if (CurrWidth(w) < MAX(Width(w) - 2 * margin,0)) {
	PixmapX(w) = 0;
	DrawX(w) = Width(w)/2 - CurrWidth(w)/2;
	DrawWidth(w) = CurrWidth(w);
    }
    else {
	PixmapX(w) = (CurrWidth(w) - (Width(w) - 2 * margin))/2;
	DrawX(w) = margin;
	DrawWidth(w) = Width(w) - 2 * margin;
    }

    if (CurrHeight(w) < MAX(Height(w) - 2 * margin,0)) {
	PixmapY(w)= 0;
	DrawY(w) = Height(w)/2 - CurrHeight(w)/2;
	DrawHeight(w) = CurrHeight(w);
    }
    else {
	PixmapY(w) = (CurrHeight(w) - (Height(w) - 2 * margin))/2;
	DrawY(w) = margin;
	DrawHeight(w) = Height(w) - 2 * margin;
    }
}

/**********************************************************************
 *
 * Class methods
 *
 *********************************************************************/

static void ClassInitialize()
{
    XmResolvePartOffsets(dogWidgetClass, &offsets);
}


static void create_GC(w)
    DogWidget w;
{
    XGCValues       values;
    XtGCMask        valueMask;

    valueMask = GCForeground | GCBackground | GCGraphicsExposures;
    values.foreground = Foreground(w);
    values.background = BackgroundPixel(w);
    values.graphics_exposures = False;
    DrawGC(w) = XtGetGC((Widget)w,valueMask,&values);
}

static void create_pixmaps(w)
    DogWidget w;
{
    UpPixmap(w) = MakePixmap(up_bits, up_width, up_height);
    DownPixmap(w) = MakePixmap(down_bits, down_width, down_height);
    BarkPixmap(w) = MakePixmap(bark_bits, bark_width, bark_height);
}

static void Initialize(request, new)
    DogWidget request;
    DogWidget new;
{
    if (Width(request) == 0)
	Width(new) = MAX(MAX(up_width, down_width),bark_width) +
		2*(ShadowThickness(new)+HighlightThickness(new));
    if (Height(request) == 0)
	Height(new) = MAX(MAX(up_height, down_height),bark_height) +
		2*(ShadowThickness(new)+HighlightThickness(new));
    create_GC(new);
    create_pixmaps(new);
    SetPixmap(new, DownPx, DownPixmap(new), down_width, down_height);
    Wagging(new) = False;
    Barking(new) = False;
    WagId(new) = 0;
    BarkId(new) = 0;

    Resize(new);
}

static void destroy_pixmaps(w)
    DogWidget w;
{
    XFreePixmap (XtDisplay(w), UpPixmap(w));
    XFreePixmap (XtDisplay(w), DownPixmap(w));
    XFreePixmap (XtDisplay(w), BarkPixmap(w));
}


static void Destroy(w)
    DogWidget w;
{
    XtReleaseGC ((Widget)w, DrawGC(w));
    destroy_pixmaps(w);

    if (WagId(w) != 0)
                XtRemoveTimeOut( WagId(w));
    if (BarkId(w) != 0)
                XtRemoveTimeOut( BarkId(w));

    XtRemoveAllCallbacks ((Widget)w, DogNbarkCallback);
}

static void Resize(w)
    DogWidget w;
{
    _DogPosition(w);
}

static Boolean DifferentBackground(w, p)
     Widget w;
     Widget p;
{
  if (XmIsPrimitive(w) && XmIsManager(p)) 
    {
      Pixel w_bg, p_bg;
      Pixmap w_px, p_px;
      
      XtVaGetValues(w, XmNbackground, &w_bg, XmNbackgroundPixmap, &w_px, NULL);
      XtVaGetValues(p, XmNbackground, &p_bg, XmNbackgroundPixmap, &p_px, NULL);
      
      return ((w_bg == p_bg) && (w_px == p_px));
    }
  
  return (False);
}

static void Redisplay(w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
    if (XtIsRealized(w)) {
	XmeDrawShadows(XtDisplay (w), XtWindow (w),
		       TopShadowGC(w), BottomShadowGC(w), 
		       HighlightThickness(w), HighlightThickness(w),
		       Width(w) - 2 * HighlightThickness(w),
		       Height(w) - 2 * HighlightThickness(w),
		       ShadowThickness(w),
		       XmSHADOW_OUT);

	if (Highlighted(w))
	   (*((DogWidgetClass)XtClass(w)) ->
	    primitive_class.border_highlight)((Widget)w);
	else if (DifferentBackground ((Widget)w, XtParent (w)))
	   (*((DogWidgetClass)XtClass(w)) ->
	      primitive_class.border_unhighlight)((Widget)w);

	_DogDrawPixmap((DogWidget)w);
    }
}

static Boolean SetValues(current, request, new)
    DogWidget current;
    DogWidget request;
    DogWidget new;

{
    Boolean redraw = False;

    if (ShadowThickness(new) != ShadowThickness(current) ||
	HighlightThickness(new) != HighlightThickness(current)) {
	_DogPosition(new);
	redraw = True;
    }
    if (Foreground(new) != Foreground(current) ||
        BackgroundPixel(new) != BackgroundPixel(current)) {
	XtReleaseGC ((Widget)current, DrawGC(current));
        create_GC(new);
	destroy_pixmaps(new);
	create_pixmaps(new);
	switch (CurrPx(new)) {
	    case(UpPx) : 
		SetPixmap(new,UpPx,UpPixmap(new),up_width,up_height);
		break;
	    case(DownPx) : 
		SetPixmap(new,DownPx,DownPixmap(new),down_width,down_height);
		break;
	    case(BarkPx) : 
		SetPixmap(new,BarkPx,BarkPixmap(new),bark_width,bark_height);
		break;
	}
	redraw = True;
    }
    return (redraw);
}

static XtGeometryResult QueryGeometry (w, intended, reply)
    DogWidget w;
    XtWidgetGeometry *intended;
    XtWidgetGeometry *reply;
{
    reply->request_mode = 0;

    if (intended->request_mode & (~(CWWidth | CWHeight)) != 0)
        return (XtGeometryNo);

    reply->request_mode = (CWWidth | CWHeight);
    reply->width = MAX(MAX(down_width,up_width),bark_width) +
			2*(ShadowThickness(w)+HighlightThickness(w));
    reply->height = MAX(MAX(down_height,up_height),bark_height) +
			2*(ShadowThickness(w)+HighlightThickness(w));

    if (reply->width != intended->width ||
	reply->height != intended->height ||
	intended->request_mode != reply->request_mode)
	return (XtGeometryAlmost);
    else {
	reply->request_mode = 0;
	return (XtGeometryYes);
    }
}
/**********************************************************************
 *
 * Widget actions
 *
 *********************************************************************/

static void bark_dog(w, event)
    DogWidget w;
    XEvent *event;
{
    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    XtCallCallbacks ((Widget)w, DogNbarkCallback, NULL);
    if (Barking(w) == True) return;
    Barking(w) = True;
    SetPixmap(w,BarkPx,BarkPixmap(w),bark_width,bark_height);
    _DogPosition(w);
    _DogDrawPixmap(w);
    BarkId(w) = XtAppAddTimeOut (XtWidgetToApplicationContext((Widget)w),
                BarkTime(w), end_bark, w);
}

static void end_bark(w, t)
    DogWidget w;
    XtIntervalId *t;
{
    SetPixmap(w,DownPx,DownPixmap(w),down_width,down_height);
    _DogPosition(w);
    _DogDrawPixmap(w);
    Barking(w) = False;
    BarkId(w) = 0;
    if (Wagging(w) == True)
        WagId(w) = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)w),
                        WagTime(w), do_wag, w);

}

static void start_wag(w, event)
    DogWidget w;
    XEvent *event;
{
    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    if (Wagging(w) == True) return;
    Wagging(w) = True;
    if (Barking(w) == True) return;
    WagId(w) = XtAppAddTimeOut (XtWidgetToApplicationContext((Widget)w),
                WagTime(w), do_wag, w);
}

static void stop_wag(w, event)
    DogWidget w;
    XEvent *event;
{
    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    Wagging(w) = False;
    if (WagId(w) != 0)
                XtRemoveTimeOut( WagId(w));
    WagId(w) = 0;

}

static void do_wag(w, t)
    XtPointer w;
    XtIntervalId *t;
{
    DogWidget dw = (DogWidget)w;

    if (Barking(dw) == True) return;
    if (Wagging(dw) == False) return;
    switch(CurrPx(dw)) {
	case(UpPx):
	    SetPixmap(dw,DownPx,DownPixmap(dw),down_width,down_height);
	    break;
	case(DownPx):
	    SetPixmap(dw,UpPx,UpPixmap(dw),up_width,up_height);
	    break;
    }
    _DogPosition(dw);
    _DogDrawPixmap(dw);
    XtAppAddTimeOut (XtWidgetToApplicationContext((Widget)dw),
		     WagTime(dw), do_wag, dw);
}

