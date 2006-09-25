/*
Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 

Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271

Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA 


*/

extern void DrawPoint(/* int x, int y; int color*/);
extern int DrawVolumePoint(/* int x, int y; int attr*/);
extern int DrawDrynessPoint(/* int x, int y; int attr*/);
extern void ClearWindow();
extern void CreateWindows(/* int *argc, char *argv[], int width, int height*/);
extern void StartWindows(); /* enter infinite loop */
extern void StartVolumeWindow(/*int width, int height*/); 
                    /* display attribute window */
extern void StartDrynessWindow(/*int width, int height*/); 
                    /* display attribute window */


