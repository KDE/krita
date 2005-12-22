/*
        FILE:           xgl_interface.c
        PURPOSE:        Creation and access to a OpenGL window
			to wet+sticky using OpenGL
        AUTHOR:         David England
        VERSION:        1.00  (21-June-96)

Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 

Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271


Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA 

*/

#include <GL/gl.h>
#include <GL/glu.h>
/*#include <imp.h>*/
/*#include "aux.h"*/

#include <stdio.h>

#include "constants.h"
#include "types.h"
#include "engine.h"
#include "canvas.h"

static int count=0;


void nullProc()
{
}

void SetupCmap()
{
}

float Value(n1, n2, hue)
float n1, n2, hue;
{
	if (hue > 360 )
		hue = hue -360;
	else if (hue < 0 )
		hue = hue +360;
	if (hue < 60  )
		return n1+(n2-n1)*hue/60;
	else if (hue < 180 )
		return n2;
	else if (hue < 240 )
		return n1+(n2-n1)*(240-hue)/60;
	else return n1;
}


RGB_COLOUR hls_to_rgb(h, l, s)
int h;
float l;
float s;
{
RGB_COLOUR rgb_colour;
float m1, m2;

if (l <= 0.5 )
	m2 = l*(1+s);
else 
	m2 = l+s -  l*s;
m1 = 2*l-m2;

rgb_colour.r = Value(m1, m2, h+120);
rgb_colour.g = Value(m1,m2, h);
rgb_colour.b = Value(m1,m2, h-120);

return (rgb_colour);
}

void
DrawPoint(x,y,hls_col)
int x;
int y;
HLS_COLOUR hls_col;
/* Draw a point on the window and the back-up Pixmap */
{
RGB_COLOUR rgb_colour;

	rgb_colour = hls_to_rgb(hls_col.hue,
				hls_col.lightness,
				hls_col.saturation);


	printf("h %.2f l %.2f s %.2f\n", hls_col.hue, hls_col.lightness,
				hls_col.saturation);

	printf( "r %.2f g %.2f b %.2f\n", rgb_colour.r, rgb_colour.g,
                                rgb_colour.b);

	glColor3f(rgb_colour.r, rgb_colour.g, rgb_colour.b);

	glBegin(GL_POINTS);
		glVertex2s(x,y);
		glVertex2s(x+1,y+1);
	glEnd();

}

void
DrawVolumePoint(x,y,attr)
int x;
int y;
int attr;
{
/*set colour, draw point at offset */	
}

void
DrawDrynessPoint(x,y,attr)
int x;
int y;
int attr;
/* Draw a point on the window and the back-up Pixmap */
{

}

void
ClearWindow()
{

}

static void
CleanWindow()
/* Fill a window with a solid, white rectangle */
{
}


void CreateWindows(argc, argv, width, height)
int *argc;
char **argv;
int width;
int height;

{

	auxInitDisplayMode( AUX_RGBA);

	auxInitPosition(50,50, width*3, height);

	auxInitWindow(argv[0]);

	glClearColor(1.0, 1.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT);

}


static void
draw_labels()
{
}

void paint_cell(cell, x, y)
CELL_PTR cell;
int x, y;

{
    DrawPoint(x,y, cell->contents.colour);

    /*  volColour is an index into the colour table in the range [0,255].
        It is used to give a false colour image of the canvas's volume.  */

    /*DrawVolumePoint(x,y,volColour);*/


    /*DrawDrynessPoint(x,y,dryness);*/
}




void draw_full_canvas()
{
   int x, y;
   CELL_PTR cell;
   POINT p;
 
          glClear(GL_COLOR_BUFFER_BIT);

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

   glFlush();
	sleep(10);
  
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


void evolve_paint()
{
   int k;
   POINT p;
   CELL_PTR cell;
   int  new_x, new_y;
   extern int count;

	/*count++;
	if (count > 500) {
		fprintf(stderr,".");
		fflush(stderr);
		bump_map();
	}*/

   for (k=0; k < STEP_LIMIT; k++) single_step();
   while (TRUE) {
      next_cell_for_repaint(&cell, &p);
      if (cell == NIL) return;
      paint_cell(cell, p.x, p.y);     
	   glFlush();
   } 
      

}

void StartVolumeWindow()
{
}

void StartDrynessWindow()
{
}

void StartWindows()
{


        draw_full_canvas();
	compute_shade_vectors(); /* Set vectors for shading */
	draw_labels();

	/*auxIdleFunc(evolve_paint);
	
	auxMainLoop(draw_full_canvas);*/


}

void
stroke()
{
	
}


void
stroke_motion()
{

}

