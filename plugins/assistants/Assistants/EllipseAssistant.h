/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ELLIPSE_ASSISTANT_H_
#define _ELLIPSE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include "Ellipse.h"
#include <QObject>

class EllipseAssistant : public KisPaintingAssistant
{
public:
    EllipseAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny) override;
    QPointF getEditorPosition() const override;
    int numHandles() const override { return 3; }
    bool isAssistantComplete() const override;

    void transform(const QTransform &transform) override;

protected:
    QRect boundingRect() const override;
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt) const;
    explicit EllipseAssistant(const EllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
    mutable Ellipse e;
};

class EllipseAssistantFactory : public KisPaintingAssistantFactory
{
public:
    EllipseAssistantFactory();
    ~EllipseAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
