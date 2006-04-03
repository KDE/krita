/* This file is part of the KDE project
   Copyright (C) 2005 Tim Beaulen <tbscope@gmail.org>

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

#include "kogradientmanager.h"

#include "svgnamedcolors.h"

#include <qtextstream.h>
#include <q3cstring.h>

#include <kdebug.h>

KoGradientManager::KoGradientManager()
{
}

KoGradientManager::~KoGradientManager()
{
    // XXX: Should we delete the gradients here?
}

KoGradient* KoGradientManager::loadGradient(const QString& filename)
{
	QString strExt;
	const int result=filename.findRev('.');
	if (result>=0)
	{
		strExt=filename.mid(result).lower();
	}

	QFile f(filename);

	if(f.open(QIODevice::ReadOnly))
	{
		if(strExt == ".ggr")
		{
			return loadKritaGradient(&f);
		}
        else if(strExt==".kgr")
		{
			return loadKarbonGradient(&f);
		}
		else if(strExt==".svg")
		{
			return loadSvgGradient(&f);
		}
	}

	return 0;
}

KoGradient* KoGradientManager::loadKarbonGradient(QFile* file)
{
	QDomDocument doc;
		
	if(!(doc.setContent(file)))
		file->close();
	else
	{
		QDomElement e;
		QDomNode n = doc.documentElement().firstChild();
		
		if(!n.isNull())
		{
			e = n.toElement();
		
			if(!e.isNull())
				if( e.tagName() == "GRADIENT" )
					return parseKarbonGradient(e);
		}
	}

	return 0;
}

KoGradient* KoGradientManager::loadKritaGradient(QFile* file)
{
	KoGradient* grad = new KoGradient();
	
	QByteArray m_data = file->readAll();
	file->close();

	QTextStream fileContent(m_data, QIODevice::ReadOnly);
	fileContent.setEncoding(QTextStream::UnicodeUTF8);

	QString header = fileContent.readLine();

	if (header != "GIMP Gradient") 
	{
		delete grad;
		return 0;
	}

	QString nameDefinition = fileContent.readLine();
	QString numSegmentsText;

	if (nameDefinition.startsWith("Name: ")) 
	{
		QString nameText = nameDefinition.right(nameDefinition.length() - 6);
		numSegmentsText = fileContent.readLine();
	}
	else 
	{
		// Older format without name.

		numSegmentsText = nameDefinition;
	}

	int numSegments;
	bool ok;

	numSegments = numSegmentsText.toInt(&ok);

	if (!ok || numSegments < 1) 
	{
		return 0;
	}

	for (int i = 0; i < numSegments; i++) 
	{
		KoColorStop *stop = new KoColorStop();

		QString segmentText = fileContent.readLine();
		QTextIStream segmentFields(&segmentText);

		double leftOffset;
		double middleOffset;
		double rightOffset;

		segmentFields >> leftOffset >> middleOffset >> rightOffset;

		double leftRed;
		double leftGreen;
		double leftBlue;
		double leftAlpha;

		segmentFields >> leftRed >> leftGreen >> leftBlue >> leftAlpha;

		double rightRed;
		double rightGreen;
		double rightBlue;
		double rightAlpha;

		segmentFields >> rightRed >> rightGreen >> rightBlue >> rightAlpha;

		int interpolationType;
		int colorInterpolationType;

		segmentFields >> interpolationType >> colorInterpolationType;

		middleOffset = (middleOffset - leftOffset) / (rightOffset - leftOffset);

		stop->opacity = leftAlpha;
		stop->midpoint = middleOffset;
		stop->offset = leftOffset;

		stop->color1 = leftRed;
		stop->color2 = leftGreen;
		stop->color3 = leftBlue;
		stop->color4 = 0.0;
		stop->colorType = colorInterpolationType;
		stop->interpolation = interpolationType; 

		grad->colorStops.append(stop);

		if(rightOffset == 1.0)
		{
			KoColorStop *lastStop = new KoColorStop();
			lastStop->opacity = rightAlpha;
			lastStop->midpoint = middleOffset;
			lastStop->offset = rightOffset;
			lastStop->color1 = rightRed;
			lastStop->color2 = rightGreen;
			lastStop->color3 = rightBlue;
			lastStop->color4 = 0.0;
			lastStop->colorType = colorInterpolationType;
			lastStop->interpolation = interpolationType; 
			grad->colorStops.append(lastStop);
		}
	}

	if (!grad->colorStops.isEmpty())
	{
		grad->originX = 0.0;
		grad->originY = 1.0;
		grad->vectorX = 0.0;
		grad->vectorY = 0.0;
		grad->focalpointX = 0.0;
		grad->focalpointY = 0.0;
		grad->gradientType = gradient_type_linear;
		grad->gradientRepeatMethod = repeat_method_none;
		
		return grad;
	}
	else 
	{
		delete grad;
		return 0;
	}
}

KoGradient* KoGradientManager::loadSvgGradient(QFile* file)
{
	QDomDocument doc;
		
	if(!(doc.setContent(file)))
		file->close();
	else
	{
		for( QDomNode n = doc.documentElement().firstChild(); !n.isNull(); n = n.nextSibling() )
		{
			QDomElement e = n.toElement();
			if( e.isNull() ) continue;
		
			if( e.tagName() == "linearGradient" || e.tagName() == "radialGradient" )
				return parseSvgGradient(e);
		}
	}

	return 0;
}

KoGradient* KoGradientManager::parseKarbonGradient(const QDomElement& element)
{
	KoGradient* grad = new KoGradient();

	grad->originX = element.attribute("originX", "0.0").toDouble();
	grad->originY = element.attribute("originY", "0.0").toDouble();
	grad->focalpointX = element.attribute("focalX", "0.0").toDouble();
	grad->focalpointY = element.attribute("focalY", "0.0").toDouble();
	grad->vectorX = element.attribute("vectorX", "0.0").toDouble();
	grad->vectorY = element.attribute("vectorY", "0.0").toDouble();
	grad->gradientType = (KoGradientType)element.attribute("type", 0).toInt();
	grad->gradientRepeatMethod = (KoGradientRepeatMethod)element.attribute("repeatMethod", 0).toInt();

	grad->colorStops.clear();

	// load stops
	QDomNodeList list = element.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement colorstop = list.item( i ).toElement();

			if( colorstop.tagName() == "COLORSTOP" )
			{
				KoColorStop *stop = new KoColorStop();

				QDomElement e = colorstop.firstChild().toElement();

				switch(e.attribute("colorSpace").toUShort())
				{
					case 1:	// cmyk
						stop->color1 = e.attribute( "v1", "0.0" ).toFloat();
						stop->color2 = e.attribute( "v2", "0.0" ).toFloat();
						stop->color3 = e.attribute( "v3", "0.0" ).toFloat();
						stop->color4 = e.attribute( "v4", "0.0" ).toFloat();
						stop->colorType = color_type_cmyk;
						stop->interpolation = interpolation_linear;
						break;
					case 2: // hsv
						stop->color1 = e.attribute( "v1", "0.0" ).toFloat();
						stop->color2 = e.attribute( "v2", "0.0" ).toFloat();
						stop->color3 = e.attribute( "v3", "0.0" ).toFloat();
						stop->color4 = 0.0;
						stop->colorType = color_type_hsv_cw;
						stop->interpolation = interpolation_linear;
						break;
					case 3: // gray
						stop->color1 = e.attribute( "v1", "0.0" ).toFloat();
						stop->color2 = 0.0;
						stop->color3 = 0.0;
						stop->color4 = 0.0;
						stop->colorType = color_type_gray;
						stop->interpolation = interpolation_linear;
						break;
					default: // rgb
						stop->color1 = e.attribute( "v1", "0.0" ).toFloat();
						stop->color2 = e.attribute( "v2", "0.0" ).toFloat();
						stop->color3 = e.attribute( "v3", "0.0" ).toFloat();
						stop->color4 = 0.0;
						stop->colorType = color_type_rgb;
						stop->interpolation = interpolation_linear;
				}

				stop->opacity = e.attribute("opacity", "1.0").toFloat();

				stop->offset = colorstop.attribute("ramppoint", "0.0").toFloat();
				stop->midpoint = colorstop.attribute("midpoint", "0.5").toFloat();

				grad->colorStops.append(stop);
			}
		}
	}

	return grad;
}

KoGradient* KoGradientManager::parseSvgGradient(const QDomElement& element)
{
	KoGradient* grad = new KoGradient;

	grad->colorStops.clear();
	grad->gradientRepeatMethod = repeat_method_none;

	/*QString href = e.attribute( "xlink:href" ).mid( 1 );
	if( !href.isEmpty() )
	{
	}*/

	bool bbox = element.attribute( "gradientUnits" ) != "userSpaceOnUse";

	if( element.tagName() == "linearGradient" )
	{
		if( bbox )
		{
			QString s;

			s = element.attribute( "x1", "0%" );
			double xOrigin;
			if( s.endsWith( "%" ) )
				xOrigin = s.remove( '%' ).toDouble();
			else
				xOrigin = s.toDouble() * 100.0;

			s = element.attribute( "y1", "0%" );
			double yOrigin;
			if( s.endsWith( "%" ) )
				yOrigin = s.remove( '%' ).toDouble();
			else
				yOrigin = s.toDouble() * 100.0;

			s = element.attribute( "x2", "100%" );
			double xVector;
			if( s.endsWith( "%" ) )
				xVector = s.remove( '%' ).toDouble();
			else
				xVector = s.toDouble() * 100.0;

			s = element.attribute( "y2", "0%" );
			double yVector;
			if( s.endsWith( "%" ) )
				yVector = s.remove( '%' ).toDouble();
			else
				yVector = s.toDouble() * 100.0;

			grad->originX = xOrigin;
			grad->originY = yOrigin;
			grad->vectorX = xVector;
			grad->vectorY = yVector;
		}
		else
		{
			grad->originX = element.attribute( "x1" ).toDouble();
			grad->originY = element.attribute( "y1" ).toDouble();
			grad->vectorX = element.attribute( "x2" ).toDouble();
			grad->vectorY = element.attribute( "y2" ).toDouble();
		}
		grad->gradientType = gradient_type_linear;
	}
	else
	{
		if( bbox )
		{
			QString s;

			s = element.attribute( "cx", "50%" );
			double xOrigin;
			if( s.endsWith( "%" ) )
				xOrigin = s.remove( '%' ).toDouble();
			else
				xOrigin = s.toDouble() * 100.0;

			s = element.attribute( "cy", "50%" );
			double yOrigin;
			if( s.endsWith( "%" ) )
				yOrigin = s.remove( '%' ).toDouble();
			else
				yOrigin = s.toDouble() * 100.0;

			s = element.attribute( "cx", "50%" );
			double xVector;
			if( s.endsWith( "%" ) )
				xVector = s.remove( '%' ).toDouble();
			else
				xVector = s.toDouble() * 100.0;

			s = element.attribute( "r", "50%" );
			if( s.endsWith( "%" ) )
				xVector += s.remove( '%' ).toDouble();
			else
				xVector += s.toDouble() * 100.0;

			s = element.attribute( "cy", "50%" );
			double yVector;
			if( s.endsWith( "%" ) )
				yVector = s.remove( '%' ).toDouble();
			else
				yVector = s.toDouble() * 100.0;

			s = element.attribute( "fx", "50%" );
			double xFocal;
			if( s.endsWith( "%" ) )
				xFocal = s.remove( '%' ).toDouble();
			else
				xFocal = s.toDouble() * 100.0;

			s = element.attribute( "fy", "50%" );
			double yFocal;
			if( s.endsWith( "%" ) )
				yFocal = s.remove( '%' ).toDouble();
			else
				yFocal = s.toDouble() * 100.0;

			grad->originX = xOrigin;
			grad->originY = yOrigin;
			grad->vectorX = xVector;
			grad->vectorY = yVector;
			grad->focalpointX = xFocal;
			grad->focalpointY = yFocal;
		}
		else
		{
			grad->originX = element.attribute( "cx" ).toDouble();
			grad->originY = element.attribute( "cy" ).toDouble();
			grad->vectorX = element.attribute( "cx" ).toDouble() + element.attribute( "r" ).toDouble();
			grad->vectorY = element.attribute( "cy" ).toDouble();
			grad->focalpointX = element.attribute( "fx" ).toDouble();
			grad->focalpointY = element.attribute( "fy" ).toDouble();
		}
		grad->gradientType = gradient_type_radial;
	}
	// handle spread method
	QString spreadMethod = element.attribute( "spreadMethod" );
	if( !spreadMethod.isEmpty() )
	{
		if( spreadMethod == "reflect" )
			grad->gradientRepeatMethod = repeat_method_reflect;
		else if( spreadMethod == "repeat" )
			grad->gradientRepeatMethod = repeat_method_repeat;
	}

	for( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		QDomElement colorstop = n.toElement();
		if( colorstop.tagName() == "stop" )
		{
			KoColorStop *stop = new KoColorStop();
			QColor c;
			float off;
			QString temp = colorstop.attribute( "offset" );
			if( temp.contains( '%' ) )
			{
				temp = temp.left( temp.length() - 1 );
				off = temp.toFloat() / 100.0;
			}
			else
				off = temp.toFloat();

			if( !colorstop.attribute( "stop-color" ).isEmpty() )
				parseSvgColor( c, colorstop.attribute( "stop-color" ) );
			else
			{
				// try style attr
				QString style = colorstop.attribute( "style" ).simplified();
				QStringList substyles = QStringList::split( ';', style );
			    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
				{
					QStringList substyle = QStringList::split( ':', (*it) );
					QString command	= substyle[0].trimmed();
					QString params	= substyle[1].trimmed();
					if( command == "stop-color" )
						parseSvgColor( c, params );
					if( command == "stop-opacity" )
						stop->opacity = params.toDouble();
				}

			}
			if( !colorstop.attribute( "stop-opacity" ).isEmpty() )
				stop->opacity = colorstop.attribute( "stop-opacity" ).toDouble();

			stop->offset = off;
			stop->midpoint = 0.5;
			stop->color1 = c.red() / 255.0;
			stop->color2 = c.green() / 255.0;
			stop->color3 = c.blue() / 255.0;
			stop->color4 = 0.0;
			stop->colorType = color_type_rgb;
			stop->interpolation = interpolation_linear;
			grad->colorStops.append(stop);
		}
	}

	return grad;
}

