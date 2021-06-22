/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoShapeFillWrapper.h"

#include <KoShape.h>
#include <QList>
#include <QBrush>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoMeshGradientBackground.h>
#include <KoShapeStroke.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeStrokeCommand.h>
#include <KoStopGradient.h>

#include "kis_assert.h"
#include "kis_debug.h"
#include "kis_global.h"

#include <KoFlakeUtils.h>

struct ShapeBackgroundFetchPolicy
{
    typedef KoFlake::FillType Type;

    typedef QSharedPointer<KoShapeBackground> PointerType;
    static PointerType getBackground(KoShape *shape) {
        return shape->background();
    }
    static Type type(KoShape *shape) {
        QSharedPointer<KoShapeBackground> background = shape->background();
        QSharedPointer<KoColorBackground> colorBackground = qSharedPointerDynamicCast<KoColorBackground>(background);
        QSharedPointer<KoGradientBackground> gradientBackground = qSharedPointerDynamicCast<KoGradientBackground>(background);
        QSharedPointer<KoPatternBackground> patternBackground = qSharedPointerDynamicCast<KoPatternBackground>(background);
        QSharedPointer<KoMeshGradientBackground> meshgradientBackground = qSharedPointerDynamicCast<KoMeshGradientBackground>(background);


        if(gradientBackground) {
            return Type::Gradient;
        }

        if (patternBackground) {
            return Type::Pattern;
        }

        if (colorBackground) {
            return Type::Solid;
        }

        if (meshgradientBackground) {
            return Type::MeshGradient;
        }

        return Type::None;
    }

    static QColor color(KoShape *shape) {
        QSharedPointer<KoColorBackground> colorBackground = qSharedPointerDynamicCast<KoColorBackground>(shape->background());
        return colorBackground ? colorBackground->color() : QColor();
    }

    static const QGradient* gradient(KoShape *shape) {
        QSharedPointer<KoGradientBackground> gradientBackground = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
        return gradientBackground ? gradientBackground->gradient() : 0;
    }

    static QTransform gradientTransform(KoShape *shape) {
        QSharedPointer<KoGradientBackground> gradientBackground = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
        return gradientBackground ? gradientBackground->transform() : QTransform();
    }

    static const SvgMeshGradient* meshgradient(KoShape *shape) {
        QSharedPointer<KoMeshGradientBackground> meshgradientBackground = qSharedPointerDynamicCast<KoMeshGradientBackground>(shape->background());
        return meshgradientBackground ? meshgradientBackground->gradient() : nullptr;
    }

    static QTransform meshgradientTransform(KoShape *shape) {
        QSharedPointer<KoMeshGradientBackground> meshgradientBackground = qSharedPointerDynamicCast<KoMeshGradientBackground>(shape->background());
        return meshgradientBackground ? meshgradientBackground->transform() : QTransform();
    }

    static bool compareTo(PointerType p1, PointerType p2) {
        return p1->compareTo(p2.data());
    }
};

struct ShapeStrokeFillFetchPolicy
{
    typedef KoFlake::FillType Type;

    typedef KoShapeStrokeModelSP PointerType;
    static PointerType getBackground(KoShape *shape) {
        return shape->stroke();
    }
    static Type type(KoShape *shape) {
        KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
        if (!stroke) return Type::None;

        // Pattern type not implemented yet, so that logic will have to be added here later
        if (stroke->lineBrush().gradient()) {
            return Type::Gradient;
        } else {

            // strokes without any width are none
            if (stroke->color().isValid() && stroke->lineWidth() != 0.0) {
                return Type::Solid;
            }

            return Type::None;
        }
    }

    static QColor color(KoShape *shape) {
        KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
        return stroke ? stroke->color() : QColor();
    }

    static const QGradient* gradient(KoShape *shape) {
        KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
        return stroke ? stroke->lineBrush().gradient() : 0;
    }

    static QTransform gradientTransform(KoShape *shape) {
        KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
        return stroke ? stroke->lineBrush().transform() : QTransform();
    }

    static bool compareTo(PointerType p1, PointerType p2) {
        return p1->compareFillTo(p2.data());
    }
};


template <class Policy>
bool compareBackgrounds(const QList<KoShape*> shapes)
{
    if (shapes.size() == 1) return true;

    typename Policy::PointerType bg =
        Policy::getBackground(shapes.first());

    Q_FOREACH (KoShape *shape, shapes) {
        if (
            !(
              (!bg && !Policy::getBackground(shape)) ||
              (bg && Policy::compareTo(bg, Policy::getBackground(shape)))
             )) {

            return false;
        }
    }

    return true;
}

/******************************************************************************/
/*             KoShapeFillWrapper::Private                                    */
/******************************************************************************/

struct KoShapeFillWrapper::Private
{
    QList<KoShape*> shapes;
    KoFlake::FillVariant fillVariant= KoFlake::Fill;

