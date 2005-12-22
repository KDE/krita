/*
	FILE:		load_ppm.c
	PURPOSE:	Defines the routines to load a PPM portable pixmap image file.
	AUTHOR:		David England
	VERSION:	1.00  (10-May-91)


Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 

Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271

Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA 

*/

#include "constants.h"
#include "types.h"
#include "canvas.h"

#include <ctype.h>
#include <ppm.h>
#include <ppmcmap.h>

/* Max number of colors allowed in ppm input. */
#define MAXCOLORS 256

extern CELL canvas[CANVAS_WIDTH][CANVAS_HEIGHT];

float
max_rgb(r,g,b)
float r;
float g;
float b;
{
	if ((r > g) && (r > b))
		return (r);

	if ((b > g) && (b > r))
		return (b);

	if ((g > b) && (g > r))
		return (g);

	return(0);
}

float 
min_rgb(r,g,b) 
float r; 
float g; 
float b;
{ 
        if ((r < g) && (b < g)) 
                return (r); 
 
        if ((b < g) && (b < r))
                return (b); 

        if ((g < b) && (g < r)) 
                return (g); 

	return(0);
}


int
GetHue(red, green, blue) /* rgb to hls */
int red;
int green;
int blue;
{
	float min_col, max_col;
	float h,s,l;
	float rc, gc, bc;
	float r, g, b;

	r =  (float)red/255.0;
	b =  (float)green/255.0;
	g =  (float)blue/255.0;

	max_col = (float)max_rgb(r, g, b);
	min_col = (float)min_rgb(r, g, b);

	l = (max_col + min_col)/2.0 ;

	if ( max_col == min_col) {
		s = 0.0;
		h = 0.0;
	} else  {
		if ( l < 0.5) {
			s = (max_col - min_col)/(max_col + min_col);
		} else  s = (max_col - min_col)/(2 - max_col - min_col);

		rc = (max_col -r)/( max_col - min_col);
		gc = (max_col -g)/(max_col - min_col);
		bc = (max_col -b)/(max_col - min_col);

		if (r == max_col)
			h = bc - gc;
		else if (g == max_col )
			h = 2 + rc - bc;
			else if (b == max_col)
				h = 4  + gc -rc;
		
		h = h * 60;

		if ( h < 0.0)
			h = h + 360;
	}

	return ((int)h);


}

GetHuePaint(r,g,b)
int r;
int g;
int b;
{

	/*if ((r == 0) && (g == 0) && (b ==0)) {
		canvas[i][j].contents.liquid_content = 0;
		   canvas[i][j].contents.drying_rate = 0;
		   canvas[i][j].contents.miscibility = 0;

		   canvas[i][j].contents.colour.hue = 0;
		   canvas[i][j].contents.colour.saturation = 1.0;
		   canvas[i][j].contents.colour.lightness = 0.0;
		   canvas[i][j].volume = 80;
		return (0);
	}

	if ((r == 255) && (g == 255) && ( b == 255)) {
		canvas[i][j].contents.liquid_content = 0;
                   canvas[i][j].contents.drying_rate = 0;
                   canvas[i][j].contents.miscibility = 0;

                   canvas[i][j].contents.colour.hue = 128;
                   canvas[i][j].contents.colour.saturation = 1.0;
                   canvas[i][j].contents.colour.lightness = 0.0;
                   canvas[i][j].volume = 80;
		return (128);
	}*/
}

load_ppm_format(filename, width, height)
char *filename;
int *width;
int *height;
{

    FILE* ifp;
    pixel **pixels;
    int rows, cols, i, j;
    pixval maxval;
	int red, green, blue;
	int hue;

    /*ppm_init( &argc, argv );*/



   ifp = pm_openr( filename);

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );

	*(width) = cols;
	*(height) = rows;

	fprintf(stderr,"Loading file %s, %dx%d. Please wait ", filename, rows, 
cols);

	if (rows > CANVAS_HEIGHT)
		rows = CANVAS_HEIGHT;

	if (cols > CANVAS_WIDTH)
		cols = CANVAS_WIDTH;

	for (i=0; i< rows; i++)  {
		for (j=0; j< cols; j++) {
			red = PPM_GETR(pixels[i][j]);
			green = PPM_GETG(pixels[i][j]);
			blue = PPM_GETB(pixels[i][j]);
			
			/*hue = GetHue(red, green, blue);*/

			/* For gray scale only */

			hue = 255 - red;


			/*fprintf(stderr,"hue %d ", hue);*/
			/*GetHuePaint(red, green, blue, i, j);*/

			canvas[i][j].contents.liquid_content = 80;
			   canvas[i][j].contents.drying_rate = 80;
			   canvas[i][j].contents.miscibility = 80;

			   canvas[i][j].contents.colour.hue = hue;
			   canvas[i][j].contents.colour.saturation = 1.0;
			   canvas[i][j].contents.colour.lightness = 0.0;
			   canvas[i][j].volume = (float)hue/2.5;

			if (red == 0) 
			  if  (green == 0) 
			   if  ( blue == 0) {
				canvas[i][j].contents.liquid_content = 0;
				canvas[i][j].contents.drying_rate = 0;
                           canvas[i][j].contents.miscibility = 0;
				canvas[i][j].contents.colour.hue = 0;
				   canvas[i][j].volume = 0;
			}

			if (red == 255) 
			   if (green == 255) 
			      if ( blue == 255) {
                                canvas[i][j].contents.liquid_content = 0;
				canvas[i][j].contents.drying_rate = 0;
                                canvas[i][j].contents.miscibility = 0;
                                canvas[i][j].contents.colour.hue = 360;
				   canvas[i][j].volume = 0;
                        }


		}		
		if (( i %10) == 0){
			fprintf(stderr,".");
			fflush(stderr);
		}
	}

	printf(" done\n");

    pm_close( ifp );

    /*exit(0);*/

}
