#ifndef KOGAMUTMASK_H
#define KOGAMUTMASK_H

#include <QPainter>
#include <QString>
#include <QVector>
#include <cmath>

#include <FlakeDebug.h>
#include <resources/KoResource.h>
#include <KoShape.h>
#include <KisGamutMaskViewConverter.h>
#include <KoShapePaintingContext.h>

class KoViewConverter;

class KoGamutMaskShape
{
public:
    KoGamutMaskShape(KoShape* shape);
    KoGamutMaskShape();
    ~KoGamutMaskShape();

    bool coordIsClear(const QPointF& coord, const KoViewConverter& viewConverter) const;
    QPainterPath outline();
    void paint(QPainter &painter, const KoViewConverter& viewConverter);
    KoShape* koShape();

private:
    KoShape* m_maskShape;
    KoShapePaintingContext m_shapePaintingContext;
};


/**
 * @brief The resource type for gamut masks used by the artistic color selector
 */
class KRITAFLAKE_EXPORT KoGamutMask : public QObject, public KoResource
{
    Q_OBJECT

public:
    /**
     * @brief load a gamut mask from given file
     * @param filename
     */
    KoGamutMask(const QString &filename);

    /**
     * @brief create KoGamutMask from polygons
     * @param polygons
     */
    KoGamutMask();

    // TODO: copy constructor, for duplicating masks
    KoGamutMask(KoGamutMask *rhs);

    bool coordIsClear(const QPointF& coord, KoViewConverter& viewConverter, bool preview);

    bool load() override __attribute__((optimize(0)));
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    void paint(QPainter &painter, KoViewConverter& viewConverter, bool preview);

    QString title();
    void setTitle(QString title);

    QString description();
    void setDescription(QString description);

    QSizeF maskSize();

    void setMaskShapes(QList<KoShape*> shapes);   
    void setPreviewMaskShapes(QList<KoShape*> shapes);
    void setMaskShapesToVector(QList<KoShape*> shapes, QVector<KoGamutMaskShape*>& targetVector);

    QList<KoShape*> koShapes() const;

    // switch back to loaded shapes when ending mask preview
    void clearPreview();
private:
    struct Private;
    Private* const d;
};

#endif // KOGAMUTMASK_H
