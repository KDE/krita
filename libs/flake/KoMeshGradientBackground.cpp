#include "KoMeshGradientBackground.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

class KoMeshGradientBackground::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , gradient(0)
    {}

    SvgMeshGradient *gradient;
    QTransform matrix;
};

KoMeshGradientBackground::~KoMeshGradientBackground()
{
    delete d->gradient;
}

KoMeshGradientBackground::KoMeshGradientBackground(SvgMeshGradient *gradient, const QTransform &matrix)
    : KoShapeBackground()
    , d(new Private)
{
    d->gradient = gradient;
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

void KoMeshGradientBackground::paint(QPainter &painter,
                                     KoShapePaintingContext &context,
                                     const QPainterPath &fillPath) const
{
    if (!d->gradient || !d->gradient->isValid())   return;

    for (int row = 0; row < d->gradient->getMeshArray()->numRows(); ++row) {
        for (int col = 0; col < d->gradient->getMeshArray()->numColumns(); ++col) {
            SvgMeshPatch *patch = d->gradient->getMeshArray()->getPatch(row, col);
            fillPatch(painter, patch, 0);
        }
    }
}

void KoMeshGradientBackground::fillPatch(QPainter &painter, const SvgMeshPatch *patch, int i) const
{
    // TODO check for color variation
    if (i <= 5) {
        QVector<SvgMeshPatch*> patches;
        patch->subdivide(patches);
        ++i;

        for (const auto& p: patches) {
            fillPatch(painter, p, i);
        }

        for (auto& p: patches) {
            delete p;
        }
    } else {
        QPen pen(patch->getStop(SvgMeshPatch::Bottom)->color);
        painter.setPen(pen);
        painter.drawPath(patch->getPath()->outline());

        QBrush brush(patch->getStop(SvgMeshPatch::Bottom)->color);
        painter.fillPath(patch->getPath()->outline(), brush);
    }
}

bool KoMeshGradientBackground::compareTo(const KoShapeBackground *other) const
{
    return false;
}

void KoMeshGradientBackground::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{

}

bool KoMeshGradientBackground::loadStyle(KoOdfLoadingContext &context, const QSizeF &shapeSize)
{
    return false;
}
