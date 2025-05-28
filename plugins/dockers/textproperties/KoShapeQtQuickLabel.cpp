/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoShapeQtQuickLabel.h"
#include <KoShape.h>
#include <SvgParser.h>
#include <KoColorBackground.h>
#include <KoDocumentResourceManager.h>
#include <QQuickWindow>
#include <KoShapeGroupCommand.h>
#include <KoShapeUngroupCommand.h>
#include <KoShapePainter.h>
#include <kis_assert.h>

struct KoShapeQtQuickLabel::Private {

    Private(): shape(new KoShapeGroup()) {

    }

    ~Private(){}

    QScopedPointer<KoShapeGroup> shape;
    QScopedPointer<KoShapePainter> shapePainter;
    QString svgData;
    bool fullColor = false;
    KoShapeQtQuickLabel::ScalingType scalingType = KoShapeQtQuickLabel::Fit;
    Qt::Alignment alignment = Qt::AlignHCenter | Qt::AlignVCenter;

    QRectF documentRect;

    int leftPadding = 0;
    int rightPadding = 0;
    int topPadding = 0;
    int bottomPadding = 0;

    QRectF adjustBBoxToScaling(const QRectF &bbox, const QRectF &widgetBounds) {
        const double newWidth = widgetBounds.width() - (leftPadding + rightPadding);
        const double newHeight = widgetBounds.height() - (topPadding + bottomPadding);

        QRectF newBox = bbox; ///< "Fit" behaviour.
        if (scalingType == KoShapeQtQuickLabel::FitWidth) {
            const double docWidth = bbox.width();
            const double docHeight = bbox.width() * (newHeight / newWidth);
            double docY = bbox.top();
            if (alignment.testFlag(Qt::AlignBottom)) {
                docY = bbox.bottom() - docHeight;
            } else if (alignment.testFlag(Qt::AlignVCenter)) {
                docY = (bbox.bottom()*0.5) - (docHeight*0.5);
            }
            newBox = QRectF(bbox.left(), docY, docWidth, docHeight);
        } else if (scalingType == KoShapeQtQuickLabel::FitHeight) {
            const double docWidth = bbox.height() * (newWidth / newHeight);
            const double docHeight = bbox.height();
            double docX = bbox.left();
            if (alignment.testFlag(Qt::AlignRight)) {
                docX = bbox.right() - docWidth;
            } else if (alignment.testFlag(Qt::AlignHCenter)) {
                docX = (bbox.right()*0.5) - (docWidth*0.5);
            }
            newBox = QRectF(docX, bbox.top(), docWidth, docHeight);
        }
        const double newTop = (topPadding/newHeight) * newBox.height();
        const double newBottom = (bottomPadding/newHeight) * newBox.height();
        const double newLeft = (leftPadding/newWidth) * newBox.width();
        const double newRight = (rightPadding/newWidth) * newBox.width();
        return newBox.adjusted(-newLeft, -newTop, newRight, newBottom);
    }

    QRectF addPaddingToBounds(QRectF bounds) {
        return bounds.adjusted(-leftPadding, -topPadding, rightPadding, bottomPadding);
    }

};

KoShapeQtQuickLabel::KoShapeQtQuickLabel(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , d(new Private())
{
    setAntialiasing(true);
    setOpaquePainting(true);

    connect(this, SIGNAL(minimumRectChanged()), this, SLOT(callUpdateIfComplete()));
}

KoShapeQtQuickLabel::~KoShapeQtQuickLabel()
{
}

void KoShapeQtQuickLabel::paint(QPainter *painter)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->shapePainter);
    if (!painter->isActive()) return;

    const QRectF bbox = d->documentRect.isValid()? d->documentRect: d->shapePainter->contentRect();
    const QRectF bounds = QRectF(0, 0, width(), height());
    painter->save();
    if (!d->shape) {
        painter->restore();
        return;
    }

    d->shapePainter->paint(*painter,
                           bounds.toAlignedRect(),
                           d->adjustBBoxToScaling(bbox, bounds));

    painter->restore();

}

QString KoShapeQtQuickLabel::svgData() const
{
    return d->svgData;
}

