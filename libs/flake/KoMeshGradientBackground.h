#ifndef KOMESHGRADIENTBACKGROUND_H
#define KOMESHGRADIENTBACKGROUND_H

#include "KoShapeBackground.h"
#include <QSharedDataPointer>
#include "SvgMeshGradient.h"

class KRITAFLAKE_EXPORT KoMeshGradientBackground : public KoShapeBackground
{
public:
    KoMeshGradientBackground(SvgMeshGradient *gradient, const QTransform &matrix = QTransform());
    ~KoMeshGradientBackground();

    void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const override;

    void fillPatch(QPainter &painter, const SvgMeshPatch *patch) const;

    bool compareTo(const KoShapeBackground *other) const override;

    void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) override;

    bool loadStyle(KoOdfLoadingContext &context, const QSizeF &shapeSize) override;

private:
    class Private;
    QSharedDataPointer<Private> d;
};


#endif // KOMESHGRADIENTBACKGROUND_H
