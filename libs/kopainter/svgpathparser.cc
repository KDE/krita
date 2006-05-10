/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "svgpathparser.h"
#include <QString>
#include <math.h>
#include <kdebug.h>

// parses the coord into number and forwards to the next token
const char *
SVGPathParser::getCoord( const char *ptr, double &number )
{
	int integer, exponent;
	double decimal, frac;
	int sign, expsign;

	exponent = 0;
	integer = 0;
	frac = 1.0;
	decimal = 0;
	sign = 1;
	expsign = 1;

	// read the sign
	if(*ptr == '+')
		ptr++;
	else if(*ptr == '-')
	{
		ptr++;
		sign = -1;
	}

	// read the integer part
	while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		integer = (integer * 10) + *(ptr++) - '0';
	if(*ptr == '.') // read the decimals
    {
		ptr++;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
			decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

	if(*ptr == 'e' || *ptr == 'E') // read the exponent part
	{
		ptr++;

		// read the sign of the exponent
		if(*ptr == '+')
			ptr++;
		else if(*ptr == '-')
		{
			ptr++;
			expsign = -1;
		}

		exponent = 0;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		{
			exponent *= 10;
			exponent += *ptr - '0';
			ptr++;
		}
    }
	number = integer + decimal;
	number *= sign * pow( (double)10, double( expsign * exponent ) );

	// skip the following space
	if(*ptr == ' ')
		ptr++;

	return ptr;
}

void
SVGPathParser::parseSVG( const QString &s, bool process )
{
	if( !s.isEmpty() )
	{
		QString d = s;
		d = d.replace( ',', ' ' );
		d = d.simplified();

		const char *ptr = d.latin1();
		const char *end = d.latin1() + d.length() + 1;

		double contrlx, contrly, curx, cury, subpathx, subpathy, tox, toy, x1, y1, x2, y2, xc, yc;
		double px1, py1, px2, py2, px3, py3;
		bool relative;
		char command = *(ptr++), lastCommand = ' ';

		subpathx = subpathy = curx = cury = contrlx = contrly = 0.0;
		while( ptr < end )
		{
			if( *ptr == ' ' )
				ptr++;

			relative = false;

			//std::cout << "Command : " << command << std::endl;
			switch( command )
			{
				case 'm':
					relative = true;
				case 'M':
				{
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if( process )
					{
						subpathx = curx = relative ? curx + tox : tox;
						subpathy = cury = relative ? cury + toy : toy;

						svgMoveTo( curx, cury );
					}
					else
						svgMoveTo( tox, toy, !relative );
					break;
				}
				case 'l':
					relative = true;
				case 'L':
				{
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if( process )
					{
						curx = relative ? curx + tox : tox;
						cury = relative ? cury + toy : toy;

						svgLineTo( curx, cury );
					}
					else
						svgLineTo( tox, toy, !relative );
					break;
				}
				case 'h':
				{
					ptr = getCoord( ptr, tox );
					if( process )
					{
						curx = curx + tox;
						svgLineTo( curx, cury );
					}
					else
						svgLineToHorizontal( tox, false );
					break;
				}
				case 'H':
				{
					ptr = getCoord( ptr, tox );
					if( process )
					{
						curx = tox;
						svgLineTo( curx, cury );
					}
					else
						svgLineToHorizontal( tox );
					break;
				}
				case 'v':
				{
					ptr = getCoord( ptr, toy );
					if( process )
					{
						cury = cury + toy;
						svgLineTo( curx, cury );
					}
					else
						svgLineToVertical( toy, false );
					break;
				}
				case 'V':
				{
					ptr = getCoord( ptr, toy );
					if( process )
					{
						cury = toy;
						svgLineTo( curx, cury );
					}
					else
						svgLineToVertical( toy );
					break;
				}
				case 'z':
				case 'Z':
				{
					// reset curx, cury for next path
					if( process )
					{
						curx = subpathx;
						cury = subpathy;
					}
					svgClosePath();
					break;
				}
				case 'c':
					relative = true;
				case 'C':
				{
					ptr = getCoord( ptr, x1 );
					ptr = getCoord( ptr, y1 );
					ptr = getCoord( ptr, x2 );
					ptr = getCoord( ptr, y2 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if( process )
					{
						px1 = relative ? curx + x1 : x1;
						py1 = relative ? cury + y1 : y1;
						px2 = relative ? curx + x2 : x2;
						py2 = relative ? cury + y2 : y2;
						px3 = relative ? curx + tox : tox;
						py3 = relative ? cury + toy : toy;

						svgCurveToCubic( px1, py1, px2, py2, px3, py3 );

						contrlx = relative ? curx + x2 : x2;
						contrly = relative ? cury + y2 : y2;
						curx = relative ? curx + tox : tox;
						cury = relative ? cury + toy : toy;
					}
					else
						svgCurveToCubic( x1, y1, x2, y2, tox, toy, !relative );

					break;
				}
				case 's':
					relative = true;
				case 'S':
				{
					ptr = getCoord( ptr, x2 );
					ptr = getCoord( ptr, y2 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );
					if( !( lastCommand == 'c' || lastCommand == 'C' ||
					    lastCommand == 's' || lastCommand == 'S' ) )
					{
						contrlx = curx;
						contrly = cury;
					}


					if( process )
					{
						px1 = 2 * curx - contrlx;
						py1 = 2 * cury - contrly;
						px2 = relative ? curx + x2 : x2;
						py2 = relative ? cury + y2 : y2;
						px3 = relative ? curx + tox : tox;
						py3 = relative ? cury + toy : toy;

						svgCurveToCubic( px1, py1, px2, py2, px3, py3 );

						contrlx = relative ? curx + x2 : x2;
						contrly = relative ? cury + y2 : y2;
						curx = relative ? curx + tox : tox;
						cury = relative ? cury + toy : toy;
					}
					else
						svgCurveToCubicSmooth( x2, y2, tox, toy, !relative );
					break;
				}
				case 'q':
					relative = true;
				case 'Q':
				{
					ptr = getCoord( ptr, x1 );
					ptr = getCoord( ptr, y1 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if( process )
					{
						px1 = relative ? (curx + 2 * (x1 + curx)) * (1.0 / 3.0) : (curx + 2 * x1) * (1.0 / 3.0);
						py1 = relative ? (cury + 2 * (y1 + cury)) * (1.0 / 3.0) : (cury + 2 * y1) * (1.0 / 3.0);
						px2 = relative ? ((curx + tox) + 2 * (x1 + curx)) * (1.0 / 3.0) : (tox + 2 * x1) * (1.0 / 3.0);
						py2 = relative ? ((cury + toy) + 2 * (y1 + cury)) * (1.0 / 3.0) : (toy + 2 * y1) * (1.0 / 3.0);
						px3 = relative ? curx + tox : tox;
						py3 = relative ? cury + toy : toy;

						svgCurveToCubic( px1, py1, px2, py2, px3, py3 );

						contrlx = relative ? curx + x1 : x1;
						contrly = relative ? cury + y1 : y1;
						curx = relative ? curx + tox : tox;
						cury = relative ? cury + toy : toy;
					}
					else
						svgCurveToQuadratic( x1, y1, tox, toy, !relative );
					break;
				}
				case 't':
					relative = true;
				case 'T':
				{
					ptr = getCoord(ptr, tox);
					ptr = getCoord(ptr, toy);
					if( !( lastCommand == 'q' || lastCommand == 'Q' ||
					       lastCommand == 't' || lastCommand == 'T' ) )
					{
						contrlx = curx;
						contrly = cury;
					}

					if( process )
					{
						xc = 2 * curx - contrlx;
						yc = 2 * cury - contrly;

						px1 = (curx + 2 * xc) * (1.0 / 3.0);
						py1 = (cury + 2 * yc) * (1.0 / 3.0);
						px2 = relative ? ((curx + tox) + 2 * xc) * (1.0 / 3.0) : (tox + 2 * xc) * (1.0 / 3.0);
						py2 = relative ? ((cury + toy) + 2 * yc) * (1.0 / 3.0) : (toy + 2 * yc) * (1.0 / 3.0);
						px3 = relative ? curx + tox : tox;
						py3 = relative ? cury + toy : toy;

						svgCurveToCubic( px1, py1, px2, py2, px3, py3 );

						contrlx = xc;
						contrly = yc;
						curx = relative ? curx + tox : tox;
						cury = relative ? cury + toy : toy;
					}
					else
						svgCurveToQuadraticSmooth( tox, toy, !relative );
					break;
				}
				case 'a':
					relative = true;
				case 'A':
				{
					bool largeArc, sweep;
					double angle, rx, ry;
					ptr = getCoord( ptr, rx );
					ptr = getCoord( ptr, ry );
					ptr = getCoord( ptr, angle );
					ptr = getCoord( ptr, tox );
					largeArc = tox == 1;
					ptr = getCoord( ptr, tox );
					sweep = tox == 1;
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					// Spec: radii are nonnegative numbers
					rx = fabs(rx);
					ry = fabs(ry);

					if( process )
						calculateArc( relative, curx, cury, angle, tox, toy, rx, ry, largeArc, sweep );
					else
						svgArcTo( tox, toy, rx, ry, angle, largeArc, sweep, !relative );
				}
				default:
				{
					// when svg parser is used for a parsing an odf path an unknown command
					// can be encountered, so we stop parsing here
					kDebug() << "SVGPathParser::parseSVG(): unknown command \"" << command << "\"" << endl;
					return;
				}
			}

			lastCommand = command;

			if(*ptr == '+' || *ptr == '-' || (*ptr >= '0' && *ptr <= '9'))
			{
				// there are still coords in this command
				if(command == 'M')
					command = 'L';
				else if(command == 'm')
					command = 'l';
			}
			else
				command = *(ptr++);

			if( lastCommand != 'C' && lastCommand != 'c' &&
				lastCommand != 'S' && lastCommand != 's' &&
				lastCommand != 'Q' && lastCommand != 'q' &&
				lastCommand != 'T' && lastCommand != 't')
			{
				contrlx = curx;
				contrly = cury;
			}
		}
	}
}

// This works by converting the SVG arc to "simple" beziers.
// For each bezier found a svgToCurve call is done.
// Adapted from Niko's code in kdelibs/kdecore/svgicons.
// Maybe this can serve in some shared lib? (Rob)
void
SVGPathParser::calculateArc(bool relative, double &curx, double &cury, double angle, double x, double y, double r1, double r2, bool largeArcFlag, bool sweepFlag)
{
	double sin_th, cos_th;
	double a00, a01, a10, a11;
	double x0, y0, x1, y1, xc, yc;
	double d, sfactor, sfactor_sq;
	double th0, th1, th_arc;
	int i, n_segs;

	sin_th = sin(angle * (M_PI / 180.0));
	cos_th = cos(angle * (M_PI / 180.0));

	double dx;

	if(!relative)
		dx = (curx - x) / 2.0;
	else
		dx = -x / 2.0;

	double dy;
		
	if(!relative)
		dy = (cury - y) / 2.0;
	else
		dy = -y / 2.0;
		
	double _x1 =  cos_th * dx + sin_th * dy;
	double _y1 = -sin_th * dx + cos_th * dy;
	double Pr1 = r1 * r1;
	double Pr2 = r2 * r2;
	double Px = _x1 * _x1;
	double Py = _y1 * _y1;

	// Spec : check if radii are large enough
	double check = Px / Pr1 + Py / Pr2;
	if(check > 1)
	{
		r1 = r1 * sqrt(check);
		r2 = r2 * sqrt(check);
	}

	a00 = cos_th / r1;
	a01 = sin_th / r1;
	a10 = -sin_th / r2;
	a11 = cos_th / r2;

	x0 = a00 * curx + a01 * cury;
	y0 = a10 * curx + a11 * cury;

	if(!relative)
		x1 = a00 * x + a01 * y;
	else
		x1 = a00 * (curx + x) + a01 * (cury + y);
		
	if(!relative)
		y1 = a10 * x + a11 * y;
	else
		y1 = a10 * (curx + x) + a11 * (cury + y);

	/* (x0, y0) is current point in transformed coordinate space.
	   (x1, y1) is new point in transformed coordinate space.

	   The arc fits a unit-radius circle in this space.
     */

	d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);

	sfactor_sq = 1.0 / d - 0.25;

	if(sfactor_sq < 0)
		sfactor_sq = 0;

	sfactor = sqrt(sfactor_sq);

	if(sweepFlag == largeArcFlag)
		sfactor = -sfactor;

	xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
	yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);

	/* (xc, yc) is center of the circle. */
	th0 = atan2(y0 - yc, x0 - xc);
	th1 = atan2(y1 - yc, x1 - xc);

	th_arc = th1 - th0;
	if(th_arc < 0 && sweepFlag)
		th_arc += 2 * M_PI;
	else if(th_arc > 0 && !sweepFlag)
		th_arc -= 2 * M_PI;

	n_segs = (int) (int) ceil(fabs(th_arc / (M_PI * 0.5 + 0.001)));

	for(i = 0; i < n_segs; i++)
	{
		{
			double sin_th, cos_th;
			double a00, a01, a10, a11;
			double x1, y1, x2, y2, x3, y3;
			double t;
			double th_half;

			double _th0 = th0 + i * th_arc / n_segs;
			double _th1 = th0 + (i + 1) * th_arc / n_segs;

			sin_th = sin(angle * (M_PI / 180.0));
			cos_th = cos(angle * (M_PI / 180.0));

			/* inverse transform compared with rsvg_path_arc */
			a00 = cos_th * r1;
			a01 = -sin_th * r2;
			a10 = sin_th * r1;
			a11 = cos_th * r2;

			th_half = 0.5 * (_th1 - _th0);
			t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
			x1 = xc + cos(_th0) - t * sin(_th0);
			y1 = yc + sin(_th0) + t * cos(_th0);
			x3 = xc + cos(_th1);
			y3 = yc + sin(_th1);
			x2 = x3 + t * sin(_th1);
			y2 = y3 - t * cos(_th1);

			svgCurveToCubic( a00 * x1 + a01 * y1, a10 * x1 + a11 * y1, a00 * x2 + a01 * y2, a10 * x2 + a11 * y2, a00 * x3 + a01 * y3, a10 * x3 + a11 * y3 );
		}
	}

	if(!relative)
		curx = x;
	else
		curx += x;

	if(!relative)
		cury = y;
	else
		cury += y;	
}

void
SVGPathParser::svgLineToHorizontal( double, bool )
{
}

void
SVGPathParser::svgLineToVertical( double, bool )
{
}

void
SVGPathParser::svgCurveToCubicSmooth( double, double, double, double, bool )
{
}

void
SVGPathParser::svgCurveToQuadratic( double, double, double, double, bool )
{
}

void
SVGPathParser::svgCurveToQuadraticSmooth( double, double, bool )
{
}

void
SVGPathParser::svgArcTo( double, double, double, double, double, bool, bool, bool )
{
}

