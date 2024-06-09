class Canvas : QObject
{
%TypeHeaderCode
#include "Canvas.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Canvas passed to a Signal is provided as a QObject instead of
  // a Canvas object
  // (not sure why, but explicit conversion here fix the problem)
  Canvas* mc = qobject_cast<Canvas*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Canvas;
  }
%End

    Canvas(const Canvas & __0);
public:    
    bool operator==(const Canvas &other) const;
    bool operator!=(const Canvas &other) const;
public Q_SLOTS:
    qreal zoomLevel() const;
    void setZoomLevel(qreal value);
    void resetZoom();
    void resetRotation();
    qreal rotation() const;
    void setRotation(qreal angle);
    bool mirror() const;
    void setMirror(bool value);
    void setWrapAroundMode(bool enable);
    bool wrapAroundMode() const;
    void setLevelOfDetailMode(bool enable);
    bool levelOfDetailMode() const;
    View *view() const /Factory/;
Q_SIGNALS:
private:
};
