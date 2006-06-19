#ifndef TOOL_H
#define TOOL_H

#include <QRectF>

class QMouseEvent;
class QPainter;

class Canvas;

class Tool
{
public:    
    Tool( Canvas * canvas )
    : m_canvas ( canvas )
    , m_buttonPressed( false )
    {}
    virtual ~Tool() {}

    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e) = 0;

    virtual void paint( QPainter & p ) = 0;

protected:
    Canvas * m_canvas;
    bool m_buttonPressed;
    QRectF m_rect;
};

class CreateTool : public Tool
{
public:    
    CreateTool( Canvas * canvas );
    virtual ~CreateTool();

    virtual void mouseReleaseEvent(QMouseEvent *e);

    virtual void paint( QPainter & p );

protected:
};

class SelectTool : public Tool
{
public:    
    SelectTool( Canvas * canvas );
    virtual ~SelectTool();

    virtual void mouseReleaseEvent(QMouseEvent *e);

    virtual void paint( QPainter & p );

protected:
};

class RemoveTool : public Tool
{
public:    
    RemoveTool( Canvas * canvas );
    virtual ~RemoveTool();

    virtual void mouseReleaseEvent(QMouseEvent *e);

    virtual void paint( QPainter & p );

protected:
};

#endif /* TOOL_H */
