#ifndef _TWO_POINT_ASSISTANT_H_
#define _TWO_POINT_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

class TwoPointAssistant : public KisPaintingAssistant
{
public:
    TwoPointAssistant();
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    void setAdjustedBrushPosition(const QPointF position) override;
    void setFollowBrushPosition(bool follow) override;
    void endStroke() override;

    QPointF getEditorPosition() const override;
    int numHandles() const override { return 3; }

    void saveCustomXml(QXmlStreamWriter* xml) override;
    bool loadCustomXml(QXmlStreamReader* xml) override;

    double gridDensity();
    void setGridDensity(double density);

    bool isAssistantComplete() const override;

    /* Generate a transform for converting handles into easier local
       coordinate system that has the following properties:
       - Rotated so horizon is perfectly horizonal
       - Translated so 3rd handle is the origin
       Paremeters are the first VP, second VP, a 3rd point which
       defines the center of vision, and lastly a reference to a size
       variable which is the radius of the 90 degree cone of vision
       (useful for computing snapping behaviour and drawing grid
       lines) */
    QTransform localTransform(QPointF vp_a, QPointF vp_b, QPointF pt_c, qreal* size);

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    explicit TwoPointAssistant(const TwoPointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
    KisCanvas2 *m_canvas;

    QLineF m_snapLine;
    double m_gridDensity = 1.0;

    // Needed to make sure that when we are in the middle of a brush stroke, the
    // guides follow the brush position, not the cursor position.
    bool m_followBrushPosition;
    bool m_adjustedPositionValid;
    QPointF m_adjustedBrushPosition;

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
