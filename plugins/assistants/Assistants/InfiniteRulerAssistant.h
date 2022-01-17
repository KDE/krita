/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 * SPDX-FileCopyrightText: 2022 Julian Schmidt <julisch1107@web.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _INFINITERULER_ASSISTANT_H_
#define _INFINITERULER_ASSISTANT_H_

#include "RulerAssistant.h"

#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>



class InfiniteRulerAssistant : public RulerAssistant
{
public:
    InfiniteRulerAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny) override;
    QPointF getEditorPosition() const override;
    int numHandles() const override { return 2; }
    bool isAssistantComplete() const override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    explicit InfiniteRulerAssistant(const InfiniteRulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
  
    void drawSubdivisions(QPainter& gc, const KisCoordinatesConverter *converter);
    
    // Helper struct for clipLineParametric's return type
    struct ClippingResult {
        bool intersects;
        qreal tmin;
        qreal tmax;
    };
    // Like KisAlgebra2D::clipLineRect, but returns the parametric positions
    static ClippingResult clipLineParametric(QLineF line, QRectF rect, bool extendFirst=true, bool extendSecond=true);
};

class InfiniteRulerAssistantFactory : public KisPaintingAssistantFactory
{
public:
    InfiniteRulerAssistantFactory();
    ~InfiniteRulerAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
