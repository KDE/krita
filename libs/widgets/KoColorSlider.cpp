/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Sven Langkamp <sven.langkamp@gmail.com>
   SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KoColorSlider.h"
#include "KoColorSpace.h"

#include <KoColor.h>
#include <KoMixColorsOp.h>

#include <QPainter>
#include <QTimer>
#include <QStyleOption>
#include <QPointer>

#define ARROWSIZE 8

struct Q_DECL_HIDDEN KoColorSlider::Private
{
    Private() : upToDate(false), displayRenderer(0) {}
    KoColor minColor;
    KoColor maxColor;
    QPixmap pixmap;
    bool upToDate;
    QPointer<KoColorDisplayRendererInterface> displayRenderer;
};

KoColorSlider::KoColorSlider(QWidget* parent, KoColorDisplayRendererInterface *displayRenderer)
  : KSelector(parent)
  , d(new Private)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()), Qt::UniqueConnection);
}

KoColorSlider::KoColorSlider(Qt::Orientation o, QWidget *parent, KoColorDisplayRendererInterface *displayRenderer)
  : KSelector(o, parent), d(new Private)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()), Qt::UniqueConnection);
}

KoColorSlider::~KoColorSlider()
{
    delete d;
}

void KoColorSlider::setColors(const KoColor& mincolor, const KoColor& maxcolor)
{
    d->minColor = mincolor;
    d->maxColor = maxcolor;
    d->upToDate = false;
    QTimer::singleShot(1, this, SLOT(update()));
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
    QRect contentsRect_(contentsRect());
    painter->fillRect(contentsRect_, QBrush(checker));

    if( !d->upToDate || d->pixmap.isNull() || d->pixmap.width() != contentsRect_.width()
        || d->pixmap.height() != contentsRect_.height() )
    {
        KoColor c = d->minColor; // smart way to fetch colorspace
        QColor color;

        const quint8 *colors[2];
        colors[0] = d->minColor.data();
        colors[1] = d->maxColor.data();

        KoMixColorsOp * mixOp = c.colorSpace()->mixColorsOp();
        Q_ASSERT(mixOp);
        QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32 );

        if( orientation() == Qt::Horizontal ) {
            for (int x = 0; x < contentsRect_.width(); x++) {

                qreal t = static_cast<qreal>(x) / (contentsRect_.width() - 1);

                qint16 colorWeights[2];
                colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
                colorWeights[1] = 255 - colorWeights[0];

                mixOp->mixColors(colors, colorWeights, 2, c.data());

                if (d->displayRenderer) {
                    color = d->displayRenderer->toQColor(c);
                }
                else {
                    color = c.toQColor();
                }

                for (int y = 0; y < contentsRect_.height(); y++)
                image.setPixel(x, y, color.rgba());
            }
        }
        else {
            for (int y = 0; y < contentsRect_.height(); y++) {

                qreal t = static_cast<qreal>(y) / (contentsRect_.height() - 1);

                qint16 colorWeights[2];
                colorWeights[0] = static_cast<quint8>((t) * 255 + 0.5);
                colorWeights[1] = 255 - colorWeights[0];

                mixOp->mixColors(colors, colorWeights, 2, c.data());

                if (d->displayRenderer) {
                    color = d->displayRenderer->toQColor(c);
                }
                else {
                    color = c.toQColor();
                }

                for (int x = 0; x < contentsRect_.width(); x++)
                image.setPixel(x, y, color.rgba());
            }
        }
        d->pixmap = QPixmap::fromImage(image);
        d->upToDate = true;
    }
    painter->drawPixmap( contentsRect_, d->pixmap, QRect( 0, 0, d->pixmap.width(), d->pixmap.height()) );
}

KoColor KoColorSlider::currentColor() const
{
    const quint8 *colors[2];
    colors[0] = d->minColor.data();
    colors[1] = d->maxColor.data();
    KoMixColorsOp * mixOp = d->minColor.colorSpace()->mixColorsOp();
    KoColor c(d->minColor.colorSpace());
    qint16 weights[2];
    weights[1] = (value() - minimum()) / qreal(maximum() - minimum()) * 255;
    weights[0] = 255 - weights[1];
    mixOp->mixColors(colors, weights, 2, c.data());
    return c;
}

void KoColorSlider::drawArrow(QPainter *painter, const QPoint &pos)
{
    painter->setPen(QPen(palette().text().color(), 0));
    painter->setBrush(palette().text());

    QStyleOption o;
    o.initFrom(this);
    o.state &= ~QStyle::State_MouseOver;

    if ( orientation() == Qt::Vertical ) {
        o.rect = QRect( pos.x(), pos.y() - ARROWSIZE / 2,
                        ARROWSIZE, ARROWSIZE );
    } else {
        o.rect = QRect( pos.x() - ARROWSIZE / 2, pos.y(),
                        ARROWSIZE, ARROWSIZE );
    }

    QStyle::PrimitiveElement arrowPE;
    switch (arrowDirection()) {
    case Qt::UpArrow:
        arrowPE = QStyle::PE_IndicatorArrowUp;
        break;
    case Qt::DownArrow:
        arrowPE = QStyle::PE_IndicatorArrowDown;
        break;
    case Qt::RightArrow:
        arrowPE = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::LeftArrow:
    default:
        arrowPE = QStyle::PE_IndicatorArrowLeft;
        break;
    }

    style()->drawPrimitive(arrowPE, &o, painter, this);

}
