#include "Tool.h"

#include <QMouseEvent>
#include <QPainter>

#include "rtreetestapp.h"

void Tool::mouseMoveEvent(QMouseEvent *e)
{
    if ( m_buttonPressed == true )
    {
        m_rect.setBottomRight( e->pos() );
        m_canvas->updateCanvas();
    }
}

void Tool::mousePressEvent(QMouseEvent *e)
{
    m_buttonPressed = true;
    m_rect.setTopLeft( e->pos() );
    m_rect.setSize( QSize( 0, 0 ) );
}

CreateTool::CreateTool( Canvas * canvas )
: Tool( canvas )
{
}

CreateTool::~CreateTool()
{
}

void CreateTool::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED( e );
    if ( m_buttonPressed == true )
    {
        m_buttonPressed = false;
        m_canvas->insert( m_rect );
    }
}

void CreateTool::paint( QPainter & p )
{
    if ( m_buttonPressed == true )
    {
        p.save();
        QPen pen( Qt::blue );
        p.setPen( pen );
        p.drawRect( m_rect.normalized() );
        p.restore();
    }
}


SelectTool::SelectTool( Canvas * canvas )
: Tool( canvas )
{
}

SelectTool::~SelectTool()
{
}

void SelectTool::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED( e );
    if ( m_buttonPressed == true )
    {
        m_buttonPressed = false;
        m_canvas->select( m_rect );
    }
}

void SelectTool::paint( QPainter & p )
{
    if ( m_buttonPressed == true )
    {
        p.save();
        QPen pen( Qt::blue );
        pen.setStyle( Qt::DashLine );
        p.setPen( pen );
        p.drawRect( m_rect.normalized() );
        p.restore();
    }
}


RemoveTool::RemoveTool( Canvas * canvas )
: Tool( canvas )
{
}

RemoveTool::~RemoveTool()
{
}

void RemoveTool::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED( e );
    if ( m_buttonPressed == true )
    {
        m_buttonPressed = false;
        m_canvas->remove( m_rect );
    }
}

void RemoveTool::paint( QPainter & p )
{
    if ( m_buttonPressed == true )
    {
        p.save();
        QPen pen( Qt::red );
        pen.setStyle( Qt::DashLine );
        p.setPen( pen );
        p.drawRect( m_rect.normalized() );
        p.restore();
    }
}

