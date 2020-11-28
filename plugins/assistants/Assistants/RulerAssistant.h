/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _RULER_ASSISTANT_H_
#define _RULER_ASSISTANT_H_

#include <QMap>

#include "kis_painting_assistant.h"

class Ruler;

class RulerAssistant : public KisPaintingAssistant
{
public:
    RulerAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    QPointF getEditorPosition() const override;
    int numHandles() const override { return 2; }
    bool isAssistantComplete() const override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt) const;
    explicit RulerAssistant(const RulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
};

class RulerAssistantFactory : public KisPaintingAssistantFactory
{
public:
    RulerAssistantFactory();
    ~RulerAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
