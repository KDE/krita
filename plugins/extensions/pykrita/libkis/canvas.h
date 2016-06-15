#ifndef CANVAS_H
#define CANVAS_H

#include <KoCanvasBase.h>

class KoCanvasBase;

class Canvas
{
public:
    explicit Canvas(KoCanvasBase * canvas);

private:
    KoCanvasBase *m_canvas;
};

#endif // CANVAS_H
