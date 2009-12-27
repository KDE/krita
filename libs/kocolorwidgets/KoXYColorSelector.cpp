/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <sven.langkamp@gmail.com>

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
#include "KoXYColorSelector.h"
#include "KoColorSpace.h"

#include <QColor>
#include <QPainter>

KoXYColorSelector::KoXYColorSelector( const KoColorSpace* colorSpace, QWidget *parent )
    : KXYSelector( parent )
    , m_colorSpace(colorSpace)
{
    setRange(0, 0, 255, 255);
}

void KoXYColorSelector::setColors( const KoColor& topLeftColor, const KoColor& topRightColor,  const KoColor& bottomLeftColor, const KoColor& bottomRightColor)
{
  m_colors[TOPLEFT] = topLeftColor;
  m_colors[TOPRIGHT] = topRightColor;
  m_colors[BOTTOMLEFT] = bottomLeftColor;
  m_colors[BOTTOMRIGHT] = bottomRightColor;

  update();
}

void KoXYColorSelector::drawContents( QPainter *painter )
{
    KoColor c = m_colors[0]; // quick way to set the colorspace
    QColor color;

    const quint8 *colors[4];
    colors[0] = m_colors[0].data();
    colors[1] = m_colors[1].data();
    colors[2] = m_colors[2].data();
    colors[3] = m_colors[3].data();

    QRect contentsRect_(contentsRect());
    QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32 );
    KoMixColorsOp * mixOp = m_colorSpace->mixColorsOp();

    for (int x = 0; x < contentsRect_.width(); x++)
        for (int y = 0; y < contentsRect_.height(); y++){

            qreal xVal = static_cast<qreal>(x) / (contentsRect_.width() - 1);
            qreal yVal = static_cast<qreal>(y) / (contentsRect_.height() - 1);

            qint16 colorWeights[4];
            colorWeights[0] = static_cast<quint8>((1.0 - yVal) * (1.0 - xVal) * 255 + 0.5);
            colorWeights[1] = static_cast<quint8>((1.0 - yVal) * (xVal) * 255 + 0.5);
            colorWeights[2] = static_cast<quint8>((yVal) * (1.0 - xVal) * 255 + 0.5);
            colorWeights[3] = static_cast<quint8>((yVal) * (xVal) * 255 + 0.5);

            mixOp->mixColors(colors, colorWeights, 4, c.data());

            c.toQColor( &color);

            image.setPixel(x, y, color.rgba());
        }

    painter->drawImage( contentsRect_, image, QRect( 0, 0, image.width(), image.height()) );
}

#include <KoXYColorSelector.moc>
