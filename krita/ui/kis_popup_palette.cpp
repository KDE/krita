/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

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

   Known issues:
   1. calculateFavoriteBrush() sometimes does not return the right value. Find
      a better formula!
*/

#include "kis_popup_palette.h"
#include <QtGui>
#include <QDebug>
#include "kis_paintop_box.h"
#include <kis_types.h>
#include "ko_favorite_resource_manager.h"
#include <QQueue>
#include <QtGui>
#include <math.h>

KisPopupPalette::KisPopupPalette(KoFavoriteResourceManager* manager, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , m_resourceManager (manager)
{
    setAutoFillBackground(true);
    setAttribute(Qt::WA_ContentsPropagated,true);
    connect(this, SIGNAL(changeActivePaintop(int)), m_resourceManager, SLOT(slotChangeActivePaintop(int)));
    connect(this, SIGNAL(selectNewColor()), this, SLOT(slotPickNewColor()));
    connect(this, SIGNAL(updateRecentColor(int)), m_resourceManager, SLOT(slotUpdateRecentColor(int)));

    colorFoo=0;
    setMouseTracking(true);
    setHoveredBrush(-1);
    setHoveredColor(-1);
    setSelectedBrush(-1);
    setSelectedColor(-1);
}

//setting KisPopupPalette properties
int KisPopupPalette::hoveredBrush() const { return m_hoveredBrush; }
void KisPopupPalette::setHoveredBrush(int x) { m_hoveredBrush = x; }
int KisPopupPalette::selectedBrush() const { return m_selectedBrush; }
void KisPopupPalette::setSelectedBrush(int x) { m_selectedBrush = x; }
int KisPopupPalette::hoveredColor() const { return m_hoveredColor; }
void KisPopupPalette::setHoveredColor(int x) { m_hoveredColor = x; }
int KisPopupPalette::selectedColor() const { return m_selectedColor; }
void KisPopupPalette::setSelectedColor(int x) { m_selectedColor = x; }

void KisPopupPalette::slotPickNewColor()
{
    //TODO:get currently used Color
    QColor newColor;

    /****************************REMOVE THIS LATER**********************************/
    switch (colorFoo % 15){
        case 0:
            newColor.setRgb (255,0,0,255);
            break;
        case 1:
            newColor.setRgb (0,197,42,255);
            break;
        case 2:
            newColor.setRgb (192,0,255,255);
            break;
        case 3:
            newColor.setRgb (0,30,255,255);
            break;
        case 4:
            newColor.setRgb (116,227,255,255);
            break;
        case 5:
            newColor.setRgb (255,240,0,255);
            break;
        case 6:
            newColor.setRgb (119,156,110,255);
            break;
        case 7:
            newColor.setRgb (144,56,91,255);
            break;
        case 8:
            newColor.setRgb (162,201,255,255);
            break;
        case 9:
            newColor.setRgb (250,162,255,255);
            break;
        case 10:
            newColor.setRgb (255,215,162,255);
            break;
        case 11:
            newColor.setRgb (162,255,245,255);
            break;
        case 12:
            newColor.setRgb (234,255,162,255);
            break;
        case 13:
            newColor.setRgb (105,111,123,255);
            break;
        default:
            newColor.setRgb (255,162,162,255);
            break;
    }
    colorFoo++;

    qDebug() << "Color to be added: (r)" << newColor.red() << "(g)" << newColor.green()
            << "(b)" << newColor.blue();
    /****************************REMOVE THIS LATER**********************************/

    //TODO: develop this more!
    m_resourceManager->addRecentColor(newColor);

    qDebug() << "new color!!";

    update();

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
    setVisible(!isVisible());
}

QSize KisPopupPalette::sizeHint() const
{
    return QSize(200,200);
}

void KisPopupPalette::resizeEvent(QResizeEvent*)
{
    int side = qMin(width(), height());
    QRegion maskedRegion (width()/2 - side/2, height()/2 - side/2, side, side, QRegion::Ellipse);
    setMask(maskedRegion);
}

void KisPopupPalette::paintEvent(QPaintEvent*)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2, height()/2);

    //painting favorite brushes
    QList<QPixmap> pixmaps (m_resourceManager->favoriteBrushPixmaps());

    //highlight hovered brush
    if (hoveredBrush() > -1)
    {
        painter.rotate((pixmaps.size() - hoveredBrush()) *360.0/pixmaps.size());
        QPainterPath path(drawDonutPathAngle(brushInnerRadius(),brushOuterRadius(), pixmaps.size()));
        painter.fillPath(path, palette().color(QPalette::Highlight));
        painter.rotate(hoveredBrush() *360.0/pixmaps.size());
    }

    //highlight selected brush
    if (selectedBrush() > -1)
    {
        painter.rotate((pixmaps.size() - selectedBrush()) *360.0/pixmaps.size());
        QPainterPath path(drawDonutPathAngle(brushInnerRadius(),brushOuterRadius(), pixmaps.size()));
        painter.fillPath(path, palette().color(QPalette::Highlight).darker(130));
        painter.rotate(selectedBrush() *360.0/pixmaps.size());
    }

    //painting favorite brushes pixmap/icon
    for (int pos = 0; pos < pixmaps.size(); pos++)
    {
        QPixmap pixmap(pixmaps.at(pos));
        QPointF pixmapOffset(pixmap.width()/2, pixmap.height()/2);

        float angle = pos*PI()*2.0/pixmaps.size();
        QPointF pointTemp(brushRadius()*sin(angle),brushRadius()*cos(angle));
        painter.drawPixmap(QPoint(pointTemp.x()-pixmapOffset.x(), pointTemp.y()-pixmapOffset.y()), pixmap);
    }

    //painting recent colors
    painter.setPen(Qt::NoPen);
    float rotationAngle = -360.0/m_resourceManager->recentColorsTotal();
    
    for (int pos = 0; pos < m_resourceManager->recentColorsTotal(); pos++)
    {
        QPainterPath path(drawDonutPathAngle(colorInnerRadius() , colorOuterRadius(), m_resourceManager->recentColorsTotal()));
    
        //accessing recent color of index pos
        painter.fillPath(path, m_resourceManager->recentColorAt(pos));
    
        painter.drawPath(path);
        painter.rotate(rotationAngle);
    }

    //temporary new color 'button'
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(0,0), 30,30);
    painter.drawText(QPoint(-15,0), "color");

    painter.setBrush(Qt::transparent);

    if (m_resourceManager->recentColorsTotal() == 0)
    {
        QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius(), colorOuterRadius()));
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter.drawPath(path_ColorDonut);
    }

    //painting hovered color
    if (hoveredColor() > -1)
    {
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

        if (m_resourceManager->recentColorsTotal() == 1)
        {
            QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius(), colorOuterRadius()));
            painter.drawPath(path_ColorDonut);
        }
        else
        {
            painter.rotate((m_resourceManager->recentColorsTotal() + hoveredColor()) *rotationAngle);
            QPainterPath path(drawDonutPathAngle(colorInnerRadius(),colorOuterRadius(), m_resourceManager->recentColorsTotal()));
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
            QPainterPath path_ColorDonut(drawDonutPathFull(0,0,colorInnerRadius(), colorOuterRadius()));
            painter.drawPath(path_ColorDonut);
        }
        else
        {
            painter.rotate((m_resourceManager->recentColorsTotal() + selectedColor()) *rotationAngle);
            QPainterPath path(drawDonutPathAngle(colorInnerRadius(),colorOuterRadius(), m_resourceManager->recentColorsTotal()));
            painter.drawPath(path);
            painter.rotate(selectedColor() *-1 *rotationAngle);
        }
    }
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
    path.moveTo(-1*outer_radius * sin(PI()/limit),
                   outer_radius * cos(PI()/limit));
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

    QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius(), brushOuterRadius()));
    QPainterPath pathColor(drawDonutPathFull(width()/2, height()/2, colorInnerRadius(), colorOuterRadius()));

    setHoveredBrush(-1);
    setHoveredColor(-1);

    if (pathBrush.contains(point))
    { //in favorite brushes area
        int pos = calculateIndex(point, m_resourceManager->favoriteBrushesTotal());

        if (pos >= 0 && pos < m_resourceManager->favoriteBrushesTotal()
                && isPointInPixmap(point, pos))
        {
            setHoveredBrush(pos);
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
        QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius(), brushOuterRadius()));
        QPainterPath pathColor(drawDonutPathFull(width()/2, height()/2, colorInnerRadius(), colorOuterRadius()));

        if (pathBrush.contains(point))
        { //in favorite brushes area
            int pos = calculateIndex(point, m_resourceManager->favoriteBrushesTotal());
            if (pos >= 0 && pos < m_resourceManager->favoriteBrushesTotal()
                && isPointInPixmap(point, pos))
            {
                setSelectedBrush(pos);
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
        QPainterPath pathBrush(drawDonutPathFull(width()/2, height()/2, brushInnerRadius(), brushOuterRadius()));
        QPainterPath pathColor(drawDonutPathFull(width()/2, height()/2, colorInnerRadius(), colorOuterRadius()));
        QPainterPath pathSelCol;
        pathSelCol.addEllipse(QPoint(width()/2, height()/2), 30,30);

        if (pathBrush.contains(point))
        { //in favorite brushes area
            if (hoveredBrush() > -1)
            {
                setSelectedBrush(hoveredBrush());
                emit changeActivePaintop(hoveredBrush());
            }
        }
        else if (pathColor.contains(point))
        {
            int pos = calculateIndex(point, m_resourceManager->recentColorsTotal());

            if (pos >= 0 && pos < m_resourceManager->recentColorsTotal())
            {
                qDebug() << "[KisPopupPalette] pos: " << pos;
                emit updateRecentColor(pos);
            }
        }
        else if (pathSelCol.contains(point))
        {
            emit selectNewColor();
        }
    }
    else if (event->button() == Qt::MidButton)
    {
        setVisible(false);
    }
}

