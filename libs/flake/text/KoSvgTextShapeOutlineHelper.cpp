/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextShapeOutlineHelper.h"

#include <KoCanvasBase.h>
#include <kis_icon.h>
#include <KoCanvasResourceProvider.h>

#include <KoSvgTextShape.h>
#include <KisHandlePainterHelper.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <QApplication>
#include <QPalette>

const int BUTTON_ICON_SIZE = 16;
const int BUTTON_PADDING = 4;
const int BUTTON_CORNER_ROUND = 1;
const QString ICON_EXIT = "object-ungroup-calligra";
const QString ICON_ENTER = "object-group-calligra";

struct KoSvgTextShapeOutlineHelper::Private {
    Private(KoCanvasBase *canvasBase): canvas(canvasBase) {
    }
    KoCanvasBase *canvas;
    int handleRadius = 7;
    int decorationThickness = 1;

    bool drawBoundingRect = true;
    bool drawTextWrappingArea = false;
    bool drawOutline = false;

    KoSvgTextShape *getTextModeShape() {
        return canvas->textShapeManagerEnabled();
    }

    KoSvgTextShape *getPotentialTextShape(const QPointF &point) {
        Q_FOREACH(KoShape*shape, canvas->shapeManager()->selection()->selectedEditableShapes()) {
            KoSvgTextShape *text = dynamic_cast<KoSvgTextShape*>(shape);
            if (text) {
                if (getButtonRectCorrected(text->boundingRect()).contains(point)) {
                    return text;
                }
            }
        }
        return nullptr;
    }

    KoViewConverter *converter() const {
        return canvas->viewConverter();
    }

    QRectF getButtonRect(QRectF base) {
        const int buttonSize = BUTTON_ICON_SIZE + (2* BUTTON_PADDING);
        return QRectF(base.topRight(), QSizeF(buttonSize, buttonSize));
    }
    QRectF getButtonRectCorrected(QRectF base) {
        return converter()->viewToDocument().mapRect(getButtonRect(converter()->documentToView().mapRect(base)));
    }
};

KoSvgTextShapeOutlineHelper::KoSvgTextShapeOutlineHelper(KoCanvasBase *canvas)
    : d(new Private(canvas))
{
}

KoSvgTextShapeOutlineHelper::~KoSvgTextShapeOutlineHelper()
{

}

void KoSvgTextShapeOutlineHelper::paintTextShape(QPainter *painter, const KoViewConverter &converter,
                                                  const QPalette &pal, KoSvgTextShape *text,
                                                  bool contourModeActive) {
    painter->save();
    KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(painter, text, converter, d->handleRadius, d->decorationThickness);
    helper.setHandleStyle(KisHandleStyle::secondarySelection());
    if (contourModeActive) {
        if (d->drawOutline) {
            Q_FOREACH(KoShape *shape, text->internalShapeManager()->shapes()) {
                helper.drawPath(shape->transformation().map(shape->outline()));
            }
        }
        if (d->drawBoundingRect) {
            QPainterPath rect;
            rect.addRect(text->outlineRect());
            helper.drawPath(rect);
        }
    }
    if (d->drawTextWrappingArea) {
        Q_FOREACH(const QPainterPath path, text->textWrappingAreas()) {
            helper.drawPath(path);
        }
    }
    painter->restore();

    painter->save();
    //painter->setTransform(converter.viewToDocument() *
    //                      painter->transform());
    QIcon icon = contourModeActive? KisIconUtils::loadIcon(ICON_EXIT): KisIconUtils::loadIcon(ICON_ENTER);
    QPixmap pm = icon.pixmap(BUTTON_ICON_SIZE, BUTTON_ICON_SIZE);
    painter->setBrush(contourModeActive? pal.highlight(): pal.button());
    QPen pen;
    pen.setColor(contourModeActive? pal.highlightedText().color(): pal.buttonText().color());
    pen.setCosmetic(true);
    pen.setWidthF(d->decorationThickness);
    painter->setPen(pen);
    const QRectF buttonRect = d->getButtonRect(converter.documentToView().mapRect(text->boundingRect()));
    painter->drawRoundedRect(buttonRect, BUTTON_CORNER_ROUND, BUTTON_CORNER_ROUND);
    painter->drawPixmap(buttonRect.topLeft()+QPointF(BUTTON_PADDING, BUTTON_PADDING), pm);
    painter->restore();
}

void KoSvgTextShapeOutlineHelper::paint(QPainter *painter, const KoViewConverter &converter)
{
    const QPalette pal = qApp->palette();
    KoSvgTextShape *text = d->getTextModeShape();
    if (text) {
        paintTextShape(painter, converter, pal, text, true);
    } else {
        Q_FOREACH(KoShape* shape, d->canvas->shapeManager()->selection()->selectedEditableShapes()) {
            text = dynamic_cast<KoSvgTextShape*>(shape);
            if (text && !text->internalShapeManager()->shapes().isEmpty()) {
                paintTextShape(painter, converter, pal, text, false);
            }
        }
    }
}

QRectF KoSvgTextShapeOutlineHelper::decorationRect()
{
    QRectF decorationRect;
    KoSvgTextShape *text = d->getTextModeShape();
    if (text) {
        QRectF base = text->boundingRect();
        base |= d->getButtonRectCorrected(base);
        decorationRect = base;
    } else {
        Q_FOREACH(KoShape* shape, d->canvas->shapeManager()->selection()->selectedEditableShapes()) {
            text = dynamic_cast<KoSvgTextShape*>(shape);
            if (text && !text->internalShapeManager()->shapes().isEmpty()) {
                QRectF base = text->boundingRect();
                base |= d->getButtonRectCorrected(base);
                decorationRect |= base;
            }
        }
    }
    return decorationRect;
}

void KoSvgTextShapeOutlineHelper::setDrawBoundingRect(bool enable)
{
    d->drawBoundingRect = enable;
}

bool KoSvgTextShapeOutlineHelper::drawBoundingRect() const
{
    return d->drawBoundingRect;
}

void KoSvgTextShapeOutlineHelper::setDrawTextWrappingArea(bool enable)
{
    d->drawTextWrappingArea = enable;
}

void KoSvgTextShapeOutlineHelper::setDrawShapeOutlines(bool enable)
{
    d->drawOutline = enable;
}

bool KoSvgTextShapeOutlineHelper::drawShapeOutlines() const
{
    return d->drawOutline;
}

void KoSvgTextShapeOutlineHelper::setHandleRadius(int radius)
{
    d->handleRadius = radius;
}

void KoSvgTextShapeOutlineHelper::setDecorationThickness(int thickness)
{
    d->decorationThickness = thickness;
}

KoSvgTextShape *KoSvgTextShapeOutlineHelper::contourModeButtonHovered(const QPointF &point)
{
    if (d->getTextModeShape()) {
        if (d->getButtonRect(d->getTextModeShape()->boundingRect()).contains(point)) {
            return d->getTextModeShape();
        }
    }
    return d->getPotentialTextShape(point);
}

void KoSvgTextShapeOutlineHelper::toggleTextContourMode(KoSvgTextShape *shape)
{
    if (d->canvas) {
        if (shape == d->canvas->textShapeManagerEnabled()) {
            d->canvas->setTextShapeManagerEnabled(nullptr);
        } else {
            d->canvas->setTextShapeManagerEnabled(shape);
        }
    }
}
