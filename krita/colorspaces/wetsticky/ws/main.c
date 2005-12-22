/*
	FILE:		main.c
	PURPOSE:	The top-level program for Wet&Sticky
	AUTHOR:		Kevin Waite and David England
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
#include "win_interface.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

double HEIGHT_SCALE;

void main(argc, argv) 
int argc;
char *argv[];

/*
   The Wet&Sticky program can be executed with optional parameters.
   Those parameters used by the X graphics system are stripped out.
   If no parameters are given then the program runs as a purely
   interactive system.  This requires input handling which is not
   yet implemented.  If the argument is the string '-blob' then the
   program uses a square blob as the starting image.  Otherwise the
   program assumes the argument is a filename containing a previously
   stored canvas.  This is then loaded into the canvas as a starting
   point.
*/

{
   char *filename;
   extern void exit();
   int width;
   int height;
   extern int optind, opterr;
   extern char *optarg;
   int c,i;
   int blob_flag;


   fprintf(stdout, "Wet&Sticky Version %s\n", VERSION);
   fprintf(stdout, "Implemented by K.Waite and D.England, 1991\n");
   fprintf(stdout, "Based on ideas by Tunde Cockshott\n\n");


   initialise_canvas();
   if (DEBUG) fprintf (stdout, "Finished initialising the canvas\n");

   CreateWindows (&argc, argv, CANVAS_WIDTH , CANVAS_HEIGHT);

   filename = argv[1];

   blob_flag = 1;
   HEIGHT_SCALE = 20.0;

  opterr = 0;
    fprintf(stderr, "HEIGHT %g\n", HEIGHT_SCALE);

  while ((c = getopt(argc, argv, "f:s:")) != EOF)
                    switch (c) {
                case 'f':
                        filename = optarg;
			load_file(filename, &width, &height);
			blob_flag = 0;
                        break;
                case 's':
                        fprintf(stderr, "HEIGHT string %s \n",optarg);
                        HEIGHT_SCALE = atof(optarg);
                        break;
		case '?':
                        break;
                }

   if (blob_flag)
	blob (DEFAULT_BLOB_SIZE);

    fprintf(stderr, "HEIGHT %g\n", HEIGHT_SCALE);

   StartVolumeWindow (CANVAS_WIDTH, CANVAS_HEIGHT);
   StartDrynessWindow (CANVAS_WIDTH, CANVAS_HEIGHT);

   if (DEBUG) fprintf (stdout, "Finished preparing X\n");

   if (DEBUG) fprintf (stdout, "Passing control to window manager\n");
   StartWindows();
   exit(0);

}
