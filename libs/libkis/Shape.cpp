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
#include <kis_processing_applicator.h>
#include <KisPart.h>
#include <KisView.h>
#include <KisDocument.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
#include <KoShapeController.h>
#include <KoSelection.h>

#include "Krita.h"
#include "Document.h"
#include "GroupShape.h"

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

bool Shape::operator==(const Shape &other) const
{
    return (d->shape == other.d->shape);
}

bool Shape::operator!=(const Shape &other) const
{
    return !(operator==(other));
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

int Shape::zIndex() const
{
    return d->shape->zIndex();
}

void Shape::setZIndex(int zindex)
{
    d->shape->setZIndex(zindex);
}

bool Shape::selectable() const
{
    return d->shape->isSelectable();
}

void Shape::setSelectable(bool selectable)
{
    d->shape->setSelectable(selectable);
}

bool Shape::geometryProtected() const
{
    return d->shape->isGeometryProtected();
}

void Shape::setGeometryProtected(bool protect)
{
    d->shape->setGeometryProtected(protect);
}

bool Shape::visible() const
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

QTransform Shape::transformation() const
{
    return d->shape->transformation();
}

void Shape::setTransformation(QTransform matrix)
{
    d->shape->setTransformation(matrix);
}

QTransform Shape::absoluteTransformation() const
{
    return d->shape->absoluteTransformation();
}

void Shape::update()
{
    return d->shape->update();
}

void Shape::updateAbsolute(QRectF box)
{
    return d->shape->updateAbsolute(box);
}

bool Shape::remove()
{
    if (!d->shape) return false;
    if (!d->shape->parent()) return false;

    bool removeStatus = false;
    Document *document = Krita::instance()->activeDocument();

    if (KisPart::instance()->viewCount(document->document()) > 0) {
        for (QPointer<KisView> view : KisPart::instance()->views()) {
            if (view && view->document() == document->document()) {
                KisProcessingApplicator::runSingleCommandStroke(view->image(), view->canvasBase()->shapeController()->removeShape(d->shape));
                view->image()->waitForDone();
                removeStatus = true;
                break;
            }
        }
    }

    delete document;

    return removeStatus;
}

QString Shape::toSvg(bool prependStyles, bool stripTextMode)
{
    QBuffer shapesBuffer;
    QBuffer stylesBuffer;

    shapesBuffer.open(QIODevice::WriteOnly);
    stylesBuffer.open(QIODevice::WriteOnly);

    {
        SvgSavingContext savingContext(shapesBuffer, stylesBuffer);
        savingContext.setStrippedTextMode(stripTextMode);
        SvgWriter writer({d->shape});
        writer.saveDetached(savingContext);
    }

    shapesBuffer.close();
    stylesBuffer.close();

    return (prependStyles ? QString::fromUtf8(stylesBuffer.data()):"") + QString::fromUtf8(shapesBuffer.data());
}

void Shape::select()
{
    if (!d->shape) return;

    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();
    KoSelection *selection = activeView->canvasBase()->shapeManager()->selection();

    selection->select(d->shape);
}

void Shape::deselect()
{
    if (!d->shape) return;

    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();
    KoSelection *selection = activeView->canvasBase()->shapeManager()->selection();

    selection->deselect(d->shape);
}

bool Shape::isSelected()
{
    if (!d->shape) return false;

    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();
    KoSelection *selection = activeView->canvasBase()->shapeManager()->selection();

    return selection->isSelected(d->shape);
}

Shape* Shape::parentShape() const
{
    if (!d->shape) return 0;
    if (!d->shape->parent()) return 0;

    if (dynamic_cast<KoShapeGroup*>(d->shape->parent())) {
        return new GroupShape(dynamic_cast<KoShapeGroup*>(d->shape->parent()));
    } else {
        return 0;
    }
}


KoShape *Shape::shape()
{
    return d->shape;
}
