/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

uniform sampler2D image;
uniform vec4 exposure;

const vec4 gamma = vec4(0.45, 0.45, 0.45, 1.0);
const vec4 grey = vec4(84.66 / 255.0, 84.66 / 255.0, 84.66 / 255.0, 1.0);

void main(void)
{
   vec4 colour = texture2D(image, gl_TexCoord[0].st);
   colour *= exposure;
   colour = pow(colour, gamma);
   colour *= grey;

   gl_FragColor = colour;
}

