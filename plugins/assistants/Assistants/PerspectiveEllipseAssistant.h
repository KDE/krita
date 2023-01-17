/*
 * SPDX-FileCopyrightText: 2022 Srirupa Datta <srirupa.sps@gmail.com>
 */

#ifndef _PERSPECTIVE_ELLIPSE_ASSISTANT_H_
#define _PERSPECTIVE_ELLIPSE_ASSISTANT_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include "Ellipse.h"
#include <QObject>

class PerspectiveEllipseAssistant : public KisAbstractPerspectiveGrid, public KisPaintingAssistant
{
    Q_OBJECT
public:
    PerspectiveEllipseAssistant(QObject * parent = 0);
    ~PerspectiveEllipseAssistant();


    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny, qreal moveThresholdPt) override;
    void adjustLine(QPointF &point, QPointF& strokeBegin) override;
    
    QPointF getDefaultEditorPosition() const override;
    int numHandles() const override { return 4; }
    bool isAssistantComplete() const override;


    void saveCustomXml(QXmlStreamWriter* xml) override;
    bool loadCustomXml(QXmlStreamReader* xml) override;

    void endStroke() override;
    void setAdjustedBrushPosition(const QPointF position) override;


    bool isConcentric() const;
    void setConcentric(bool isConcentric);

    // implements KisAbstractPerspectiveGrid
    bool contains(const QPointF& point) const override;
    qreal distance(const QPointF& point) const override;
    bool isActive() const  override;

    
protected:
    QRect boundingRect() const override;
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);

    // finds the transform from perspective coordinates (a unit square) to the document
    bool getTransform(QPolygonF& polyOut, QTransform& transformOut);


    bool isEllipseValid();
    void updateCache();


     
    explicit PerspectiveEllipseAssistant(const PerspectiveEllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);


    class Private;
    QScopedPointer<Private> d;
    
};

class PerspectiveEllipseAssistantFactory : public KisPaintingAssistantFactory
{
public:
    PerspectiveEllipseAssistantFactory();
    ~PerspectiveEllipseAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
