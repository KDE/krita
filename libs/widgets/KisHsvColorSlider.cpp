/*
 *  SPDX-FileCopyrightText: 2022 Sam Linnfer <littlelightlittlefire@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisHsvColorSlider.h"

#include "KoColorSpace.h"
#include <KoColor.h>

#include <QPainter>
#include <QTimer>
#include <QStyleOption>
#include <QPointer>
#include "KoColorDisplayRendererInterface.h"

namespace {
    const qreal EPSILON = 1e6;
    const int ARROW_SIZE = 8;
}

struct Q_DECL_HIDDEN KisHsvColorSlider::Private
{
    Private()
        : minColor()
        , maxColor()
        , pixmap()
        , upToDate(false)
        , displayRenderer(nullptr)
        , circularHue(false)
    {

    }

    KoColor minColor;
    KoColor maxColor;
    QPixmap pixmap;
    bool upToDate;
    QPointer<KoColorDisplayRendererInterface> displayRenderer;

    // If the min and max color has the same hue, should the widget display the entire gamut of hues.
    bool circularHue;
};

KisHsvColorSlider::KisHsvColorSlider(QWidget *parent, KoColorDisplayRendererInterface *displayRenderer)
    : KSelector(parent)
    , d(new Private)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()), Qt::UniqueConnection);
}

KisHsvColorSlider::KisHsvColorSlider(Qt::Orientation orientation, QWidget *parent, KoColorDisplayRendererInterface *displayRenderer)
    : KSelector(orientation, parent)
    , d(new Private)
{
    setMaximum(255);
    d->displayRenderer = displayRenderer;
    connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), SLOT(update()), Qt::UniqueConnection);
}

KisHsvColorSlider::~KisHsvColorSlider()
{
    delete d;
}

void KisHsvColorSlider::setColors(const KoColor& mincolor, const KoColor& maxcolor)
{
    d->minColor = mincolor;
    d->maxColor = maxcolor;
    d->upToDate = false;
    QTimer::singleShot(1, this, SLOT(update()));
}

void KisHsvColorSlider::setCircularHue(bool value) {
    d->circularHue = value;
}

void KisHsvColorSlider::baseRange(qreal &minH, qreal &minS, qreal &minV, qreal &dH, qreal &dS, qreal &dV) const {
    QColor minColor = d->minColor.toQColor();
    QColor maxColor = d->maxColor.toQColor();

    minColor.getHsvF(&minH, &minS, &minV);

    qreal maxH, maxS, maxV;
    maxColor.getHsvF(&maxH, &maxS, &maxV);

    // getHsvF can return -1 values for hue
    // this happens when the color whites out and the hue informaiton is destroyed
    // e.g. for all x, hsv(x, 0, 1) is always rgb(255, 255, 255) white
    if (minH < 0 && maxH < 0) {
        minH = 0;
        maxH = 0;
    } else if (minH < 0 && maxH > 0) {
        minH = maxH;
    } else if (minH > 0 && maxH < 0) {
        maxH = minH;
    }

    dH = maxH - minH;
    if (d->circularHue) {
        if (dH < EPSILON) {
            dH += 1;
        }
    } else {
        dH = 0;
    }

    dS = maxS - minS;
    dV = maxV - minV;
}

KoColor KisHsvColorSlider::currentColor() const
{
    qreal minH, minS, minV;
    qreal dH, dS, dV;

    baseRange(minH, minS, minV, dH, dS, dV);

    const qreal t = (value() - minimum()) / qreal(maximum() - minimum()) * 255;

    const qreal h = fmod(minH + t * dH, 1);
    const qreal s = minS + t * dS;
    const qreal v = minV + t * dV;

    QColor color;
    color.setHsvF(h, s, v);

    KoColor c(d->minColor.colorSpace());
    c.fromQColor(color);
    return c;
}

void KisHsvColorSlider::drawContents(QPainter *painter)
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

    if (!d->upToDate || d->pixmap.isNull() || d->pixmap.width() != contentsRect_.width()
        || d->pixmap.height() != contentsRect_.height())
    {
        qreal minH, minS, minV;
        qreal dH, dS, dV;

        baseRange(minH, minS, minV, dH, dS, dV);

        QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32);

        if (orientation() == Qt::Horizontal) {
            if (contentsRect_.width() > 0) {
                for (int x = 0; x < contentsRect_.width(); x++) {
                    const qreal t = static_cast<qreal>(x) / (contentsRect_.width());

                    const qreal h = fmod(minH + t * dH, 1);
                    const qreal s = minS + t * dS;
                    const qreal v = minV + t * dV;

                    QColor color;
                    color.setHsvF(h, s, v);

                    for (int y = 0; y < contentsRect_.height(); y++) {
                        image.setPixel(x, y, color.rgba());
                    }
                }
            }
        } else {
            if (contentsRect_.height() > 0) {
                for (int y = 0; y < contentsRect_.height(); y++) {
                    const qreal t = static_cast<qreal>(y) / (contentsRect_.height());

                    const qreal h = fmod(minH + t * dH, 1);
                    const qreal s = minS + t * dS;
                    const qreal v = minV + t * dV;

                    QColor color;
                    color.setHsvF(h, s, v);

                    for (int x = 0; x < contentsRect_.width(); x++) {
                        image.setPixel(x, y, color.rgba());
                    }
                }
            }
        }
        d->pixmap = QPixmap::fromImage(image);
        d->upToDate = true;
    }

    painter->drawPixmap(contentsRect_, d->pixmap, QRect(0, 0, d->pixmap.width(), d->pixmap.height()));
}

void KisHsvColorSlider::drawArrow(QPainter *painter, const QPoint &pos)
{
    painter->setPen(QPen(palette().text().color(), 0));
    painter->setBrush(palette().text());

    QStyleOption o;
    o.initFrom(this);
    o.state &= ~QStyle::State_MouseOver;

    if (orientation() == Qt::Vertical) {
        o.rect = QRect(pos.x(), pos.y() - ARROW_SIZE / 2, ARROW_SIZE, ARROW_SIZE);
    } else {
        o.rect = QRect(pos.x() - ARROW_SIZE / 2, pos.y(), ARROW_SIZE, ARROW_SIZE);
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
        arrowPE = QStyle::PE_IndicatorArrowLeft;
        break;
    default:
        arrowPE = QStyle::PE_IndicatorArrowLeft;
        break;
    }

    style()->drawPrimitive(arrowPE, &o, painter, this);

}
