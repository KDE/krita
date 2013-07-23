/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>
   Copyright 2011 Sven Langkamp <sven.langkamp@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kis_popup_palette.h"
#include "kis_paintop_box.h"
#include "ko_favorite_resource_manager.h"
#include "KoColorSpaceRegistry.h"
#include "KoResource.h"
#include <kis_types.h>
#include <QtGui>
#include <QDebug>
#include <QQueue>
#include <math.h>

#define maxPresetCount 10

class PopupColorTriangle : public KoTriangleColorSelector
{
public:
    PopupColorTriangle(QWidget* parent) : KoTriangleColorSelector(parent), m_dragging(false) {}
    virtual ~PopupColorTriangle() {}

    void tabletEvent(QTabletEvent* event)
    {
        event->accept();
        QMouseEvent* mouseEvent = 0;
        switch(event->type())
        {
        case QEvent::TabletPress:
            mouseEvent = new QMouseEvent(QEvent::MouseButtonPress, event->pos(),
                                         Qt::LeftButton, Qt::LeftButton, event->modifiers());
            m_dragging = true;
            mousePressEvent(mouseEvent);
            break;
        case QEvent::TabletMove:
            mouseEvent = new QMouseEvent(QEvent::MouseMove, event->pos(),
                                         (m_dragging) ? Qt::LeftButton : Qt::NoButton,
                                         (m_dragging) ? Qt::LeftButton : Qt::NoButton, event->modifiers());
            mouseMoveEvent(mouseEvent);
            break;
        case QEvent::TabletRelease:
            mouseEvent = new QMouseEvent(QEvent::MouseButtonRelease, event->pos(),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         event->modifiers());
            m_dragging = false;
            mouseReleaseEvent(mouseEvent);
            break;
        default: break;
        }
        delete mouseEvent;
    }

private:
    bool m_dragging;
};

