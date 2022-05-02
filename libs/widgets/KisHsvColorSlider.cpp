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

const qreal EPSILON = 1e-6;
const int ARROW_SIZE = 8;
const int FRAME_SIZE = 5;

// Internal color representation used by the slider.
// h, s, v are values between 0 and 1
struct HSVColor {
    explicit HSVColor()
        : h(0), s(0), v(0)
    {
    }

    explicit HSVColor(qreal hh, qreal ss, qreal vv)
        : h(hh), s(ss), v(vv)
    {
    }

    qreal h, s, v;
};

void fromQColor(const QColor minC, const QColor maxC, HSVColor &min, HSVColor &max) {
    minC.getHsvF(&min.h, &min.s, &min.v);
    maxC.getHsvF(&max.h, &max.s, &max.v);

    // getHsvF can return -1 values for hue
    // this happens when the color whites out and the hue information is destroyed
    // e.g. for all x, hsv(x, 0, 1) is always rgb(255, 255, 255) white
    if (min.h < EPSILON && max.h < EPSILON) {
        min.h = 0;
        max.h = 0;
    } else if (min.h < EPSILON && max.h > EPSILON) {
        min.h = max.h;
    } else if (min.h > EPSILON && max.h < EPSILON) {
        max.h = min.h;
    }
}

void baseRange(HSVColor min, HSVColor max, bool circularHue, qreal &dH, qreal &dS, qreal &dV) {
    dH = max.h - min.h;
    if (circularHue) {
        if (dH < EPSILON) {
            dH += 1;
        }
    } else {
        dH = 0;
    }

    dS = max.s - min.s;
    dV = max.v - min.v;
}

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

    HSVColor minColor;
    HSVColor maxColor;
    QPixmap pixmap;
    bool upToDate;
    QPointer<KoColorDisplayRendererInterface> displayRenderer;

    // If the min and max color has the same hue, should the widget display the entire gamut of hues.
    bool circularHue;
};

KisHsvColorSlider::KisHsvColorSlider(QWidget *parent, KoColorDisplayRendererInterface *displayRenderer)
    : KisHsvColorSlider(Qt::Horizontal, parent, displayRenderer)
{
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

void KisHsvColorSlider::setColors(const KoColor min, const KoColor max)
{
    fromQColor(min.toQColor(), max.toQColor(), d->minColor, d->maxColor);
    d->upToDate = false;
    QTimer::singleShot(1, this, SLOT(update()));
}

void KisHsvColorSlider::setColors(const QColor min, const QColor max)
{
    fromQColor(min, max, d->minColor, d->maxColor);
    d->upToDate = false;
    QTimer::singleShot(1, this, SLOT(update()));
}

void KisHsvColorSlider::setCircularHue(bool value) {
    d->circularHue = value;
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
        qreal dH, dS, dV;
        HSVColor min = d->minColor;
        baseRange(d->minColor, d->maxColor, d->circularHue, dH, dS, dV);

        QImage image(contentsRect_.width(), contentsRect_.height(), QImage::Format_ARGB32);

        if (orientation() == Qt::Horizontal) {
            if (contentsRect_.width() > 0) {
                for (int x = 0; x < contentsRect_.width(); x++) {
                    const qreal t = static_cast<qreal>(x) / (contentsRect_.width() - 1);

                    const qreal h = fmod(min.h + t * dH, 1);
                    const qreal s = min.s + t * dS;
                    const qreal v = min.v + t * dV;

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
                    const qreal t = static_cast<qreal>(y) / (contentsRect_.height() - 1);

                    const qreal h = fmod(min.h + t * dH, 1);
                    const qreal s = min.s + t * dS;
                    const qreal v = min.v + t * dV;

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

QPoint KisHsvColorSlider::calcArrowPos(int value) {
    QPoint p;
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int iw = (w < FRAME_SIZE) ? FRAME_SIZE : w;

    double t = static_cast<double>(value - minimum()) / static_cast<double>(maximum() - minimum());
    if (orientation() == Qt::Vertical) {
        p.setY(height() - iw - 1 - (height() - 2 * iw - 1) * t);

        if (arrowDirection() == Qt::RightArrow) {
            p.setX(0);
        } else {
            p.setX(width() - FRAME_SIZE);
        }
    } else {
        p.setX(iw + (width() - 2 * iw - 1) * t);

        if (arrowDirection() == Qt::DownArrow) {
            p.setY(0);
        } else {
            p.setY(height() - FRAME_SIZE);
        }
    }

    return p;
}

void KisHsvColorSlider::drawArrow(QPainter *painter, const QPoint &)
{
    painter->setPen(QPen(palette().text().color(), 0));
    painter->setBrush(palette().text());

    QStyleOption o;
    o.initFrom(this);
    o.state &= ~QStyle::State_MouseOver;

    // Recalculate pos since the value returned by the parent is bugged and doesn't account for negative setMinimum/setMaximum values
    QPoint pos = calcArrowPos(value());

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
