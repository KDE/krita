/*
        FILE:           x_interface.c
        PURPOSE:        Creation and access to an X windows interface
			to wet+sticky using Athena Widgets
        AUTHOR:         David England
        VERSION:        1.00  (13-May-91)

Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 
Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271


Wet and Sticky is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version. Wet and Sticky is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along
with Wet and Sticky; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

*/

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include <X11/cursorfont.h>
#include <X11/StringDefs.h>

#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <stdio.h>

#include "constants.h"
#include "types.h"
#include "engine.h"
#include "canvas.h"

/* Window Heirarchy - 
	Three shell widgets, one for colour output, two for attributes output
	plus a back_up pixmap for redrawing
*/

static Widget top_level;
static Widget 	    colour_shell;
static Widget            colour_box;
static Widget		        colour_canvas;
static Pixmap		        colour_pm;

static Widget      volume_shell;
static Widget            volume_box;
static Widget                  volume_canvas;
static Pixmap		        volume_pm;

static Widget      dryness_shell;
static Widget            dryness_box;
static Widget                  dryness_canvas;
static Pixmap		        dryness_pm;

static GC gc;
static GC tmp_gc;
static long mask;
static XGCValues values;

static Colormap cmap;
static XColor colours[256];
void stroke();
void stroke_motion();

Display *display;
int screen;
Screen          *screen_ptr;
Window root;

static int count=0;
static int frame_count=0;
char pix_file[64];

static void
expose_event(w, event)
Widget          w;
XEvent          *event;
{
/* Re-display the colour window if an exposure event is received */
int width, height;

	width = height = 300;

        XCopyArea(XtDisplay(colour_canvas), colour_pm, 
		XtWindow(colour_canvas), gc, 0, 0, width, height, 0,0);

}


static void
expose_volume(w, event)
Widget          w;
XEvent          *event;
{
/* Re-display the volume window if an exposure event is received */
int width, height;

	width = 300;
	height = 300;

      XCopyArea(XtDisplay(volume_canvas), volume_pm,
	      XtWindow(volume_canvas), gc, 0, 0, width, height, 0,0);
      

}

static void
expose_dryness(w, event)
Widget          w;
XEvent          *event;
{
/* Re-display the dryness window if an exposure event is received */
int width, height;

        width = height = 300;

      XCopyArea(XtDisplay(dryness_canvas), dryness_pm,
              XtWindow(dryness_canvas), gc, 0, 0, width, height, 0,0);


}

void expose_canvases()
{
int width, height;

width = height = 300;

	XCopyArea(XtDisplay(colour_canvas), colour_pm,
                XtWindow(colour_canvas), gc, 0, 0, width, height, 0,0);

      XCopyArea(XtDisplay(dryness_canvas), dryness_pm,
              XtWindow(dryness_canvas), gc, 0, 0, width, height, 0,0);

	XCopyArea(XtDisplay(dryness_canvas), dryness_pm,
                      XtWindow(dryness_canvas), gc, 0, 0, 300, 300, 0,0);

}

int 
GetHueValue(red, green, blue)
int red;
int green;
int blue;
{
	XColor colour;

	colour.red = red * 257;
	colour.green = green * 257;
	colour.blue = blue * 257;
	colour.flags = DoRed | DoGreen | DoBlue;

	if (XAllocColor(display, cmap, &colour) == 0)
		fprintf(stderr,"colour allocation failed\n");

	return (colour.pixel);	
}


void
DrawPoint(x,y,colour)
int x;
int y;
int colour;
/* Draw a point on the window and the back-up Pixmap */
{
	/* PROBS ? */

	XSetForeground(XtDisplay(top_level), gc, colours[colour].pixel);

	XDrawPoint(XtDisplay(top_level), XtWindow(colour_canvas), gc, x, y);
	XDrawPoint(XtDisplay(top_level), colour_pm, gc, x, y);
}

void
DrawBackgroundPoint(x,y,colour)
int x;
int y;
int colour;
/* Draw a point on the window and the back-up Pixmap */
{
        /* PROBS ? */

        XSetForeground(XtDisplay(top_level), gc, colours[colour].pixel);

        XDrawPoint(XtDisplay(top_level), colour_pm, gc, x, y);
}


