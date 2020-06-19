#include "KoMeshGradientBackground.h"
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
