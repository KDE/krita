#include "KoMeshGradientBackground.h"
#include <KoColorSpaceRegistry.h>
#include <KoMixColorsOp.h>
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

    ~Private() { delete gradient; }

    SvgMeshGradient *gradient;
    QTransform matrix;
};

KoMeshGradientBackground::KoMeshGradientBackground(SvgMeshGradient *gradient, const QTransform &matrix)
    : KoShapeBackground()
    , d(new Private)
{
    d->gradient = gradient;
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoMeshGradientBackground::~KoMeshGradientBackground()
{
}

void KoMeshGradientBackground::paint(QPainter &painter,
                                     KoShapePaintingContext &context,
                                     const QPainterPath &fillPath) const
{
    if (!d->gradient || !d->gradient->isValid())   return;

    for (int row = 0; row < d->gradient->getMeshArray()->numRows(); ++row) {
        for (int col = 0; col < d->gradient->getMeshArray()->numColumns(); ++col) {
            SvgMeshPatch *patch = d->gradient->getMeshArray()->getPatch(row, col);
            fillPatch(painter, patch);
        }
    }
}

void KoMeshGradientBackground::fillPatch(QPainter &painter, const SvgMeshPatch *patch) const
{
    QColor color0 = patch->getStop(SvgMeshPatch::Top).color;
    QColor color1 = patch->getStop(SvgMeshPatch::Right).color;
    QColor color2 = patch->getStop(SvgMeshPatch::Bottom).color;
    QColor color3 = patch->getStop(SvgMeshPatch::Left).color;

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    quint8 c[4][4];
    cs->fromQColor(color0, c[0]);
    cs->fromQColor(color1, c[1]);
    cs->fromQColor(color2, c[2]);
    cs->fromQColor(color3, c[3]);

    const quint8 threshold = 0;

    // check if color variation is acceptable and patch size is less than ~pixel width/heigh
    if ((cs->difference(c[0], c[1]) > threshold || cs->difference(c[1], c[2]) > threshold ||
         cs->difference(c[2], c[3]) > threshold || cs->difference(c[3], c[0]) > threshold) &&
        patch->size().width() > 1 && patch->size().height() > 1) {

        QVector<SvgMeshPatch*> patches;
        patch->subdivide(patches);

        for (const auto& p: patches) {
            fillPatch(painter, p);
        }

        for (auto& p: patches) {
            delete p;
        }
    } else {
        quint8 mixed[4];
        cs->mixColorsOp()->mixColors(c[0], 4, mixed);

        QColor average;
        cs->toQColor(mixed, &average);

        QPen pen(average);
        painter.setPen(pen);
        painter.drawPath(patch->getPath()->outline());

        QBrush brush(average);
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
