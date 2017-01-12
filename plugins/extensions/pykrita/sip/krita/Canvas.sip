class Canvas : QObject
{
%TypeHeaderCode
#include "Canvas.h"
%End
    Canvas(const Canvas & __0);
public Q_SLOTS:
    Document * document() const;
    int zoomLevel() const;
    void setZoomLevel(int value);
    int rotation() const;
    void setRotation(int value);
    bool mirror() const;
    void setMirror(bool value);
Q_SIGNALS:
private:
};