KisPopupPalette::KisPopupPalette(KoFavoriteResourceManager* manager, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , m_resourceManager (manager)
    , m_triangleColorSelector (0)
    , m_timer(0)
{
    m_triangleColorSelector  = new PopupColorTriangle(this);
    m_triangleColorSelector->move(90, 90);
    m_triangleColorSelector->setVisible(true);

    setAutoFillBackground(true);
    setAttribute(Qt::WA_ContentsPropagated,true);
    //    setAttribute(Qt::WA_TranslucentBackground, true);

    connect(m_triangleColorSelector, SIGNAL(colorChanged(QColor)), SLOT(slotChangefGColor(QColor)));
    connect(this, SIGNAL(sigChangeActivePaintop(int)), m_resourceManager, SLOT(slotChangeActivePaintop(int)));
    connect(this, SIGNAL(sigUpdateRecentColor(int)), m_resourceManager, SLOT(slotUpdateRecentColor(int)));
    connect(this, SIGNAL(sigChangefGColor(KoColor)), m_resourceManager, SIGNAL(sigSetFGColor(KoColor)));
    connect(m_resourceManager, SIGNAL(sigChangeFGColorSelector(QColor)), m_triangleColorSelector, SLOT(setQColor(QColor)));

    // This is used to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(this, SIGNAL(sigTriggerTimer()), this, SLOT(slotTriggerTimer()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotEnableChangeFGColor()));
    connect(this, SIGNAL(sigEnableChangeFGColor(bool)), m_resourceManager, SIGNAL(sigEnableChangeColor(bool)));

    m_colorChangeTimer = new QTimer(this);
    m_colorChangeTimer->setInterval(50);
    m_colorChangeTimer->setSingleShot(true);
    connect(m_colorChangeTimer, SIGNAL(timeout()), this, SLOT(slotColorChangeTimeout()));

    setMouseTracking(true);
    setHoveredPreset(-1);
    setHoveredColor(-1);
    setSelectedColor(-1);

    setVisible(true);
    setVisible(false);
}

//setting KisPopupPalette properties
int KisPopupPalette::hoveredPreset() const { return m_hoveredPreset; }
void KisPopupPalette::setHoveredPreset(int x) { m_hoveredPreset = x; }
int KisPopupPalette::hoveredColor() const { return m_hoveredColor; }
void KisPopupPalette::setHoveredColor(int x) { m_hoveredColor = x; }
int KisPopupPalette::selectedColor() const { return m_selectedColor; }
void KisPopupPalette::setSelectedColor(int x) { m_selectedColor = x; }

void KisPopupPalette::slotChangefGColor(const QColor& /*newColor*/)
{
    m_colorChangeTimer->start();
}

void KisPopupPalette::slotColorChangeTimeout()
{
    KoColor color ( m_triangleColorSelector->color(), KoColorSpaceRegistry::instance()->rgb16(0));
    emit sigChangefGColor(color);
}

void KisPopupPalette::slotTriggerTimer()
{
    m_timer->start(750);
}

void KisPopupPalette::slotEnableChangeFGColor()
{
    emit sigEnableChangeFGColor(true);
}

void KisPopupPalette::showPopupPalette (const QPoint &p)
{
    if (!isVisible())
    {
        QSize parentSize(parentWidget()->size());
        QPoint pointPalette(p.x() - width()/2, p.y() - height()/2);

        //setting offset point in case the widget is shown outside of canvas region
        int offsetX = 0, offsetY=0;

        if ((offsetX = pointPalette.x() + width() - parentSize.width()) > 0 || (offsetX = pointPalette.x()) < 0)
        {
            pointPalette.setX(pointPalette.x() - offsetX);
        }
        if ((offsetY = pointPalette.y() + height() - parentSize.height()) > 0 || (offsetY = pointPalette.y()) < 0)
        {
            pointPalette.setY(pointPalette.y() - offsetY);
        }
        move(pointPalette);

    }
    showPopupPalette(!isVisible());

}

void KisPopupPalette::showPopupPalette(bool show)
{
    if (show)
    {
        emit sigEnableChangeFGColor(!show);
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    else
    {
        emit sigTriggerTimer();
        QApplication::restoreOverrideCursor();
    }

    setVisible(show);
}

//redefinition of setVariable function to change the scope to private
void KisPopupPalette::setVisible(bool b)
{
    QWidget::setVisible(b);
}

QSize KisPopupPalette::sizeHint() const
{
    return QSize(280,280);
}

void KisPopupPalette::resizeEvent(QResizeEvent*)
{
    int side = qMin(width(), height());
    QRegion maskedRegion (width()/2 - side/2, height()/2 - side/2, side, side, QRegion::Ellipse);
    setMask(maskedRegion);
}

void KisPopupPalette::paintEvent(QPaintEvent* e)
{
    float rotationAngle = 0.0;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(e->rect(), palette().brush(QPalette::Window));
    painter.translate(width()/2, height()/2);

    //painting favorite brushes
    QList<QImage> images (m_resourceManager->favoritePresetImages());

    //painting favorite brushes pixmap/icon
    QPainterPath path;
    for (int pos = 0; pos < images.size(); pos++)
    {
        painter.save();
        
        path = pathFromPresetIndex(pos);
        painter.setClipPath(path);

        QRect bounds = path.boundingRect().toAlignedRect();
        painter.drawImage(bounds.topLeft(), images.at(pos).scaled(bounds.size(), Qt::KeepAspectRatioByExpanding));
        painter.restore();        
    }
    if(hoveredPreset() > -1) {
        path = pathFromPresetIndex(hoveredPreset());
        QPen pen(palette().color(QPalette::Highlight));
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawPath(path);
                        
    }

    QColor currColor;
    //painting currently used color as color selector background
    if (selectedColor() > -1)
    {
        m_resourceManager->recentColorAt(selectedColor()).toQColor(&currColor);
        painter.setPen(Qt::NoPen);
        QPainterPath path;
        path.addEllipse(QPoint(0,0), colorInnerRadius-5, colorInnerRadius-5);
        painter.fillPath(path, currColor);
        painter.drawPath(path);
    }

    //painting recent colors : bevel (raised)
    drawArcRisen (painter, Qt::darkGray, Qt::white, 51);
    drawArcRisen (painter, Qt::white, Qt::darkGray, 34);

    //painting recent colors
    painter.setPen(Qt::NoPen);
    rotationAngle = -360.0/m_resourceManager->recentColorsTotal();

    QColor qcolor;
    KoColor kcolor;

    for (int pos = 0; pos < m_resourceManager->recentColorsTotal(); pos++)
    {
        QPainterPath path(drawDonutPathAngle(colorInnerRadius, colorOuterRadius, m_resourceManager->recentColorsTotal()));

        //accessing recent color of index pos
        kcolor = m_resourceManager->recentColorAt(pos);
        kcolor.toQColor(&qcolor);
        painter.fillPath(path, qcolor);

        painter.drawPath(path);
        painter.rotate(rotationAngle);
    }

    painter.setBrush(Qt::transparent);

    if (m_resourceManager->recentColorsTotal() == 0)
    {
        QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius, colorOuterRadius));
        painter.setPen(QPen(palette().color(QPalette::Window).darker(130), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter.drawPath(path_ColorDonut);
    }

    //painting hovered color
    if (hoveredColor() > -1)
    {
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

        if (m_resourceManager->recentColorsTotal() == 1)
        {
            QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius, colorOuterRadius));
            painter.drawPath(path_ColorDonut);
        }
        else
        {
            painter.rotate((m_resourceManager->recentColorsTotal() + hoveredColor()) *rotationAngle);
            QPainterPath path(drawDonutPathAngle(colorInnerRadius, colorOuterRadius, m_resourceManager->recentColorsTotal()));
            painter.drawPath(path);
            painter.rotate(hoveredColor() *-1 *rotationAngle);
        }
    }

    //painting selected color
    if (selectedColor() > -1)
    {
        painter.setPen(QPen(palette().color(QPalette::Highlight).darker(130), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

        if (m_resourceManager->recentColorsTotal() == 1)
        {
            QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius, colorOuterRadius));
            painter.drawPath(path_ColorDonut);
        }
        else
        {
            painter.rotate((m_resourceManager->recentColorsTotal() + selectedColor()) *rotationAngle);
            QPainterPath path(drawDonutPathAngle(colorInnerRadius,colorOuterRadius, m_resourceManager->recentColorsTotal()));
            painter.drawPath(path);
            painter.rotate(selectedColor() *-1 *rotationAngle);
        }
    }
}