    QSharedPointer<KoShapeBackground> applyFillGradientStops(KoShape *shape, const QGradient *srcQGradient);
    void applyFillGradientStops(KoShapeStrokeSP shapeStroke, const QGradient *stopGradient);
};

QSharedPointer<KoShapeBackground> KoShapeFillWrapper::Private::applyFillGradientStops(KoShape *shape, const QGradient *stopGradient)
{
    QGradientStops stops = stopGradient->stops();

    if (!shape || !stops.count()) {
        return QSharedPointer<KoShapeBackground>();
    }

    KoGradientBackground *newGradient = 0;
    QSharedPointer<KoGradientBackground> oldGradient = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
    if (oldGradient) {
        // just copy the gradient and set the new stops
        QGradient *g = KoFlake::mergeGradient(oldGradient->gradient(), stopGradient);
        newGradient = new KoGradientBackground(g);
        newGradient->setTransform(oldGradient->transform());
    }
    else {
        // No gradient yet, so create a new one.
        QScopedPointer<QLinearGradient> fakeShapeGradient(new QLinearGradient(QPointF(0, 0), QPointF(1, 1)));
        fakeShapeGradient->setCoordinateMode(QGradient::ObjectBoundingMode);

        QGradient *g = KoFlake::mergeGradient(fakeShapeGradient.data(), stopGradient);
        newGradient = new KoGradientBackground(g);
    }
    return QSharedPointer<KoGradientBackground>(newGradient);
}

void KoShapeFillWrapper::Private::applyFillGradientStops(KoShapeStrokeSP shapeStroke, const QGradient *stopGradient)
{
    QGradientStops stops = stopGradient->stops();
    if (!stops.count()) return;

    QLinearGradient fakeShapeGradient(QPointF(0, 0), QPointF(1, 1));
    fakeShapeGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    QTransform gradientTransform;
    const QGradient *shapeGradient = 0;

    {
        QBrush brush = shapeStroke->lineBrush();
        gradientTransform = brush.transform();
        shapeGradient = brush.gradient() ? brush.gradient() : &fakeShapeGradient;
    }

    {
        QScopedPointer<QGradient> g(KoFlake::mergeGradient(shapeGradient, stopGradient));
        QBrush newBrush = *g;
        newBrush.setTransform(gradientTransform);
        shapeStroke->setLineBrush(newBrush);
    }
}

/******************************************************************************/
/*             KoShapeFillWrapper                                             */
/******************************************************************************/

KoShapeFillWrapper::KoShapeFillWrapper(KoShape *shape, KoFlake::FillVariant fillVariant)
    : m_d(new Private())
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);
    m_d->shapes << shape;
    m_d->fillVariant= fillVariant;
}


KoShapeFillWrapper::KoShapeFillWrapper(QList<KoShape*> shapes, KoFlake::FillVariant fillVariant)
    : m_d(new Private())
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!shapes.isEmpty());
    m_d->shapes = shapes;
    m_d->fillVariant= fillVariant;
}

KoShapeFillWrapper::~KoShapeFillWrapper()
{
}

bool KoShapeFillWrapper::isMixedFill() const
{
    if (m_d->shapes.isEmpty()) return false;

    return m_d->fillVariant == KoFlake::Fill ?
        !compareBackgrounds<ShapeBackgroundFetchPolicy>(m_d->shapes) :
        !compareBackgrounds<ShapeStrokeFillFetchPolicy>(m_d->shapes);
}

KoFlake::FillType KoShapeFillWrapper::type() const
{
    if (m_d->shapes.isEmpty() || isMixedFill()) return KoFlake::None;

    KoShape *shape = m_d->shapes.first();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, KoFlake::None);

    KoFlake::FillType fillType;
    if (m_d->fillVariant == KoFlake::Fill) {
        // fill property of vector object
        fillType = ShapeBackgroundFetchPolicy::type(shape);
    } else {
        // stroke property of vector object
        fillType = ShapeStrokeFillFetchPolicy::type(shape);
    }

    return fillType;
}

QColor KoShapeFillWrapper::color() const
{
    // this check guarantees that the shapes list is not empty and
    // the fill is not mixed!
    if (type() != KoFlake::Solid) return QColor();

    KoShape *shape = m_d->shapes.first();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, QColor());

    return m_d->fillVariant == KoFlake::Fill ?
        ShapeBackgroundFetchPolicy::color(shape) :
        ShapeStrokeFillFetchPolicy::color(shape);
}

const QGradient* KoShapeFillWrapper::gradient() const
{
    // this check guarantees that the shapes list is not empty and
    // the fill is not mixed!
    if (type() != KoFlake::Gradient) return 0;

    KoShape *shape = m_d->shapes.first();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, 0);

    return m_d->fillVariant == KoFlake::Fill ?
        ShapeBackgroundFetchPolicy::gradient(shape) :
        ShapeStrokeFillFetchPolicy::gradient(shape);
}

