/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2021 Nabil Maghfur Usman <nmaghfurusman@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _TWO_POINT_ASSISTANT_H_
#define _TWO_POINT_ASSISTANT_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include <QObject>
#include <QLineF>
#include <QTransform>

class TwoPointAssistant : public KisAbstractPerspectiveGrid, public KisPaintingAssistant
{
    Q_OBJECT
public:

    enum TwoPointHandle {
        FirstHandle,
        SecondHandle,
        VerticalHandle,
        LocalFirstHandle,
        LocalSecondHandle
    };


    TwoPointAssistant();
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny, qreal moveThresholdPt) override;
    void adjustLine(QPointF &point, QPointF& strokeBegin) override;
    void endStroke() override;
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    QPointF getDefaultEditorPosition() const override;
    int numHandles() const override { return isLocal() ? 5 : 3; }

    void saveCustomXml(QXmlStreamWriter* xml) override;
    bool loadCustomXml(QXmlStreamReader* xml) override;

    double gridDensity();
    void setGridDensity(double density);

    /* If true, it means the assistant will have three handles
     * If false,
     * */
    bool useVertical();
    void setUseVertical(bool value);

    bool isAssistantComplete() const override;
    bool canBeLocal() const override;

    bool contains(const QPointF& point) const override;
    qreal distance(const QPointF& point) const override;
    bool isActive() const override;

    /* Generate a transform for converting handles into easier local
       coordinate system that has the following properties:
       - Rotated so horizon is perfectly horizontal
       - Translated so the origin is the projection of the 3rd handle
         onto the horizon; the handle lies on the local y axis
       Parameters are the first VP, second VP, a 3rd point which
       defines the center of vision, and lastly a reference to a size
       variable which is the radius of the 90 degree cone of vision.
       The transform only rotates and translates, so size is also an
       image-space length. The horizon is at local y = 0.
       (useful for computing snapping behaviour and drawing grid
       lines) */
    QTransform localTransform(QPointF vp_a, QPointF vp_b, QPointF pt_c, qreal* size) const;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, const KoColorDisplayRendererInterface *displayRenderInterface, bool  cached = true,KisCanvas2* canvas=nullptr, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter, const KoColorDisplayRendererInterface *displayRenderInterface,  bool assistantVisible=true) override;

    KisPaintingAssistantHandleSP firstLocalHandle() const override;
    KisPaintingAssistantHandleSP secondLocalHandle() const override;


private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin, const bool snapToAny, qreal moveThreshold);
    bool localPerspectiveData(const QPointF &point, QPointF *localPoint, qreal *size) const;
    explicit TwoPointAssistant(const TwoPointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
    KisCanvas2 *m_canvas {nullptr};

    QLineF m_snapLine;
    double m_gridDensity {1.0};
    bool m_useVertical {true};

    int m_lastUsedPoint {-1}; // last used vanishing point

};

class TwoPointAssistantFactory : public KisPaintingAssistantFactory
{
public:
    TwoPointAssistantFactory();
    ~TwoPointAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
