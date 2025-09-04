/*
 *  SPDX-FileCopyrightText: 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Scratchpad.h"
#include <KoResource.h>
#include <kis_config.h>
#include <kis_gradient_painter.h>
#include "kis_scratch_pad.h"
#include "Resource.h"
#include "View.h"
#include "Canvas.h"
#include <KoCanvasBase.h>
#include <kis_canvas2.h>
#include "kis_scratch_pad.h"

#include <QColor>
#include <QVBoxLayout>
#include <QWidget>

struct Scratchpad::Private
{
    KisScratchPad *scratchpad = 0;
};


Scratchpad::Scratchpad(View *view, const QColor & defaultColor, QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->scratchpad = new KisScratchPad();
    d->scratchpad->setupScratchPad(view->view()->resourceProvider(), defaultColor);
    d->scratchpad->setMinimumSize(50, 50);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->scratchpad);

    // Forward KisScratchPad::scaleChanged to Scratchpad::scaleChanged
    connect(d->scratchpad, SIGNAL(scaleChanged(qreal)), this, SIGNAL(scaleChanged(qreal)));

    // Forward KisScratchPad::contentChanged to Scratchpad::contentChanged
    connect(d->scratchpad, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()));

    // Forward KisScratchPad::viewportChanged to Scratchpad::viewportChanged
    connect(d->scratchpad, SIGNAL(viewportChanged(const QRect)), this, SIGNAL(viewportChanged(const QRect)));
}

Scratchpad::~Scratchpad()
{
}

void Scratchpad::setModeManually(bool value)
{
    d->scratchpad->setModeManually(value);
}

void Scratchpad::setMode(QString modeType)
{
    d->scratchpad->setModeType(modeType);
}

void Scratchpad::linkCanvasZoom(bool value)
{
    qWarning("[DEPRECATED] Use Scratchpad.setCanvasZoomLink() instead of Scratchpad.linkCanvasZoom()");
    d->scratchpad->setCanvasZoomLink(value);
}

void Scratchpad::setCanvasZoomLink(bool value)
{
    d->scratchpad->setCanvasZoomLink(value);
}

bool Scratchpad::canvasZoomLink()
{
    return d->scratchpad->canvasZoomLink();
}

qreal Scratchpad::scale()
{
    // return scale X only, consider zoom X/Y will always be the same
    return d->scratchpad->scaleX();
}

bool Scratchpad::setScale(qreal scale) const
{
    if (d->scratchpad->canvasZoomLink()) {
        // if zoom level is linked to canvas zoom level, ignore setScale()
        return false;
    };
    return d->scratchpad->setScale(scale, scale);
}

void Scratchpad::scaleToFit()
{
    if (d->scratchpad->canvasZoomLink()) {
        // if zoom level is linked to canvas zoom level, ignore setScale()
        return;
    };
    d->scratchpad->scaleToFit();
}

void Scratchpad::scaleReset()
{
    if (d->scratchpad->canvasZoomLink()) {
        // if zoom level is linked to canvas zoom level, ignore setScale()
        return;
    };
    d->scratchpad->scaleReset();
}

void Scratchpad::panTo(qint32 x, qint32 y)
{
    d->scratchpad->panTo(x, y);
}

void Scratchpad::panCenter()
{
    d->scratchpad->panCenter();
}

void Scratchpad::loadScratchpadImage(QImage image)
{
    d->scratchpad->loadScratchpadImage(image);
}

QImage Scratchpad::copyScratchpadImageData()
{
    return d->scratchpad->copyScratchpadImageData();
}

void Scratchpad::clear()
{
    // need ability to set color
    d->scratchpad->fillDefault();
}

void Scratchpad::setFillColor(QColor color)
{
    d->scratchpad->setFillColor(color);
}

void Scratchpad::fillDefault()
{
    d->scratchpad->fillDefault();
}

void Scratchpad::fillTransparent()
{
    d->scratchpad->fillTransparent();
}

void Scratchpad::fillBackground()
{
    d->scratchpad->fillBackground();
}

void Scratchpad::fillForeground()
{
    d->scratchpad->fillForeground();
}

void Scratchpad::fillGradient(const QPoint &gradientVectorStart,
                              const QPoint &gradientVectorEnd,
                              const QString &gradientShape,
                              const QString &gradientRepeat,
                              bool reverseGradient,
                              bool dither)
{
    KisGradientPainter::enumGradientShape gradientShapeValue = KisGradientPainter::GradientShapeLinear;
    KisGradientPainter::enumGradientRepeat gradientRepeatValue = KisGradientPainter::GradientRepeatNone;

    if (gradientShape != "linear") {
        if (gradientShape == "bilinear") {
            gradientShapeValue = KisGradientPainter::GradientShapeBiLinear;
        } else if ( gradientShape == "radial") {
            gradientShapeValue = KisGradientPainter::GradientShapeRadial;
        } else if ( gradientShape == "square") {
            gradientShapeValue = KisGradientPainter::GradientShapeSquare;
        } else if ( gradientShape == "conical") {
            gradientShapeValue = KisGradientPainter::GradientShapeConical;
        } else if ( gradientShape == "conicalSymmetric") {
            gradientShapeValue = KisGradientPainter::GradientShapeConicalSymetric;
        } else if ( gradientShape == "spiral") {
            gradientShapeValue = KisGradientPainter::GradientShapeSpiral;
        } else if ( gradientShape == "reverseSpiral") {
            gradientShapeValue = KisGradientPainter::GradientShapeReverseSpiral;
        } else if ( gradientShape == "polygonal") {
            gradientShapeValue = KisGradientPainter::GradientShapePolygonal;
        }
    }

    if (gradientRepeat != "none") {
        if (gradientRepeat == "alternate") {
            gradientRepeatValue = KisGradientPainter::GradientRepeatAlternate;
        } else if (gradientRepeat == "forwards") {
            gradientRepeatValue = KisGradientPainter::GradientRepeatForwards;
        }
    }
    d->scratchpad->fillGradient(gradientVectorStart, gradientVectorEnd, gradientShapeValue, gradientRepeatValue, reverseGradient, dither);
}

void Scratchpad::fillLayer(bool fullContent)
{
    d->scratchpad->fillLayer(fullContent);
}

void Scratchpad::fillDocument(bool fullContent)
{
    d->scratchpad->fillDocument(fullContent);
}

void Scratchpad::fillPattern(QTransform transform)
{
    d->scratchpad->fillPattern(transform);
}

QRect Scratchpad::viewportBounds() const
{
    return d->scratchpad->imageBounds();
}

QRect Scratchpad::contentBounds() const
{
    return d->scratchpad->contentBounds();
}
