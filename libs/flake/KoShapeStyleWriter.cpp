/* This file is part of the KDE project
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#include "KoShapeStyleWriter.h"
#include "KoShapeSavingContext.h"
#include "KoShape.h"
#include "KoShapeBorderModel.h"

#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOasisStyles.h>

KoShapeStyleWriter::KoShapeStyleWriter( KoShapeSavingContext &context )
    : m_context( context )
{
}

QString KoShapeStyleWriter::addFillStyle( KoGenStyle &style, const QBrush &fill )
{
    switch ( fill.style() )
    {
        case Qt::NoBrush:
            style.addProperty( "draw:fill","none" );
            break;
        // the pattern style needs special handling regarding the saving of
        // the pattern texture, os we handle that here
        case Qt::TexturePattern:
            style.addProperty( "draw:fill","bitmap" );
            style.addProperty( "draw:fill-image-name", savePatternStyle( style, fill ) );
            break;
        default:
            KoOasisStyles::saveOasisFillStyle( style, m_context.mainStyles(), fill );
            break;
    }

    if ( m_context.isSet( KoShapeSavingContext::AutoStyleInStyleXml ) ) {
        style.setAutoStyleInStylesDotXml( true );
    }

    return m_context.mainStyles().lookup( style, m_context.isSet( KoShapeSavingContext::PresentationShape ) ? "pr" : "gr" );
}

QString KoShapeStyleWriter::savePatternStyle( KoGenStyle &style, const QBrush &brush )
{
    QPixmap texture = brush.texture();
    QMatrix matrix = brush.matrix();
    QSize size = texture.size();

    style.addProperty( "style:repeat", "repeat" );
    style.addProperty( "draw:fill-image-ref-point-x", QString("%1%").arg( matrix.dx()/size.width() * 100.0 ) );
    style.addProperty( "draw:fill-image-ref-point-y", QString("%1%").arg( matrix.dy()/size.height() * 100.0 ) );

    //style.addAttribute( "draw:fill-image-height", texture.height() );
    //style.addAttribute( "draw:fill-image-width", texture.width() );

    KoGenStyle patternStyle( KoGenStyle::StyleFillImage /*no family name*/ );
    patternStyle.addAttribute( "xlink:show", "embed" );
    patternStyle.addAttribute( "xlink:actuate", "onLoad" );
    patternStyle.addAttribute( "xlink:type", "simple" );
    patternStyle.addAttribute( "xlink:href", m_context.addImageForSaving( texture ) );

    return m_context.mainStyles().lookup( patternStyle, "picture" );
}

void KoShapeStyleWriter::writeOfficeStyles( KoXmlWriter* styleWriter )
{
    KoGenStyles & mainStyles = m_context.mainStyles();
    QList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::StyleGradientLinear );
    QList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();

    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "svg:linearGradient", (*it).name, 0, true, true /*add draw:name*/);

    styles = mainStyles.styles( KoGenStyle::StyleGradientRadial );
    it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "svg:radialGradient", (*it).name, 0, true, true /*add draw:name*/);

    styles = mainStyles.styles( KoGenStyle::StyleStrokeDash );
    it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "draw:stroke-dash", (*it).name, 0, true, true /*add draw:name*/);

    styles = mainStyles.styles( KoGenStyle::StyleFillImage );
    it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "draw:fill-image", (*it).name, 0, true, true /*add draw:name*/);
}