void KisPopupPalette::drawArcRisen (QPainter& painter, const QColor& color1, const QColor& color2, int radius)
{
    QRadialGradient gradient;
    gradient.setColorAt(1, color1);
    gradient.setColorAt(0, color2);
    gradient.setFocalPoint(sin(M_PI/4)*radius, cos(M_PI/4)*radius);
    painter.setPen(QPen(QBrush(gradient), 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawEllipse(-1*radius+0.5, -1*radius+0.5, 2*radius+0.5, 2*radius+0.5);
}

QPainterPath KisPopupPalette::drawDonutPathFull(int x, int y, int inner_radius, int outer_radius)
{
    QPainterPath path;
    path.addEllipse(QPointF(x,y), outer_radius, outer_radius);
    path.addEllipse(QPointF(x,y), inner_radius, inner_radius);
    path.setFillRule(Qt::OddEvenFill);

    return path;
}

QPainterPath KisPopupPalette::drawDonutPathAngle(int inner_radius, int outer_radius, int limit)
{
    QPainterPath path;
    path.moveTo(-0.999 * outer_radius * sin(M_PI/limit), 0.999 * outer_radius * cos(M_PI/limit));
    path.arcTo(-1*outer_radius, -1*outer_radius, 2*outer_radius,2*outer_radius,-90.0 - 180.0/limit,
               360.0/limit);
    path.arcTo(-1*inner_radius, -1*inner_radius, 2*inner_radius,2*inner_radius,-90.0 + 180.0/limit,
               - 360.0/limit);
    path.closeSubpath();

    return path;
}

void KisPopupPalette::mouseMoveEvent (QMouseEvent* event)
{
    QPointF point = event->posF();
    event->accept();

    QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius, brushOuterRadius));
    QPainterPath pathColor(drawDonutPathFull(width()/2, height()/2, colorInnerRadius, colorOuterRadius));

    setHoveredPreset(-1);
    setHoveredColor(-1);

    if (pathBrush.contains(point))
    { //in favorite brushes area
        int pos = calculatePresetIndex(point, m_resourceManager->numFavoritePresets());

        if (pos >= 0)
        {
            setHoveredPreset(pos);
        }
    }
    else if (pathColor.contains(point))
    {
        int pos = calculateIndex(point, m_resourceManager->recentColorsTotal());

        if (pos >= 0 && pos < m_resourceManager->recentColorsTotal())
        {
            setHoveredColor(pos);
        }
    }
    update();
}

void KisPopupPalette::mousePressEvent(QMouseEvent* event)
{
    QPointF point = event->posF();
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius, brushOuterRadius));

        if (pathBrush.contains(point))
        { //in favorite brushes area
            int pos = calculateIndex(point, m_resourceManager->numFavoritePresets());
            if (pos >= 0 && pos < m_resourceManager->numFavoritePresets()
                    && isPointInPixmap(point, pos))
            {
                //setSelectedBrush(pos);
                update();
            }
        }
    }
}

