/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <longamp@reallygood.de>

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

#include <QColor>
#include <QPainter>

#include "KoXYColorSelector.h"

KoXYColorSelector::KoXYColorSelector( KoColorSpace* colorSpace, QWidget *parent ): KXYSelector( parent ), m_colorSpace(colorSpace)
{
  //XXX: Only for testing remove later
  xColor1 = KoColor( QColor( 255, 0, 0), m_colorSpace);
  xColor2 = KoColor( QColor( 255, 0, 255), m_colorSpace);
  yColor1 = KoColor( QColor( 0, 255, 0), m_colorSpace);
  yColor2 = KoColor( QColor( 0, 0, 0), m_colorSpace);
}

void KoXYColorSelector::setXColors( const KoColor& leftColor, const KoColor& rightColor)
{
  xColor1 = leftColor;
  xColor2 = rightColor;
  xColor1.convertTo(m_colorSpace);
  xColor2.convertTo(m_colorSpace);

  update();
}

void KoXYColorSelector::setYColors( const KoColor& topColor, const KoColor& bottomColor)
{
  yColor1 = topColor;
  yColor2 = bottomColor;
  yColor1.convertTo(m_colorSpace);
  yColor2.convertTo(m_colorSpace);

  update();
}

void KoXYColorSelector::drawContents( QPainter *painter )
{
    KoColor c = xColor1;
    QColor color;
    quint8 opacity;

    const quint8 *colors[4];
    colors[0] = xColor1.data();
    colors[1] = xColor2.data();
    colors[2] = yColor1.data();
    colors[3] = yColor2.data();

    QImage image(contentsRect().width(), contentsRect().height(), QImage::Format_ARGB32 );

    for (int x = 0; x < contentsRect().width(); x++)
        for (int y = 0; y < contentsRect().height(); y++){

            double xVal = static_cast<double>(x) / (contentsRect().width() - 1);
            double yVal = static_cast<double>(y) / (contentsRect().height() - 1);

            quint8 colorWeights[4];
            colorWeights[0] = static_cast<quint8>((1.0 - xVal) * 255 + 0.5);
            colorWeights[1] = 255 - colorWeights[0];
            colorWeights[2] = static_cast<quint8>((1.0 - yVal) * 255 + 0.5);
            colorWeights[3] = 255 - colorWeights[2];

            m_colorSpace->mixColors(colors, colorWeights, 4, c.data());

            c.toQColor( &color, &opacity );
            color.setAlpha(opacity);

            image.setPixel(x, y, color.rgba());
        }

    painter->drawImage( contentsRect(), image, QRect( 0, 0, image.width(), image.height()) );
}

#include "KoXYColorSelector.moc"