QTransform KoShapeFillWrapper::gradientTransform() const
{
    // this check guarantees that the shapes list is not empty and
    // the fill is not mixed!
    if (type() != KoFlake::Gradient) return QTransform();

    KoShape *shape = m_d->shapes.first();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, QTransform());

    return m_d->fillVariant == KoFlake::Fill ?
        ShapeBackgroundFetchPolicy::gradientTransform(shape) :
                ShapeStrokeFillFetchPolicy::gradientTransform(shape);
}

const SvgMeshGradient* KoShapeFillWrapper::meshgradient() const
{
    if (type() != KoFlake::MeshGradient) return nullptr;

    KoShape *shape = m_d->shapes.first();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, 0);

    return m_d->fillVariant == KoFlake::Fill ?
        ShapeBackgroundFetchPolicy::meshgradient(shape) :
        nullptr;
}

KUndo2Command *KoShapeFillWrapper::setColor(const QColor &color)
{
    KUndo2Command *command = 0;

    if (m_d->fillVariant == KoFlake::Fill) {
         QSharedPointer<KoShapeBackground> bg;

        if (color.isValid()) {
            bg = toQShared(new KoColorBackground(color));
        }

        QSharedPointer<KoShapeBackground> fill(bg);
        command = new KoShapeBackgroundCommand(m_d->shapes, fill);
    } else {
        command = KoFlake::modifyShapesStrokes(m_d->shapes,
            [color] (KoShapeStrokeSP stroke) {
                stroke->setLineBrush(Qt::NoBrush);
                stroke->setColor(color);

            });
    }

    return command;
}

KUndo2Command *KoShapeFillWrapper::setLineWidth(const float &lineWidth)
{
    KUndo2Command *command = 0;

    command = KoFlake::modifyShapesStrokes(m_d->shapes, [lineWidth](KoShapeStrokeSP stroke) {
            stroke->setColor(Qt::transparent);
            stroke->setLineWidth(lineWidth);

     });

   return command;
}


bool KoShapeFillWrapper::hasZeroLineWidth() const
{
        KoShape *shape = m_d->shapes.first();
        if (!shape) return false;
        if (m_d->fillVariant == KoFlake::Fill)  return false;

        // this check is useful to determine if
        KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
        if (!stroke) return false;

        if ( stroke->lineWidth() == 0.0) {
            return true;
        }

        return false;
}


KUndo2Command *KoShapeFillWrapper::setGradient(const QGradient *gradient, const QTransform &transform)
{
    KUndo2Command *command = 0;

    if (m_d->fillVariant == KoFlake::Fill) {
        QList<QSharedPointer<KoShapeBackground>> newBackgrounds;

        foreach (KoShape *shape, m_d->shapes) {
            Q_UNUSED(shape);

            KoGradientBackground *newGradient = new KoGradientBackground(KoFlake::cloneGradient(gradient));
            newGradient->setTransform(transform);
            newBackgrounds << toQShared(newGradient);
        }

        command = new KoShapeBackgroundCommand(m_d->shapes, newBackgrounds);

    } else {
        command = KoFlake::modifyShapesStrokes(m_d->shapes,
            [gradient, transform] (KoShapeStrokeSP stroke) {
                QBrush newBrush = *gradient;
                newBrush.setTransform(transform);

                stroke->setLineBrush(newBrush);
                stroke->setColor(Qt::transparent);
            });
    }

    return command;
}

KUndo2Command* KoShapeFillWrapper::applyGradient(const QGradient *gradient)
{
    return setGradient(gradient, gradientTransform());
}

KUndo2Command* KoShapeFillWrapper::applyGradientStopsOnly(const QGradient *gradient)
{
    KUndo2Command *command = 0;

    if (m_d->fillVariant == KoFlake::Fill) {
        QList<QSharedPointer<KoShapeBackground>> newBackgrounds;

        foreach (KoShape *shape, m_d->shapes) {
            newBackgrounds <<  m_d->applyFillGradientStops(shape, gradient);
        }

        command = new KoShapeBackgroundCommand(m_d->shapes, newBackgrounds);

    } else {
        command = KoFlake::modifyShapesStrokes(m_d->shapes,
            [this, gradient] (KoShapeStrokeSP stroke) {
                m_d->applyFillGradientStops(stroke, gradient);
            });
    }

    return command;
}

KUndo2Command* KoShapeFillWrapper::setMeshGradient(const SvgMeshGradient *gradient,
                                                   const QTransform &transform)
{
    KUndo2Command *command = nullptr;
    if (m_d->fillVariant == KoFlake::Fill) {
        QList<QSharedPointer<KoShapeBackground>> newBackgrounds;

        for (const auto &shape: m_d->shapes) {
            Q_UNUSED(shape);
            KoMeshGradientBackground *newBackground =
                new KoMeshGradientBackground(gradient, transform);

            newBackgrounds << toQShared(newBackground);
        }
        command = new KoShapeBackgroundCommand(m_d->shapes, newBackgrounds);
    }
    // TODO: for strokes!!
    return command;
}
