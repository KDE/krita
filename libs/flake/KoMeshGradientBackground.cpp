/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KoMeshGradientBackground.h"
#include <KoColorSpaceRegistry.h>
#include <KoMixColorsOp.h>
#include <kis_algebra_2d.h>

#include <QRegion>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include "KoMeshPatchesRenderer.h"

class KoMeshGradientBackground::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , gradient(0)
        , renderer(new KoMeshPatchesRenderer)
    {}

    Private(const Private& other)
        : QSharedData()
        , gradient(new SvgMeshGradient(*other.gradient))
        , matrix(other.matrix)
        , renderer(new KoMeshPatchesRenderer)
    {
    }

    ~Private() {
        delete renderer;
    }

    QScopedPointer<SvgMeshGradient> gradient;
    QTransform matrix;
    KoMeshPatchesRenderer *renderer;
};

KoMeshGradientBackground::KoMeshGradientBackground(const SvgMeshGradient *gradient, const QTransform &matrix)
    : KoShapeBackground()
    , d(new Private)
{
    d->gradient.reset(new SvgMeshGradient(*gradient));
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoMeshGradientBackground::~KoMeshGradientBackground()
{
}

KoMeshGradientBackground::KoMeshGradientBackground(const KoMeshGradientBackground &rhs)
    : d(new Private(*rhs.d))
{
}
KoMeshGradientBackground &KoMeshGradientBackground::operator=(const KoMeshGradientBackground &rhs)
{
    d = rhs.d;
    return *this;
}

void KoMeshGradientBackground::paint(QPainter &painter,
                                     KoShapePaintingContext &,
                                     const QPainterPath &fillPath) const
{
    if (!d->gradient || !d->gradient->isValid())   return;
    painter.save();

    QScopedPointer<SvgMeshGradient> gradient(new SvgMeshGradient(*d->gradient));

    QRectF meshBoundingRect = gradient->boundingRect();

    if (gradient->gradientUnits() == KoFlake::ObjectBoundingBox) {
        const QTransform relativeToShape = KisAlgebra2D::mapToRect(fillPath.boundingRect());
        gradient->setTransform(relativeToShape);
        meshBoundingRect = gradient->boundingRect();
    }

    if (d->renderer->patchImage()->isNull()) {

        d->renderer->configure(meshBoundingRect, painter.transform());
        SvgMeshArray *mesharray = gradient->getMeshArray().data();

        for (int row = 0; row < mesharray->numRows(); ++row) {
            for (int col = 0; col < mesharray->numColumns(); ++col) {
                SvgMeshPatch *patch = mesharray->getPatch(row, col);
                d->renderer->fillPatch(patch, gradient->type(), mesharray, row, col);
            }
        }
        // uncomment to debug
        //  d->renderer->patchImage()->save("mesh-patch.png");
    }

    painter.setClipPath(fillPath);

    // patch is to be drawn wrt. to "user" coordinates
    painter.drawImage(meshBoundingRect, *d->renderer->patchImage());

    painter.restore();
}

bool KoMeshGradientBackground::compareTo(const KoShapeBackground*) const
{
    return false;
}

SvgMeshGradient* KoMeshGradientBackground::gradient()
{
    return d->gradient.data();
}

QTransform KoMeshGradientBackground::transform()
{
    return d->matrix;
}