void KoShapeQtQuickLabel::setSvgData(const QString &newSvgData)
{
    if (d->svgData == newSvgData)
        return;
    d->svgData = newSvgData;

    QDomDocument doc = SvgParser::createDocumentFromSvg(d->svgData);
    KoDocumentResourceManager resourceManager;
    SvgParser p(&resourceManager);
    QRectF bb = QRectF( 0, 0, 200, 200);
    QSizeF sz = bb.size();
    p.setResolution(bb, 72);

    QList<KoShape*> shapes = p.parseSvg(doc.documentElement(), &sz);
    if (shapes.isEmpty()) return;
    if (!d->shape->shapes().isEmpty()) {
        KoShapeUngroupCommand unGroup(d->shape.data(), d->shape->shapes());
        unGroup.redo();
    }
    KoShapeGroupCommand group(d->shape.data(), shapes, false);
    group.redo();

    emit svgDataChanged();
    emit minimumRectChanged();
}

QColor KoShapeQtQuickLabel::foregroundColor() const
{
    QColor fg = Qt::black;
    if (d->shape && d->shape->background()) {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(d->shape->background().data());
        if (bg) {
            return bg->color();
        }

    }
    return fg;
}

void KoShapeQtQuickLabel::setForegroundColor(const QColor &newForegroundColor)
{
    if (!d->shape) return;
    KoColorBackground *bg = dynamic_cast<KoColorBackground*>(d->shape->background().data());
    if (bg) {
        if (bg->color() == newForegroundColor)
            return;
    }

    d->shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(newForegroundColor)));
    callUpdateIfComplete();
    emit foregroundColorChanged();
}

int KoShapeQtQuickLabel::padding() const
{
    return (d->leftPadding + d->rightPadding + d->topPadding + d->bottomPadding) / 4;
}

void KoShapeQtQuickLabel::setPadding(int newPadding)
{
    const int currentPadding = (d->leftPadding + d->rightPadding + d->topPadding + d->bottomPadding) / 4;
    if (currentPadding == newPadding)
        return;
    d->leftPadding = newPadding;
    d->rightPadding = newPadding;
    d->topPadding = newPadding;
    d->bottomPadding = newPadding;

    emit paddingChanged();
    emit minimumRectChanged();
}

bool KoShapeQtQuickLabel::fullColor() const
{
    return d->fullColor;
}

void KoShapeQtQuickLabel::setFullColor(bool newFullColor)
{
    if (d->fullColor == newFullColor)
        return;
    d->fullColor = newFullColor;

    emit fullColorChanged();
}

KoShapeQtQuickLabel::ScalingType KoShapeQtQuickLabel::scalingType() const
{
    return d->scalingType;
}

void KoShapeQtQuickLabel::setScalingType(const ScalingType type)
{
    if (d->scalingType == type) return;
    d->scalingType = type;
    emit scalingTypeChanged();
}

Qt::Alignment KoShapeQtQuickLabel::alignment() const
{
    return d->alignment;
}

void KoShapeQtQuickLabel::setAlignment(const Qt::Alignment align)
{
    if (d->alignment == align) return;
    d->alignment = align;
    emit alignmentChanged();
}

void KoShapeQtQuickLabel::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    updateShapes();
    if (!d->shapePainter) {
        d->shapePainter.reset(new KoShapePainter());
        d->shapePainter->setShapes({d->shape.data()});
        emit minimumRectChanged();
    }
    setOpaquePainting(fillColor().alpha() == 255);
}

void KoShapeQtQuickLabel::updateShapes()
{
    if (d->shape) {
        for (int i = 0; i < d->shape->shapes().size(); i++) {
            d->shape->shapes().at(i)->setInheritBackground(!d->fullColor);
        }
    }
}

void KoShapeQtQuickLabel::callUpdateIfComplete()
{
    if (isComponentComplete()) {
        update(boundingRect().toAlignedRect());
    }
}

QRectF KoShapeQtQuickLabel::documentRect() const
{
    return d->documentRect;
}

void KoShapeQtQuickLabel::setDocumentRect(const QRectF &rect)
{
    if (d->documentRect == rect) return;
    d->documentRect = rect;
    emit documentRectChanged();
    emit minimumRectChanged();
}

QRectF KoShapeQtQuickLabel::minimumRect() const
{
    const QRectF bounds = d->documentRect.isValid()? d->documentRect: d->shapePainter? d->shapePainter->contentRect(): QRectF();
    return d->addPaddingToBounds(bounds);
}
