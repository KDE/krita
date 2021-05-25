/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _VANISHINGPOINT_ASSISTANT_H_
#define _VANISHINGPOINT_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
/* Design:
 *The idea behind the vanishing point ruler is that in a perspective deformed landscape, a set of parallel
 *lines al share a single vanishing point.
 *Therefore, a perspective can contain an theoretical infinite of vanishing points.
 *It's a pity if we only allowed an artist to access 1, 2 or 3 of these at any given time, as other
 *solutions for perspective tools do.
 *Hence a vanishing point ruler.
 *
 *This ruler is relatively simple compared to the other perspective ruler:
 *It has only one vanishing point that is required to draw.
 *However, it does have it's own weaknesses in how to determine onto which of these infinite rulers to snap.
 *Furthermore, it has four extra handles for adding a perspective ruler to a preexisting perspective.
 */
//class VanishingPoint;

class VanishingPointAssistant : public KisPaintingAssistant
{
public:
    VanishingPointAssistant();

    enum VanishingPointAssistantHandle {
        VanishingPointHandle,
        LocalFirstHandle,
        LocalSecondHandle
    };

    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, bool snapToAny) override;
    void setAdjustedBrushPosition(const QPointF position) override;
    void setFollowBrushPosition(bool follow) override;
    void endStroke() override;
    QPointF getEditorPosition() const override;
    int numHandles() const override { return isLocal() ? 3 : 1; }

    float referenceLineDensity();
    void setReferenceLineDensity(float value);

    bool isAssistantComplete() const override;
    bool canBeLocal() const override;

    void saveCustomXml(QXmlStreamWriter* xml) override;
    bool loadCustomXml(QXmlStreamReader* xml) override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;

    KisPaintingAssistantHandleSP firstLocalHandle() const override;
    KisPaintingAssistantHandleSP secondLocalHandle() const override;

private:


    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    explicit VanishingPointAssistant(const VanishingPointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

    KisCanvas2 *m_canvas;

    float m_referenceLineDensity = 15.0;

    // Needed to make sure that when we are in the middle of a brush stroke, the
    // guides follow the brush position, not the cursor position.
    bool m_followBrushPosition;
    bool m_adjustedPositionValid;
    QPointF m_adjustedBrushPosition;
    bool m_hasBeenInsideLocalRect;

};

class VanishingPointAssistantFactory : public KisPaintingAssistantFactory
{
public:
    VanishingPointAssistantFactory();
    ~VanishingPointAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