void KisPopupPalette::mouseReleaseEvent ( QMouseEvent * event )
{
    QPointF point = event->posF();
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius, brushOuterRadius));
        QPainterPath pathColor(drawDonutPathFull(width()/2, height()/2, colorInnerRadius, colorOuterRadius));

        if (pathBrush.contains(point))
        { //in favorite brushes area
            if (hoveredPreset() > -1)
            {
                //setSelectedBrush(hoveredBrush());
                emit sigChangeActivePaintop(hoveredPreset());
            }
        }
        else if (pathColor.contains(point))
        {
            int pos = calculateIndex(point, m_resourceManager->recentColorsTotal());

            if (pos >= 0 && pos < m_resourceManager->recentColorsTotal())
            {
                emit sigUpdateRecentColor(pos);
            }
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        showPopupPalette(false);
    }
}

void KisPopupPalette::tabletEvent(QTabletEvent* event)
{
    event->accept();
    QMouseEvent* mouseEvent = 0;
    switch(event->type())
    {
    case QEvent::TabletPress:

        mouseEvent = new QMouseEvent(QEvent::MouseButtonPress, event->pos(),
                                     Qt::LeftButton, Qt::LeftButton, event->modifiers());
        mousePressEvent(mouseEvent);
        break;
    case QEvent::TabletMove:
        mouseEvent = new QMouseEvent(QEvent::MouseMove, event->pos(),
                                     Qt::NoButton, Qt::NoButton, event->modifiers());
        mouseMoveEvent(mouseEvent);
        break;
    case QEvent::TabletRelease:
        mouseEvent = new QMouseEvent(QEvent::MouseButtonRelease, event->pos(),
                                     Qt::LeftButton, Qt::LeftButton, event->modifiers());
        mouseReleaseEvent(mouseEvent);
        break;
    default: break;
    }
    delete mouseEvent;
}

int KisPopupPalette::calculateIndex(QPointF point, int n)
{
    calculatePresetIndex(point, n);
    //translate to (0,0)
    point.setX(point.x() - width()/2);
    point.setY(point.y() - height()/2);

    //rotate
    float smallerAngle = M_PI/2 + M_PI/n - atan2 ( point.y(), point.x() );
    float radius = sqrt ( (float)point.x() * point.x() + point.y() * point.y() );
    point.setX( radius * cos(smallerAngle) );
    point.setY( radius * sin(smallerAngle) );

    //calculate brush index
    int pos = floor (acos(point.x()/radius) * n/ (2 * M_PI));
    if (point.y() < 0) pos =  n - pos - 1;

    return pos;
}

bool KisPopupPalette::isPointInPixmap(QPointF& point, int pos)
{
    if(pathFromPresetIndex(pos).contains(point + QPointF(-width()/2, -height()/2))) {
        return true;
    }
    return false;
}

KisPopupPalette::~KisPopupPalette()
{
    delete m_triangleColorSelector;
    m_triangleColorSelector = 0;
    m_resourceManager = 0;
}

QPainterPath KisPopupPalette::pathFromPresetIndex(int index)
{
    QRect outerRect(-width()/2, -height()/2, width(), height());
    outerRect.adjust(3, 3, -3, -3);
    int ringSize = brushOuterRadius - colorOuterRadius;
    QRect innerRect = outerRect.adjusted(ringSize, ringSize, -ringSize, -ringSize);

    qreal angleLength = 360/10;
    qreal angle = index*angleLength;
    
    QPainterPath path;
    path.moveTo( brushOuterRadius*cos(angle/180*M_PI),-brushOuterRadius*sin(angle/180*M_PI));
    path.arcTo ( outerRect, angle, angleLength );
    path.arcTo ( innerRect, angle + angleLength, -angleLength );
    path.closeSubpath();
    return path;
}

int KisPopupPalette::calculatePresetIndex(QPointF point, int /*n*/)
{
    int x = point.x() - width()/2;
    int y = point.y() - height()/2;

    qreal radius = sqrt ( (qreal) x*x + y*y );

    qreal angle;
    // y coordinate is the reverse of the cartesian one
    if(y < 0) {
        angle = acos(x/radius);
    } else {
        angle = 2*M_PI - acos(x/radius);
    }
    int pos = floor(angle/(2*M_PI/10));
    return pos;
}

#include "kis_popup_palette.moc"
