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
    enum ZoomMode
    {
        ZOOM_CONSTANT = 0,
        ZOOM_PAGE     = 1,
        ZOOM_WIDTH    = 2,
        ZOOM_HEIGHT   = 16,
    };
    bool operator==(const Canvas &other) const;
    bool operator!=(const Canvas &other) const;
public Q_SLOTS:
    qreal zoomLevel() const;
    void setZoomLevel(qreal value);
    QPointF preferredCenter() const;
    void setPreferredCenter(const QPointF& imagePos);
    void pan(const QPoint& offset);
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
    void removed();
    void offsetChanged();
    void cursorPositionChanged(const QPoint &position);
    void documentCursorPositionChanged(const QPointF &position);
    void sizeChanged(const QSize &size);
    void documentOffsetChanged(const QPointF &oldOffset, const QPointF &newOffset);
    void effectiveZoomChanged(qreal effectiveZoom);
    void zoomStateChanged(const ZoomMode zoomMode, qreal zoom, qreal minZoom, qreal maxZoom);
    void documentRectInWidgetPixelsChanged(const QRectF &documentRectInWidgetPixels);
    void rotationChanged(qreal angle);
    void mirrorChanged(bool mirrorX, bool mirrorY);
    void stateChanged();
private:
};
