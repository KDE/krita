class Canvas : QObject
{
%TypeHeaderCode
#include "Canvas.h"
%End
    Canvas(const Canvas & __0);
public:
    Document * document() const;
    int zoomLevel() const;
    void setZoomLevel(int value);
    int rotation() const;
    void setRotation(int value);
    bool mirror() const;
    void setMirror(bool value);
    ColorManager * colorManager() const;
    void setColorManager(ColorManager*  value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
