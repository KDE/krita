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

Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA 

*/

#define FRAME_LIMIT  36

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
        Three shell widgets, one for colour output, two for attributes
output
        plus a back_up pixmap for redrawing
*/

static Widget top_level;
static Widget       colour_shell;
static Widget            colour_box;
static Widget                   colour_canvas;
static Pixmap    colour_pm[FRAME_LIMIT];

static GC gc;
static GC tmp_gc;
static long mask;
static XGCValues values;

Display *display;
char pix_file[32];
unsigned int delay;

main(argc, argv)
int argc;
char *argv[];
{
    static Arg args[]={
        {XtNwidth, (XtArgVal) 0},
        {XtNheight, (XtArgVal)0} };

	XEvent event;
        int i, width, height;
	int ret;
	int c;

	extern char *optarg;
       extern int optind;

	width = 300;
	height = 300;

        args[0].value = (XtArgVal)width;

        args[1].value = (XtArgVal)height;

        top_level =  XtInitialize("wet+sticky", "Wet+Sticky", NULL,
                                                 0, &argc, argv);

	delay = 100000; /*default delay in microseconds between frames */

	 while ((c = getopt(argc, argv, "D:")) != -1)
                    switch (c) {
                    case 'D':
			delay = atoi(optarg);
			delay = delay * 1000000;
			break;
		   }


        display = XtDisplay(top_level);

        colour_shell = XtCreateApplicationShell("colour_frame",
                        topLevelShellWidgetClass, NULL, 0);

        colour_box = XtCreateManagedWidget("colour_box", boxWidgetClass,
                                                colour_shell, NULL, 0);


        colour_canvas = XtCreateManagedWidget("", labelWidgetClass,
                                        colour_box, args, XtNumber(args));


        /*XtAddEventHandler(colour_canvas, ExposureMask, False,
	expose_event, 0);*/

        XtRealizeWidget(colour_shell);

	XSynchronize(display, True);

	mask = GCBackground| GCForeground|  GCFunction;

        values.function = GXcopy;
        values.background = 0;
        values.foreground = 1;


        gc = XtGetGC(colour_canvas, mask, &values);

	

	fprintf(stderr,"Read files ...");
	for (i=0; i < FRAME_LIMIT; i++) {
		sprintf(pix_file,"pixmap.%d",i);
		if ((XReadBitmapFile(display, XtWindow(colour_shell), pix_file,
			&width, &height, &colour_pm[i], &ret, &ret)) != 
			BitmapSuccess)
		perror("bad bitmap");
	}
	fprintf(stderr,"done.\nBegin Animation\n");


	for (;;) {
		 for (i=0; i < FRAME_LIMIT; i++) {
			XCopyPlane(display, colour_pm[i], 
				XtWindow(colour_canvas),gc,
				0, 0, 300, 300, 0, 0, 1);
			fprintf(stderr,"pre sleep\n");
			usleep(delay);
		}
		

           } /* End for loop */
}xk

