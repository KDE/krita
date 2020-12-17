/*
 * SPDX-FileCopyrightText: 2006, 2008-2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoViewConverter.h"

#include <QPointF>
#include <QRectF>
#include <QTransform>

KoViewConverter::KoViewConverter()
    : m_zoomLevel(1.0)
{
}

QPointF KoViewConverter::documentToView(const QPointF &documentPoint) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentPoint;
    return QPointF(documentToViewX(documentPoint.x()), documentToViewY(documentPoint.y()));
}

QPointF KoViewConverter::viewToDocument(const QPointF &viewPoint) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewPoint;
    return QPointF(viewToDocumentX(viewPoint.x()), viewToDocumentY(viewPoint.y()));
}

QRectF KoViewConverter::documentToView(const QRectF &documentRect) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentRect;
    return QRectF(documentToView(documentRect.topLeft()), documentToView(documentRect.size()));
}

QRectF KoViewConverter::viewToDocument(const QRectF &viewRect) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewRect;
    return QRectF(viewToDocument(viewRect.topLeft()), viewToDocument(viewRect.size()));
}

QSizeF KoViewConverter::documentToView(const QSizeF &documentSize) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentSize;
    return QSizeF(documentToViewX(documentSize.width()), documentToViewY(documentSize.height()));
}

QSizeF KoViewConverter::viewToDocument(const QSizeF &viewSize) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewSize;
    return QSizeF(viewToDocumentX(viewSize.width()), viewToDocumentY(viewSize.height()));
}

void KoViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    *zoomX = m_zoomLevel;
    *zoomY = m_zoomLevel;
}

qreal KoViewConverter::documentToViewX(qreal documentX) const
{
    return documentX * m_zoomLevel;
}

qreal KoViewConverter::documentToViewY(qreal documentY) const
{
    return documentY * m_zoomLevel;
}

qreal KoViewConverter::viewToDocumentX(qreal viewX) const
{
    return viewX / m_zoomLevel;
}

qreal KoViewConverter::viewToDocumentY(qreal viewY) const
{
    return viewY / m_zoomLevel;
}



void KoViewConverter::setZoom(qreal zoom)
{
    if (qFuzzyCompare(zoom, qreal(0.0)) || qFuzzyCompare(zoom, qreal(1.0))) {
        zoom = 1;
    }
    m_zoomLevel = zoom;
}

qreal KoViewConverter::zoom() const
{
    return m_zoomLevel;
}

QTransform KoViewConverter::documentToView() const
{
    qreal zoomX, zoomY;
    zoom(&zoomX, &zoomY);
    return QTransform::fromScale(zoomX, zoomY);
}

QTransform KoViewConverter::viewToDocument() const
{
    qreal zoomX, zoomY;
    zoom(&zoomX, &zoomY);
    return QTransform::fromScale(1.0 / zoomX, 1.0 / zoomY);
}

QTransform KoViewConverter::viewToWidget() const
{
    return QTransform();
}

QTransform KoViewConverter::widgetToView() const
{
    return QTransform();
}
