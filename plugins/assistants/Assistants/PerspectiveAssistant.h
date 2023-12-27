/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PERSPECTIVE_ASSISTANT_H_
#define _PERSPECTIVE_ASSISTANT_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

#include <PerspectiveBasedAssistantHelper.h>

class PerspectiveAssistant : public KisAbstractPerspectiveGrid, public KisPaintingAssistant
{
    Q_OBJECT
public:
    PerspectiveAssistant(QObject * parent = 0);
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny, qreal moveThresholdPt) override;
    void adjustLine(QPointF &point, QPointF& strokeBegin) override;
    void endStroke() override;

    QPointF getDefaultEditorPosition() const override;
    int numHandles() const override { return 4; }
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;

    bool contains(const QPointF& point) const override;
    qreal distance(const QPointF& point) const override;
    bool isActive() const override;

    int subdivisions() const;
    void setSubdivisions(int subdivisions);

    bool isAssistantComplete() const override;

    void saveCustomXml(QXmlStreamWriter *xml) override;
    bool loadCustomXml(QXmlStreamReader *xml) override;

protected:
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin, const bool alwaysStartAnew, qreal moveThresholdPt);
    // creates the convex hull, returns false if it's not a quadrilateral
    // finds the transform from perspective coordinates (a unit square) to the document
    bool getTransform(QPolygonF& polyOut, QTransform& transformOut) const;
    explicit PerspectiveAssistant(const PerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

    // The number of subdivisions to draw
    int m_subdivisions {8};
    // which direction to snap to (in transformed coordinates)
    QLineF m_snapLine;
    // cached information
    mutable QTransform m_cachedTransform;
    mutable QPolygonF m_cachedPolygon;
    mutable QPointF m_cachedPoints[4];
    mutable bool m_cacheValid {false};

    mutable PerspectiveBasedAssistantHelper::CacheData m_cache;

};

class PerspectiveAssistantFactory : public KisPaintingAssistantFactory
{
public:
    PerspectiveAssistantFactory();
    ~PerspectiveAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