int
DrawVolumePoint(x,y,attr)
int x;
int y;
int attr;
/* Draw a point on the window and the back-up Pixmap */
{
	if (XtWindow(volume_canvas) == NULL)
		return (-1);

	XSetForeground(XtDisplay(top_level), gc, colours[attr].pixel);

	XDrawPoint(XtDisplay(top_level), XtWindow(volume_canvas), gc, x, y);
	XDrawPoint(XtDisplay(top_level), volume_pm, gc, x, y);

	return(0);
}

int
DrawBackgroundVolumePoint(x,y,attr)
int x;
int y;
int attr;
/* Draw a point on the window and the back-up Pixmap */
{
        if (XtWindow(volume_canvas) == NULL)
                return (-1);

        XSetForeground(XtDisplay(top_level), gc, colours[attr].pixel);

        XDrawPoint(XtDisplay(top_level), volume_pm, gc, x, y);

        return(0);
}


int
DrawDrynessPoint(x,y,attr)
int x;
int y;
int attr;
/* Draw a point on the window and the back-up Pixmap */
{
        /* later - use the range of the dryness to affect the colour
                value
        */

        if (XtWindow(dryness_canvas) == NULL)
                return (-1);

        XSetForeground(XtDisplay(top_level), gc, colours[attr].pixel);

/*        XDrawPoint(XtDisplay(top_level), XtWindow(dryness_canvas), gc, x, y);
*/
        XDrawPoint(XtDisplay(top_level), dryness_pm, gc, x, y);

        return(0);
}

void
ClearWindow()
{
	XClearWindow(XtDisplay(top_level), XtWindow(colour_canvas));
}

static void
CleanWindow(win)
Drawable win;
/* Fill a window with a solid, white rectangle */
{
XGCValues values;
long mask;

	values.background = colours[0].pixel;
        values.foreground = colours[255].pixel;;
        values.fill_style = FillSolid;
        values.function   = GXclear;


         mask = GCBackground| GCForeground| GCFillStyle | GCFunction;

         tmp_gc = XtGetGC(top_level,  mask, &values);

        XFillRectangle(XtDisplay(top_level), win, tmp_gc, 0,  0, 300, 300);

}

void SetupCmap()
{
int i;

        for (i=0;i<256;i++) {
                colours[i].red = i*257;
                colours[i].flags = DoRed | DoBlue | DoGreen;
	}

     /*   for (i=0;i<=127;i++)
                colours[i].green = i*2*257;

        for (i=128;i>0;i--)
                colours[255-i].green = (i-1)*2*257;*/

       for (i=0;i<64;i++)
                colours[i].green = i*4*257;

        for (i=64;i<128;i++)
                colours[i].green = 65536-i*4*257;
 
         for (i=128;i<192;i++)
                colours[i].green = (i-128)*2*257;
 
         for (i=192;i<255;i++)
                colours[i].green = 65536-(i-128)*2*257;


        for (i=0;i<256;i++)
                colours[i].blue = 65536 - i*257;

	colours[0].red = 65535;
	colours[0].green = 65535;
	colours[0].blue = 65535;
}

void
SetupGreyMap()
{
int i;

	 for (i=0;i<256;i++) {
                colours[i].red = i*257;
                colours[255 - i].flags = DoRed | DoBlue | DoGreen;
	}


         for (i=0;i<256;i++)
                colours[i].green =  i*257;

         for (i=0;i<256;i++)
                colours[i].blue =  i*257;

	colours[255].red = 255*257;
	colours[255].green = 255*257;
	colours[255].blue = 255*257;

}