int KisPopupPalette::calculateIndex(QPointF point, int n)
{
    //translate to (0,0)
    point.setX(point.x() - width()/2);
    point.setY(point.y() - height()/2);

    //rotate
    float smallerAngle = PI()/2 + PI()/n - atan2 ( point.y(), point.x() );
    float radius = sqrt ( point.x()*point.x() + point.y()*point.y() );
    point.setX( radius * cos(smallerAngle) );
    point.setY( radius * sin(smallerAngle) );

    //calculate brush index
    int pos = floor (acos(point.x()/radius) * n/ (2 * PI()));
    if (point.y() < 0) pos =  n - pos - 1;

    return pos;
}

bool KisPopupPalette::isPointInPixmap(QPointF& point, int pos)
{
    QPixmap pixmap(m_resourceManager->favoriteBrushPixmap(pos));

    //calculating if the point is inside the pixmap
    float angle = pos*PI()*2.0/m_resourceManager->favoriteBrushesTotal();
    QPainterPath path;
    path.addRect(brushRadius()*sin(angle)-pixmap.width()/2+width()/2,
                 brushRadius()*cos(angle)-pixmap.height()/2+height()/2,
                 pixmap.width(), pixmap.height());

    if(path.contains(point) || pixmap.isNull()) return true;
    else return false;
}

KisPopupPalette::~KisPopupPalette()
{
    m_resourceManager = 0;
}

#include "kis_popup_palette.moc"
