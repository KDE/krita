/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
   Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>

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
#include "kis_hsv_slider.h"
#include "KoColorSpace.h"

#include <KoColor.h>
#include <KoMixColorsOp.h>
#include <QPainter>
#include <QTimer>

#include "kis_display_color_converter.h"

struct KisHSVSlider::Private
{
    Private() : upToDate(false), displayRenderer(0) {}
    KoColor currentColorF;
    int HSVtype;
    KoColor minColor;
    qreal hue_b;
    QPixmap pixmap;
    bool upToDate;
    KoColorDisplayRendererInterface *displayRenderer;
};

KisHSVSlider::KisHSVSlider(QWidget* parent, KoColorDisplayRendererInterface *displayRenderer)
  : KSelector(parent), d(new Private)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()));
}

KisHSVSlider::KisHSVSlider(Qt::Orientation o, QWidget *parent, KoColorDisplayRendererInterface *displayRenderer, KisCanvas2* canvas)
  : KSelector(o, parent), d(new Private), m_canvas(canvas)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()));
}

KisHSVSlider::~KisHSVSlider()
{
    delete d;
}


void KisHSVSlider::setColors(const KoColor& currentc, const int type, qreal hue_backup, qreal l_R, qreal l_G, qreal l_B)
{
    d->currentColorF = currentc;
    KoColor c = currentc;
    d->HSVtype = type;
    d->hue_b = hue_backup/360.0f;
    R=l_R;
    G=l_G;
    B=l_B;

    d->upToDate = false;

    QTimer::singleShot(1, this, SLOT(update()));
}

void KisHSVSlider::drawContents( QPainter *painter )
{
    QPixmap checker(8, 8);
    QPainter p(&checker);
    p.fillRect(0, 0, 4, 4, Qt::lightGray);
    p.fillRect(4, 0, 4, 4, Qt::darkGray);
    p.fillRect(0, 4, 4, 4, Qt::darkGray);
    p.fillRect(4, 4, 4, 4, Qt::lightGray);
    p.end();
    QRect contentsRect_(contentsRect());
    painter->fillRect(contentsRect_, QBrush(checker));

    if( !d->upToDate || d->pixmap.isNull() || d->pixmap.width() != contentsRect_.width()
        || d->pixmap.height() != contentsRect_.height() )
    {
        KoColor c = d->currentColorF; // smart way to fetch colorspace
        QColor color;
        int type = d->HSVtype;

        QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32 );

        if( orientation() == Qt::Horizontal ) {
            for (int x = 0; x < contentsRect_.width(); x++) {

                qreal t = static_cast<qreal>(x) / (contentsRect_.width() - 1);
                t = 1.0-t;
                
                //function find current color from hsv thingymabobs.
                c = HSXcolor(type, t);
                
                color = d->displayRenderer->toQColor(c);

                for (int y = 0; y < contentsRect_.height(); y++)
                image.setPixel(x, y, color.rgba());
            }
        }
        else {
            for (int y = 0; y < contentsRect_.height(); y++) {

                qreal t = static_cast<qreal>(y) / (contentsRect_.height() - 1);
                
                c = HSXcolor(type, t);
                
                color = d->displayRenderer->toQColor(c);

                for (int x = 0; x < contentsRect_.width(); x++)
                image.setPixel(x, y, color.rgba());
            }   
        }
        d->pixmap = QPixmap::fromImage(image);
        d->upToDate = true;
    }
    painter->drawPixmap( contentsRect_, d->pixmap, QRect( 0, 0, d->pixmap.width(), d->pixmap.height()) );
}

KoColor KisHSVSlider::currentColor() const
{
    KoColor c(d->minColor.colorSpace());
    int type = d->HSVtype;
    qreal t = (value() - minimum()) / qreal(maximum() - minimum()) * 255;
    c = HSXcolor(type, t);
    return c;
}

KoColor KisHSVSlider::HSXcolor(int type, qreal t) const
{
    KoColor coordinate_color(d->currentColorF.colorSpace());
    KoColor c = d->currentColorF;
    qreal hue, sat, val;
    qreal hue_backup = d->hue_b;

    switch(type){
        case 0:
            this->converter()->getHsvF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHsvF( (1.0 - t) , sat, val);
            break;//hsv hue
        case 1:
            this->converter()->getHsvF(c, &hue, &sat, &val);
            if (sat<=0.0 && t<1.0) {
                hue=hue_backup;
                }
            coordinate_color = this->converter()->fromHsvF( hue, (1.0 - t), val);
            break;//hsv sat
        case 2:
            this->converter()->getHsvF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHsvF( hue, sat, (1.0 - t));
            break;//hsv value
        case 3:
            this->converter()->getHslF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHslF( (1.0 - t) , sat, val);
            break;//hsl hue
        case 4:
            this->converter()->getHslF(c, &hue, &sat, &val);
            if (sat<=0.0 && t<1.0) {
                hue=hue_backup;
                }
            coordinate_color = this->converter()->fromHslF( hue, (1.0 - t), val);
            break;//hsl sat
        case 5:
            this->converter()->getHslF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHslF( hue, sat, (1.0 - t));
            break;//hsl value
        case 6:
            this->converter()->getHsiF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHsiF( (1.0 - t) , sat, val);
            break;//hsi hue
        case 7:
            this->converter()->getHsiF(c, &hue, &sat, &val);
            if (sat<=0.0 && t<1.0) {
                hue=hue_backup;
                }
            coordinate_color = this->converter()->fromHsiF( hue, (1.0 - t), val);
            break;//hsi sat
        case 8:
            this->converter()->getHsiF(c, &hue, &sat, &val);
            coordinate_color = this->converter()->fromHsiF( hue, sat, (1.0 - t));
            break;//hsi value
        case 9:
            this->converter()->getHsyF(c, &hue, &sat, &val, R, G, B);
            coordinate_color = this->converter()->fromHsyF( (1.0 - t) , sat, val, R, G, B);
            break;//hsy hue
        case 10:
            this->converter()->getHsyF(c, &hue, &sat, &val, R, G, B);
            if (sat==0 && (1.0-t)>0) {
                hue=hue_backup;
                }
            coordinate_color = this->converter()->fromHsyF( hue, (1.0 - t), val, R, G, B);
            break;//hsy sat
        case 11:
            this->converter()->getHsyF(c, &hue, &sat, &val, R, G, B);
            coordinate_color = this->converter()->fromHsyF( hue, sat, (1.0 - t), R, G, B);
            break;//hsy value
        }
    return coordinate_color;
}

KisDisplayColorConverter* KisHSVSlider::converter() const
{
    return m_canvas ?
        m_canvas->displayColorConverter() :
        KisDisplayColorConverter::dumbConverterInstance();
}
#include <kis_hsv_slider.moc>