void CreateWindows(argc, argv, width, height)
int *argc;
char **argv;
int width;
int height;
/* Create colour window heirarchy and add event handlers */
{
	
    static Arg args[]={
        {XtNwidth, (XtArgVal) 0},
        {XtNheight, (XtArgVal)0} };	

	int i;

	args[0].value = (XtArgVal)width;

	args[1].value = (XtArgVal)height;

	top_level =  XtInitialize("wet+sticky", "Wet+Sticky", NULL,
                                                 0, argc, argv);


	display = XtDisplay(top_level);
        screen = DefaultScreen(display);
	screen_ptr = ScreenOfDisplay(display, DefaultScreen(display));

	root = RootWindow(display, screen);

	colour_shell = XtCreateApplicationShell("colour_frame", 
			topLevelShellWidgetClass, NULL, 0);



        colour_box = XtCreateManagedWidget("colour_box", boxWidgetClass,
                                                colour_shell, NULL, 0);


        colour_canvas = XtCreateManagedWidget("", labelWidgetClass,
                                        colour_box, args, XtNumber(args));


	XtAddEventHandler(colour_canvas, ExposureMask, False, expose_event, 0);

	XtAddEventHandler(colour_canvas, ButtonPressMask, False, stroke, 0);

	XtAddEventHandler(colour_canvas, Button1MotionMask, 
		False, stroke_motion, 0);

	XtRealizeWidget(colour_shell);

	cmap = XCreateColormap( display, XtWindow(colour_shell),
                        XDefaultVisualOfScreen(screen_ptr), AllocAll);

        for (i=0; i <= 255; i++)
                colours[i].pixel = i;

        XQueryColors(display, DefaultColormapOfScreen(screen_ptr),colours, 256);

        SetupCmap();

	/*SetupGreyMap();*/

        XStoreColors(display, cmap, colours, 256);

        i=0;
        while( XAllocColorCells( display, DefaultColormapOfScreen(screen_ptr),
                 True, NULL, 0, &colours[i].pixel, 1 ) ) {
                        colours[i].pixel = i;
                        i++;
          }

	XSetWindowColormap(display, XtWindow(colour_shell), cmap);

        XInstallColormap(display, cmap);

	mask = GCBackground| GCForeground|  GCFunction; 

	values.function = GXcopy;
	values.background = colours[0].pixel;
	values.foreground = colours[255].pixel;


	gc = XtGetGC(colour_canvas, mask, &values);

	colour_pm = XCreatePixmap(XtDisplay(top_level), 
			XtWindow(colour_shell), width, height,
                            XDefaultDepth(XtDisplay(top_level), 0));

	CleanWindow(colour_pm);

}


void StartVolumeWindow(width, height)
int width;
int height;
/* Create Volume heirarchy and add event handlers */
{
	static Arg args[]={
	{XtNwidth, (XtArgVal) 0},
	{XtNheight, (XtArgVal)0} };

	args[0].value = (XtArgVal)width;

	args[1].value = (XtArgVal)height;

	volume_shell = XtCreateApplicationShell("volume_frame",
			topLevelShellWidgetClass, NULL, 0);

	volume_box = XtCreateManagedWidget("volume_box", boxWidgetClass,
				volume_shell, NULL, 0);
											
	volume_canvas = XtCreateManagedWidget("", labelWidgetClass,
				volume_box, args, XtNumber(args));

	XtAddEventHandler(volume_canvas, ExposureMask, False, 
				expose_volume, 0);

	XtRealizeWidget(volume_shell);

	XSetWindowColormap(display, XtWindow(volume_shell), cmap);

	volume_pm = XCreatePixmap(XtDisplay(top_level), 
			XtWindow(colour_shell), width, height,
                            XDefaultDepth(XtDisplay(top_level), 0));

	CleanWindow(volume_pm);


}

