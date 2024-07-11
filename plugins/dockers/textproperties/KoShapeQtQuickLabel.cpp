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

struct KoShapeQtQuickLabel::Private {
    KoShapeGroup *shape = nullptr;
    QString svgData;
    int imagePadding = 5;
    qreal imageScale = 1;
};

KoShapeQtQuickLabel::KoShapeQtQuickLabel(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , d(new Private)
{

}

KoShapeQtQuickLabel::~KoShapeQtQuickLabel()
{
}

void KoShapeQtQuickLabel::paint(QPainter *painter)
{
    if (!painter->isActive()) return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(0, 0, width(), height(), fillColor());
    if (!d->shape) {
        painter->restore();
        return;
    }

    QRectF bbox = d->shape->boundingRect();
    qreal scale = (height()- d->imagePadding*2) / bbox.height();

    painter->translate(boundingRect().center());
    painter->scale(scale, scale);
    painter->translate(-bbox.center());

    qreal pixelRatio = this->window()->effectiveDevicePixelRatio();
    QRectF scaledBounds = QRectF(0, 0, this->boundingRect().width()*pixelRatio, this->boundingRect().height()*pixelRatio);

    painter->setClipRect(painter->transform().inverted().mapRect(scaledBounds));
    painter->setPen(Qt::transparent);

    Q_FOREACH(const KoShape *shape, d->shape->shapes()) {
        shape->paint(*painter);
    }

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
    qreal max = qMax(60.0, qMax(width()-(d->imagePadding*2), height()-(d->imagePadding*2)));
    QRectF bb = QRectF( 0, 0, max, max);
    QSizeF sz = bb.size();
    p.setResolution(bb, 72);

    QList<KoShape*> shapes = p.parseSvg(doc.documentElement(), &sz);
    if (shapes.isEmpty()) return;
    d->shape = new KoShapeGroup();
    KoShapeGroupCommand cmd(d->shape, shapes, false);
    cmd.redo();

    d->shape->setAbsolutePosition(QPointF(), KoFlake::TopLeft);

    qreal scale = bb.height() / sz.height();

    setImplicitHeight(bb.height());

    emit svgDataChanged();
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

    emit foregroundColorChanged();
}

int KoShapeQtQuickLabel::imagePadding() const
{
    return d->imagePadding;
}

void KoShapeQtQuickLabel::setImagePadding(int newPadding)
{
    if (d->imagePadding == newPadding)
        return;
    d->imagePadding = newPadding;
    emit imagePaddingChanged();
}

qreal KoShapeQtQuickLabel::imageScale() const
{
    return d->imageScale;
}

void KoShapeQtQuickLabel::setImageScale(qreal newImageScale)
{
    if (qFuzzyCompare(d->imageScale, newImageScale))
        return;
    d->imageScale = newImageScale;
    emit imageScaleChanged();
}
