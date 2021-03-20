/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Shape.h"
#include <kis_icon_utils.h>
#include <SvgWriter.h>
#include <SvgParser.h>
#include <SvgSavingContext.h>
#include <QBuffer>
#include <KoDocumentResourceManager.h>
struct Shape::Private {
    Private() {}
    KoShape *shape {0};
};

Shape::Shape(KoShape *shape, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->shape = shape;
}

Shape::~Shape()
{
    delete d;
}

QString Shape::name() const
{
    return d->shape->name();
}

void Shape::setName(const QString &name)
{
    d->shape->setName(name);
}

QString Shape::type() const
{
    return d->shape->shapeId();
}

bool Shape::visible()
{
    return d->shape->isVisible();
}

void Shape::setVisible(bool visible)
{
    d->shape->setVisible(visible);
}

QRectF Shape::boundingBox() const
{
    return d->shape->boundingRect();
}

QPointF Shape::position() const
{
    return d->shape->position();
}

void Shape::setPosition(QPointF point)
{
    d->shape->setPosition(point);
}

QString Shape::toSvg()
{
    QBuffer shapesBuffer;
    QBuffer stylesBuffer;

    shapesBuffer.open(QIODevice::WriteOnly);
    stylesBuffer.open(QIODevice::WriteOnly);

    {
        SvgSavingContext savingContext(shapesBuffer, stylesBuffer);
        savingContext.setStrippedTextMode(true);
        SvgWriter writer({d->shape});
        writer.saveDetached(savingContext);
    }

    shapesBuffer.close();
    stylesBuffer.close();

    return QString::fromUtf8(shapesBuffer.data());
}

KoShape *Shape::shape()
{
    return d->shape;
}