void StartDrynessWindow(width, height)
int width;
int height;
/* Create dryness heirarchy and add event handlers */
{
	static Arg args[]={
	{XtNwidth, (XtArgVal) 0},
	{XtNheight, (XtArgVal)0} };

	char name[32];

	args[0].value = (XtArgVal)width;

	args[1].value = (XtArgVal)height;

	dryness_shell = XtCreateApplicationShell("dryness_frame",
			topLevelShellWidgetClass, NULL, 0);

	dryness_box = XtCreateManagedWidget("dryness_box", boxWidgetClass,
				dryness_shell, NULL, 0);
	dryness_canvas = XtCreateManagedWidget("Name", labelWidgetClass,
				dryness_box, args, XtNumber(args));

	fprintf(stderr,"Bumps %d\n",(int)dryness_canvas);

	XtAddEventHandler(dryness_canvas, ExposureMask, False, 
				expose_dryness, 0);

	XtRealizeWidget(dryness_shell);

	XSetWindowColormap(display, XtWindow(dryness_shell), cmap);

	dryness_pm = XCreatePixmap(XtDisplay(top_level), 
			XtWindow(colour_shell), width, height,
                            XDefaultDepth(XtDisplay(top_level), 0));

	XStoreName(display, XtWindow(dryness_shell), "bumps");

	CleanWindow(dryness_pm);

}

static void
draw_labels()
{
	XSetForeground(XtDisplay(colour_shell), gc, colours[1].pixel);

	XDrawString(XtDisplay(colour_shell), XtWindow(colour_canvas), gc, 10, 10, 
			"Colour", strlen("Colour"));
	XDrawString(XtDisplay(colour_shell), colour_pm, gc, 10, 10, 
			"Colour", strlen("Colour")); 

	XSetForeground(XtDisplay(colour_shell), gc, colours[128].pixel);

	XDrawString(XtDisplay(colour_shell), XtWindow(volume_canvas), gc, 10, 10, 
			"Volume", strlen("Volume")); 
	XDrawString(XtDisplay(colour_shell), volume_pm, gc, 10, 10, 
			"Volume", strlen("Volume"));

        XSetForeground(XtDisplay(colour_shell), gc, colours[255].pixel);

	XDrawString(XtDisplay(colour_shell), XtWindow(dryness_canvas), gc, 10, 10, 
			"Bump Map", strlen("Bump Map"));
	XDrawString(XtDisplay(colour_shell), dryness_pm, gc, 10, 10, 
			"Bump Map", strlen("Bump Map"));
}

void paint_cell(cell, x, y)
CELL_PTR cell;
int x, y;

{
    int colour, volColour, dryness;
    POINT p;

    p.x = x;
    p.y = y;

    /*  The current display simply maps hue onto the indices of the colour
        table.  This involves some scaling since hues are in the range [0,360)
        with the colour table being [0,256).  */

    colour = (int) (cell->contents.colour.hue * 
			((float) MAX_COLOUR_INDEX / 360.0));

    DrawBackgroundPoint(x,y,colour);   

    /*  volColour is an index into the colour table in the range [0,255].
        It is used to give a false colour image of the canvas's volume.  */

    /*if (x < SCALE_WIDTH) return;    Don't draw over colour scale.  */

    volColour = MIN(cell->volume * 2, 255);
    volColour = MAX(volColour, 0);        
				/* Make unfilled cells have a zero vol.  */

    DrawBackgroundVolumePoint(x,y,volColour);

    /*  Dryness will be in the range [0,255].  */
    dryness = (cell->contents.liquid_content * 255) / 100;

    /*DrawDrynessPoint(x,y,dryness);*/
}



void draw_false_colour_scale()
/*  This routine places a scale along the top of the volume window
    showing the colours being used.   Low is at the left edge. 
    The colour palette has indices 0..255.    */
{
    int x, y;

    /*for (x=0; x < 255; x++) 
       for (y=0; y < SCALE_WIDTH; y++) DrawVolumePoint(x,y,MIN(x, 255));*/
}


void draw_full_canvas()
{
   int x, y;
   CELL_PTR cell;
   POINT p;
 
  if (DEBUG) {
     printf ("Starting to paint full canvas...");
     fflush(stdout);
   }

   for (y=0; y < CANVAS_HEIGHT; y++) 
     for (x=0; x < CANVAS_WIDTH; x++) {
        p.x = x;
        p.y = y;
        cell = get_cell(p);
        paint_cell(cell, x, y);
     }

  expose_canvases();
  
  draw_false_colour_scale();
  if (DEBUG) printf ("done.\n");
}

