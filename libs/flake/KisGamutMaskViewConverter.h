#ifndef KISGAMUTMASKVIEWCONVERTER_H
#define KISGAMUTMASKVIEWCONVERTER_H

#include "kritaflake_export.h"

#include <QtGlobal>
#include <KoViewConverter.h>
#include <QSizeF>

class QPointF;
class QRectF;

/**
 * @brief view convertor for gamut mask calculations and painting; 0,0 in the center
 */
class KRITAFLAKE_EXPORT KisGamutMaskViewConverter : public KoViewConverter
{
public:
    KisGamutMaskViewConverter();
    ~KisGamutMaskViewConverter();

    void setViewSize(QSize viewSize);
    void setMaskSize(QSizeF maskSize);

    QPointF documentToView(const QPointF &documentPoint) const override;
    QPointF viewToDocument(const QPointF &viewPoint) const override;

    QRectF documentToView(const QRectF &documentRect) const override;
    QRectF viewToDocument(const QRectF &viewRect) const override;

    QSizeF documentToView(const QSizeF& documentSize) const override;
    QSizeF viewToDocument(const QSizeF& viewSize) const override;

    qreal documentToViewX(qreal documentX) const override;
    qreal documentToViewY(qreal documentY) const override;
    qreal viewToDocumentX(qreal viewX) const override;
    qreal viewToDocumentY(qreal viewY) const override;

    void setZoom(qreal zoom) override;
    void zoom(qreal *zoomX, qreal *zoomY) const override;

private:
    void computeAndSetZoom();

    qreal m_zoomLevel; // 1.0 is 100%
    int m_viewSize;
    QSizeF m_maskSize;
    qreal m_maskResolution;
};

#endif // KISGAMUTMASKVIEWCONVERTER_H
