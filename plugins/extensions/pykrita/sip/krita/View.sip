class View : QObject
{
%TypeHeaderCode
#include "View.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a View passed to a Signal is provided as a QObject instead of
  // a View object
  // (not sure why, but explicit conversion here fix the problem)
  View* mc = qobject_cast<View*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_View;
  }
%End

    View(const View & __0);
public:
    bool operator==(const View &other) const;
    bool operator!=(const View &other) const;

public Q_SLOTS:
    Window * window() const /Factory/;
    Document * document() const /Factory/;
    void setDocument(Document *document);
    bool visible() const;
    void setVisible();
    Canvas * canvas() const /Factory/;
    void activateResource(Resource *resource);

    ManagedColor *foregroundColor() const /Factory/;
    void setForeGroundColor(ManagedColor *color);

    ManagedColor *backgroundColor() const /Factory/;
    void setBackGroundColor(ManagedColor *color);

    Resource *currentBrushPreset() const /Factory/;
    void setCurrentBrushPreset(Resource *resource);

    Resource *currentPattern() const /Factory/;
    void setCurrentPattern(Resource *resource);

    Resource *currentGradient() const /Factory/;
    void setCurrentGradient(Resource *resource);

    QString currentBlendingMode() const;
    void setCurrentBlendingMode(const QString &blendingMode);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    float HDRGamma() const;
    void setHDRGamma(float gamma);

    qreal paintingOpacity() const;
    void setPaintingOpacity(qreal opacity);

    qreal brushSize() const;
    void setBrushSize(qreal brushSize);

    qreal brushFade() const;
    void setBrushFade(qreal brushFade);

    qreal brushRotation() const;
    void setBrushRotation(qreal brushRotation);

    qreal paintingFlow() const;
    void setPaintingFlow(qreal flow);

    qreal patternSize() const;
    void setPatternSize(qreal size);

    bool eraserMode() const;
    void setEraserMode(bool value);

    bool globalAlphaLock() const;
    void setGlobalAlphaLock(bool value);

    bool disablePressure() const;
    void setDisablePressure(bool value);

    void showFloatingMessage(const QString &message, const QIcon& icon, int timeout, int priority);

    QList<Node *> selectedNodes() const /Factory/;

    QTransform flakeToDocumentTransform() const;
    QTransform flakeToCanvasTransform() const;
    QTransform flakeToImageTransform() const;
private:
};