void
bump_map()
{
	POINT p;
	CELL_PTR cell;
	register int x, y;
	register int colour;


   for (y=0; y < CANVAS_HEIGHT; y++) {
     for (x=0; x < CANVAS_WIDTH; x++) {
        p.x = x;
        p.y = y;
        cell = get_cell(p);
	colour = (int) new_intensity_value(p);

/*	colour = (int) (cell->contents.colour.hue *
                        ((float) MAX_COLOUR_INDEX / 360.0));*/
	DrawDrynessPoint(x,y,colour);
     }
  }

}

draw_cmap_line()

{

int i;

	for (i=0; i< 255; i++) {
	  DrawDrynessPoint(0,i,i);
	}
}

void evolve_paint()
{
   int k;
   POINT p;
   CELL_PTR cell;
   int  new_x, new_y;
   Window tempChild;
   extern Window root;
   extern int count;
   extern int frame_count;

	count++;
	if (count > 5000) {
		fprintf(stderr,".");
		fflush(stderr);
		bump_map();
		expose_canvases();

		/*XTranslateCoordinates(display,XtWindow(dryness_canvas), root,
			0,0, &new_x, &new_y, &tempChild);*/
	/*if (frame_count < 10) 
	sprintf(pix_file,"xwd -name bumps -out pixmap0%d.xwd &" ,frame_count);
	else
	sprintf(pix_file,"xwd -name bumps -out pixmap%d.xwd &" ,frame_count);
	*/
	

		/*system(pix_file);*/
	/*XWriteBitmapFile(display, pix_file,dryness_pm, 300, 300,-1,-1 );*/
		count = 0;
		frame_count++;	
		/*if (frame_count > 250) {
			fprintf(stderr,"Done\n");
			sleep(2); 
			exit(0);
		} */
		/*exit(0)*/;
	} 



   for (k=0; k < STEP_LIMIT; k++) single_step();
   while (TRUE) {
      next_cell_for_repaint(&cell, &p);
      if (cell == NIL) return;
      paint_cell(cell, p.x, p.y);     
   } 
      
}



void StartWindows()
{
/* Start the X windows event loop and paint processing */
XEvent event;


        draw_full_canvas();
	compute_shade_vectors(); /* Set vectors for shading */
	draw_labels();

	for (;;) {
	   if (XtPending()) {
		XtNextEvent(&event);
		XtDispatchEvent(&event);
	   }
	   else {
		/* Evolve paint and re-display*/
		evolve_paint();
	        }
		
	   } /* End for loop */


}

void
stroke(w, client_data, event)
Widget w;
caddr_t client_data;
XEvent *event;
{
/*	brush_stroke(event->xbutton.x, event->xbutton.y);*/


	/*if ((XEvent *)event != (XEvent *)NULL)
	else
		printf("Null event\n"); */

/*	DrawPoint(event->xbutton.x, event->xbutton.y, 128);
        DrawVolumePoint(event->xbutton.x, event->xbutton.y, 128);
        DrawDrynessPoint(event->xbutton.x, event->xbutton.y, 128);*/

	
        XSetForeground(XtDisplay(top_level), gc, colours[128].pixel);

        XFillRectangle(XtDisplay(top_level), XtWindow(colour_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);

        XFillRectangle(XtDisplay(top_level), XtWindow(volume_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);

        XFillRectangle(XtDisplay(top_level), XtWindow(dryness_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);

	brush_stroke(event->xbutton.x, event->xbutton.y);
}


void
stroke_motion(w, client_data, event)
Widget w;
caddr_t client_data;
XEvent *event;
{


        /*if ((XEvent *)event != (XEvent *)NULL)
        else
                printf("Null event\n"); */


        XSetForeground(XtDisplay(top_level), gc, colours[128].pixel);

        XFillRectangle(XtDisplay(top_level), XtWindow(colour_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);
        XFillRectangle(XtDisplay(top_level), XtWindow(volume_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);

        XFillRectangle(XtDisplay(top_level), XtWindow(dryness_canvas), gc,
			 event->xmotion.x, event->xmotion.y ,1, 1);

      brush_stroke(event->xbutton.x, event->xbutton.y);
}

