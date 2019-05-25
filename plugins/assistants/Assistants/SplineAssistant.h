/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _SPLINE_ASSISTANT_H_
#define _SPLINE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>

class SplineAssistant : public KisPaintingAssistant
{
public:
    SplineAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    QPointF buttonPosition() const override;
    int numHandles() const override { return 4; }
    bool isAssistantComplete() const override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt) const;
    explicit SplineAssistant(const SplineAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

    /// used for getting the decoration so the bezier handles aren't drawn while editing
    KisCanvas2* m_canvas;
};

class SplineAssistantFactory : public KisPaintingAssistantFactory
{
public:
    SplineAssistantFactory();
    ~SplineAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
