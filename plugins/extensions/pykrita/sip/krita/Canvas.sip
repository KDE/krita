class Canvas : QObject
{
%TypeHeaderCode
#include "Canvas.h"
%End
    Canvas(const Canvas & __0);
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
