#ifndef _CONJUGATE_ASSISTANT_H_
#define _CONJUGATE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

class ConjugateAssistant : public KisPaintingAssistant
{
public:
    ConjugateAssistant();
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;
    //virtual void endStroke();
    QPointF getEditorPosition() const override;
    int numHandles() const override { return 3; }

    float referenceLineDensity();
    void setReferenceLineDensity(float value);

    bool isAssistantComplete() const override;

    void saveCustomXml(QXmlStreamWriter* xml) override;
    bool loadCustomXml(QXmlStreamReader* xml) override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    explicit ConjugateAssistant(const ConjugateAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
    KisCanvas2 *m_canvas;

    float m_referenceLineDensity = 15.0;
  QLineF m_snapLine;


};

class ConjugateAssistantFactory : public KisPaintingAssistantFactory
{
public:
    ConjugateAssistantFactory();
    ~ConjugateAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
