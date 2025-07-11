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

    Private() {

    }

    ~Private(){
        shapePainter.reset();
        qDeleteAll(shapes);
    }

    QList<KoShape*> shapes;
    QScopedPointer<KoShapePainter> shapePainter;
    QString svgData;

    QColor fgColor = Qt::black;
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
    if (!d->shapePainter) return;
    if (!painter->isActive()) return;

    const QRectF bbox = d->documentRect.isValid()? d->documentRect: d->shapePainter->contentRect();
    const QRectF bounds = QRectF(0, 0, width(), height());

    if (d->shapes.isEmpty()) {
        return;
    }

    d->shapePainter->paint(*painter,
                           bounds.toAlignedRect(),
                           d->adjustBBoxToScaling(bbox, bounds));

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
    p.setInheritByDefault(true);

    QList<KoShape*> shapes = p.parseSvg(doc.documentElement(), &sz);

    // TODO: Evaluate if this can't be faster.
    d->shapePainter.reset(new KoShapePainter());

    if (!d->shapes.isEmpty()) {
        qDeleteAll(d->shapes);
    }

    d->shapes = shapes;
    d->shapePainter->setShapes(d->shapes);

    updateShapes();

    emit svgDataChanged();
    emit minimumRectChanged();
}

QColor KoShapeQtQuickLabel::foregroundColor() const
{
    return d->fgColor;
}

void KoShapeQtQuickLabel::setForegroundColor(const QColor &newForegroundColor)
{
    if (d->fgColor == newForegroundColor)
            return;

    d->fgColor = newForegroundColor;
    updateShapes();
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
    setOpaquePainting(fillColor().alpha() == 255);
}

void KoShapeQtQuickLabel::updateShapes()
{
    if (!d->fullColor) {
        QSharedPointer<KoColorBackground> bg(new KoColorBackground(d->fgColor));
        for (int i = 0; i < d->shapes.size(); i++) {
            d->shapes.at(i)->setBackground(bg);
        }
    }
    callUpdateIfComplete();
}

void KoShapeQtQuickLabel::callUpdateIfComplete()
{
    if (isComponentComplete() && isVisible()) {
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
