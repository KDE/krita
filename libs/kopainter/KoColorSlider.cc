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

#include "KoColorSlider.h"

KoColorSlider::KoColorSlider(QWidget* parent)
  : KSelector(parent)
{
    setMaximum(255);
}

KoColorSlider::KoColorSlider(Qt::Orientation o, QWidget *parent)
  : KSelector(o, parent)
{
    setMaximum(255);
}

KoColorSlider::~KoColorSlider()
{
}

void KoColorSlider::setColors(const KoColor& mincolor, const KoColor& maxcolor)
{
  m_minColor = mincolor;
  m_maxColor = maxcolor;

  update();
}

void KoColorSlider::drawContents( QPainter *painter )
{
  QPixmap checker(8, 8);
  QPainter p(&checker);
  p.fillRect(0, 0, 4, 4, Qt::lightGray);
  p.fillRect(4, 0, 4, 4, Qt::darkGray);
  p.fillRect(0, 4, 4, 4, Qt::darkGray);
  p.fillRect(4, 4, 4, 4, Qt::lightGray);
  p.end();
  painter->fillRect(contentsRect(), QBrush(checker));

  KoColor c = m_minColor; // smart way to fetch colorspace
  QColor color;
  quint8 opacity;

  const quint8 *colors[2];
  colors[0] = m_minColor.data();
  colors[1] = m_maxColor.data();

  QImage image(contentsRect().width(), contentsRect().height(), QImage::Format_ARGB32 );

  if( orientation() == Qt::Horizontal ) {
    for (int x = 0; x < contentsRect().width(); x++) {

        double t = static_cast<double>(x) / (contentsRect().width() - 1);

        quint8 colorWeights[2];
        colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
        colorWeights[1] = 255 - colorWeights[0];

        c.colorSpace()->mixColors(colors, colorWeights, 2, c.data());

        c.toQColor(&color, &opacity);
        color.setAlpha(opacity);

        for (int y = 0; y < contentsRect().height(); y++)
          image.setPixel(x, y, color.rgba());
    }
  }
  else {
    for (int y = 0; y < contentsRect().height(); y++) {

        double t = static_cast<double>(y) / (contentsRect().height() - 1);

        quint8 colorWeights[2];
        colorWeights[0] = static_cast<quint8>((t) * 255 + 0.5);
        colorWeights[1] = 255 - colorWeights[0];

        c.colorSpace()->mixColors(colors, colorWeights, 2, c.data());

        c.toQColor(&color, &opacity);
        color.setAlpha(opacity);

        for (int x = 0; x < contentsRect().width(); x++)
          image.setPixel(x, y, color.rgba());
    }
  }
  painter->drawImage( contentsRect(), image, QRect( 0, 0, image.width(), image.height()) );
}

#include "KoColorSlider.moc"
