#include "KisGamutMaskViewConverter.h"

#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <FlakeDebug.h>

//#define DEBUG_GAMUT_MASK_CONVERTER

KisGamutMaskViewConverter::KisGamutMaskViewConverter()
    : m_viewSize(1.0)
    , m_maskSize(QSizeF(1,1))
    , m_maskResolution(1)
{
    computeAndSetZoom();
}

KisGamutMaskViewConverter::~KisGamutMaskViewConverter()
{
}

QPointF KisGamutMaskViewConverter::documentToView(const QPointF &documentPoint) const
{
    return QPointF(documentToViewX(documentPoint.x()), documentToViewY(documentPoint.y()));
}


QPointF KisGamutMaskViewConverter::viewToDocument(const QPointF &viewPoint) const
{
    return QPointF(viewToDocumentX(viewPoint.x()), viewToDocumentY(viewPoint.y()));
}

QRectF KisGamutMaskViewConverter::documentToView(const QRectF &documentRect) const
{
    return QRectF(documentToView(documentRect.topLeft()), documentToView(documentRect.size()));
}

QRectF KisGamutMaskViewConverter::viewToDocument(const QRectF &viewRect) const
{
    return QRectF(viewToDocument(viewRect.topLeft()), viewToDocument(viewRect.size()));
}

QSizeF KisGamutMaskViewConverter::documentToView(const QSizeF &documentSize) const
{
    return QSizeF(documentToViewX(documentSize.width()), documentToViewY(documentSize.height()));
}

QSizeF KisGamutMaskViewConverter::viewToDocument(const QSizeF &viewSize) const
{
    return QSizeF(viewToDocumentX(viewSize.width()), viewToDocumentY(viewSize.height()));
}

qreal KisGamutMaskViewConverter::documentToViewX(qreal documentX) const
{
       qreal translated = documentX * m_zoomLevel;

#ifdef DEBUG_GAMUT_MASK_CONVERTER
    debugFlake << "KisGamutMaskViewConverter::DocumentToViewX: "
               << "documentX: " << documentX
               << " -> translated: " << translated;
#endif

    return translated;
}

qreal KisGamutMaskViewConverter::documentToViewY(qreal documentY) const
{   
    qreal translated = documentY * m_zoomLevel;

#ifdef DEBUG_GAMUT_MASK_CONVERTER
    debugFlake << "KisGamutMaskViewConverter::DocumentToViewY: "
               << "documentY: " << documentY
               << " -> translated: " << translated;
#endif

    return translated;
}

qreal KisGamutMaskViewConverter::viewToDocumentX(qreal viewX) const
{
    qreal translated = viewX / m_zoomLevel;

#ifdef DEBUG_GAMUT_MASK_CONVERTER
    debugFlake << "KisGamutMaskViewConverter::viewToDocumentX: "
               << "viewX: " << viewX
               << " -> translated: " << translated;
#endif

    return translated;
}

qreal KisGamutMaskViewConverter::viewToDocumentY(qreal viewY) const
{
    qreal translated = viewY / m_zoomLevel;

#ifdef DEBUG_GAMUT_MASK_CONVERTER
       debugFlake << "KisGamutMaskViewConverter::viewToDocumentY: "
               << "viewY: " << viewY
               << " -> translated: " << translated;
#endif

    return translated;
}


void KisGamutMaskViewConverter::setZoom(qreal zoom)
{
    if (qFuzzyCompare(zoom, qreal(0.0)) || qFuzzyCompare(zoom, qreal(1.0))) {
        zoom = 1;
    }

#ifdef DEBUG_GAMUT_MASK_CONVERTER
    debugFlake << "KisGamutMaskViewConverter::setZoom: setting to " << zoom;
#endif

    m_zoomLevel = zoom;
}

void KisGamutMaskViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    *zoomX = m_zoomLevel;
    *zoomY = m_zoomLevel;
}

void KisGamutMaskViewConverter::setViewSize(QSize viewSize)
{
    m_viewSize = viewSize.width();

    computeAndSetZoom();
}

void KisGamutMaskViewConverter::setMaskSize(QSizeF maskSize)
{
    m_maskSize = maskSize;
    m_maskResolution = maskSize.width();

    computeAndSetZoom();
}

void KisGamutMaskViewConverter::computeAndSetZoom()
{
    qreal zoom = m_viewSize / m_maskResolution;

#ifdef DEBUG_GAMUT_MASK_CONVERTER
    debugFlake << "KisGamutMaskViewConverter::computeAndSetZoom: "
               << "m_viewSize: " << m_viewSize
               << " m_maskSize: " << m_maskResolution;
#endif

    setZoom(zoom);
}