void KoGradientManager::parseSvgColor(QColor &color, const QString &s)
{
	if( s.startsWith( "rgb(" ) )
	{
		QString parse = s.trimmed();
		QStringList colors = QStringList::split( ',', parse );
		QString r = colors[0].right( ( colors[0].length() - 4 ) );
		QString g = colors[1];
		QString b = colors[2].left( ( colors[2].length() - 1 ) );

		if( r.contains( "%" ) )
		{
			r = r.left( r.length() - 1 );
			r = QString::number( int( ( double( 255 * r.toDouble() ) / 100.0 ) ) );
		}

		if( g.contains( "%" ) )
		{
			g = g.left( g.length() - 1 );
			g = QString::number( int( ( double( 255 * g.toDouble() ) / 100.0 ) ) );
		}

		if( b.contains( "%" ) )
		{
			b = b.left( b.length() - 1 );
			b = QString::number( int( ( double( 255 * b.toDouble() ) / 100.0 ) ) );
		}

		color = QColor( r.toInt(), g.toInt(), b.toInt() );
	}
	else
	{
		QString rgbColor = s.trimmed();
		QColor c;
		if( rgbColor.startsWith( "#" ) )
			c.setNamedColor( rgbColor );
		else
		{
			int r, g, b;
			svgNamedColorToRGB( rgbColor, r, g, b );
			c = QColor( r, g, b );
		}
		color = c;
	}
}
	
