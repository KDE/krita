/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 * SPDX-FileCopyrightText: 2022 Julian Schmidt <julisch1107@web.de>
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
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny) override;
    QPointF getEditorPosition() const override;
    int numHandles() const override { return 2; }
    bool isAssistantComplete() const override;
    void saveCustomXml(QXmlStreamWriter *xml) override;
    bool loadCustomXml(QXmlStreamReader *xml) override;
    
    int subdivisions() const;
    void setSubdivisions(int subdivisions);
    int minorSubdivisions() const;
    void setMinorSubdivisions(int subdivisions);
    bool hasFixedLength() const;
    void enableFixedLength(bool enabled);
    qreal fixedLength() const;
    void setFixedLength(qreal length);
    QString fixedLengthUnit() const;
    void setFixedLengthUnit(QString unit);
    
    void ensureLength();

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
    explicit RulerAssistant(const QString& id, const QString& name);
    explicit RulerAssistant(const RulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

  private:
    QPointF project(const QPointF& pt) const;
    void drawSubdivisions(QPainter& gc, const KisCoordinatesConverter *converter);
    void drawHandleAnnotations(QPainter& gc, const KisCoordinatesConverter *converter);
    
    int m_subdivisions {0};
    int m_minorSubdivisions {0};
    bool m_hasFixedLength {false};
    qreal m_fixedLength {0.0};
    QString m_fixedLengthUnit {"px"};
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
