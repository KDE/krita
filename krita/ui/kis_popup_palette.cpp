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
#include "kis_recent_color_data.h"
#include "flowlayout.h"
#include <QtGui>
#include <QDebug>
#include "kis_paintop_box.h"
#include <kis_types.h>
#include "ko_favorite_resource_manager.h"
#include <QQueue>
#include <QtGui>
#include <math.h>
#define PI 3.14159265
#define DIAMETER 200
#define BRUSH_RADIUS 50

#ifndef _MSC_EXTENSIONS
int const KisPopupPalette::BUTTON_SIZE;
#endif

KisPopupPalette::KisPopupPalette(KoFavoriteResourceManager* manager, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , m_resourceManager (manager)
{
    setAutoFillBackground(true);
    setAttribute(Qt::WA_ContentsPropagated,true);
    connect(this, SIGNAL(changeActivePaintop(int)), m_resourceManager, SLOT(slotChangeActivePaintop(int)));
    connect(this, SIGNAL(selectNewColor()), this, SLOT(slotPickNewColor()));

    colorFoo=0;
}

void KisPopupPalette::slotPickNewColor()
{
    //TODO:get currently used Color
    KisRecentColorData *newColor;

    /****************************REMOVE THIS LATER**********************************/
    switch (colorFoo % 15){
        case 0:
            newColor = new KisRecentColorData(new QColor (255,0,0,255));
            break;
        case 1:
            newColor = new KisRecentColorData(new QColor (0,197,42,255));
            break;
        case 2:
            newColor = new KisRecentColorData(new QColor (192,0,255,255));
            break;
        case 3:
            newColor = new KisRecentColorData(new QColor (0,30,255,255));
            break;
        case 4:
            newColor = new KisRecentColorData(new QColor (116,227,255,255));
            break;
        case 5:
            newColor = new KisRecentColorData(new QColor (255,240,0,255));
            break;
        case 6:
            newColor = new KisRecentColorData(new QColor (119,156,110,255));
            break;
        case 7:
            newColor = new KisRecentColorData(new QColor (144,56,91,255));
            break;
        case 8:
            newColor = new KisRecentColorData(new QColor (162,201,255,255));
            break;
        case 9:
            newColor = new KisRecentColorData(new QColor (250,162,255,255));
            break;
        case 10:
            newColor = new KisRecentColorData(new QColor (255,215,162,255));
            break;
        case 11:
            newColor = new KisRecentColorData(new QColor (162,255,245,255));
            break;
        case 12:
            newColor = new KisRecentColorData(new QColor (234,255,162,255));
            break;
        case 13:
            newColor = new KisRecentColorData(new QColor (105,111,123,255));
            break;
        default:
            newColor = new KisRecentColorData(new QColor (255,162,162,255));
            break;
    }
    colorFoo++;

    qDebug() << "Color to be added: (r)" << newColor->color()->red() << "(g)" << newColor->color()->green()
            << "(b)" << newColor->color()->blue();
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
            qDebug() << "[KisPopupPalette] out of canvas region x";
            pointPalette.setX(pointPalette.x() - offsetX);
        }
        if ((offsetY = pointPalette.y() + height() - parentSize.height()) > 0 || (offsetY = pointPalette.y()) < 0)
        {
            qDebug() << "[KisPopupPalette] out of canvas region y";
            pointPalette.setY(pointPalette.y() - offsetY);
        }
        move(pointPalette);

    }
    setVisible(!isVisible());
}

void KisPopupPalette::mouseReleaseEvent ( QMouseEvent * event )
{
    QPointF point = event->posF();
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        QPainterPath pathBrush(drawBrushDonutPath(width()/2, height()/2));
        QPainterPath pathColor(drawColorDonutPath(width()/2, height()/2));
        QPainterPath pathSelCol;
        pathSelCol.addEllipse(QPoint(width()/2, height()/2), 30,30);

        qDebug() << "[KisPopupPalette] mouse position: " << point;

        if (pathBrush.contains(point))
        { //in favorite brushes area
            int pos = calculateIndex(point, m_resourceManager->favoriteBrushesTotal());
            qDebug() << "[KisPopupPalette] favorite brush position: " << pos;
            if (pos >= 0 && pos < m_resourceManager->favoriteBrushesTotal())
            {
                QPixmap pixmap(m_resourceManager->favoriteBrushPixmap(pos));

                //calculating if the point is inside the pixmap
                float angle = pos*PI*2.0/m_resourceManager->favoriteBrushesTotal();
                QPainterPath path;
                path.addRect(BRUSH_RADIUS*sin(angle)-pixmap.width()/2+width()/2,
                              BRUSH_RADIUS*cos(angle)-pixmap.height()/2+height()/2,
                              pixmap.width(), pixmap.height());

                if(path.contains(point) || pixmap.isNull()) emit changeActivePaintop(pos);
            }
        }
        else if (pathColor.contains(point))
        {
            int pos = calculateIndex(point, m_resourceManager->recentColorsList()->size());
            qDebug() << "[KisPopupPalette] selected color: " << *(m_resourceManager->recentColorsList()->at(pos)->color());
        }
        else if (pathSelCol.contains(point))
        {
            qDebug() << "[KisPopupPalette] in select new color";
            emit selectNewColor();
        }
    }
    else if (event->button() == Qt::MidButton)
    {
        setVisible(false);
    }
}

QSize KisPopupPalette::sizeHint() const
{
    return QSize(DIAMETER,DIAMETER);
}

void KisPopupPalette::resizeEvent(QResizeEvent*)
{
    int side = qMin(width(), height());
    QRegion maskedRegion (width()/2 - side/2, height()/2 - side/2, side, side, QRegion::Ellipse);
    setMask(maskedRegion);
}

int KisPopupPalette::calculateIndex(QPointF point, int n)
{
    //translate to (0,0)
    point.setX(point.x() - width()/2);
    point.setY(point.y() - height()/2);

    //rotate
    float smallerAngle = PI/2 + PI/n - atan2 ( point.y(), point.x() );
    float radius = sqrt ( point.x()*point.x() + point.y()*point.y() );
    point.setX( radius * cos(smallerAngle) );
    point.setY( radius * sin(smallerAngle) );

    //calculate brush index
    int pos = floor (acos(point.x()/radius) * n/ (2 * PI));
    if (point.y() < 0) pos =  n - pos - 1;

    return pos;
}

QPainterPath KisPopupPalette::drawColorDonutPath(int x, int y)
{
    QPainterPath path_ColorDonut;
    path_ColorDonut.addEllipse(QPointF(x,y), width()/2 - 10, height()/2 - 10);
    path_ColorDonut.addEllipse(QPointF(x,y), width()/2 - 30, height()/2 - 30);
    path_ColorDonut.setFillRule(Qt::OddEvenFill);

    return path_ColorDonut;
}

QPainterPath KisPopupPalette::drawBrushDonutPath(int x, int y)
{
    QPainterPath path_BrushDonut;
    path_BrushDonut.addEllipse(QPointF(x,y), width()/2 - 40, height()/2 - 40);
    path_BrushDonut.addEllipse(QPointF(x,y), width()/2 - 60, height()/2 - 60);
    path_BrushDonut.setFillRule(Qt::OddEvenFill);

    return path_BrushDonut;
}

void KisPopupPalette::paintEvent(QPaintEvent*)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2, height()/2);

//    QPainterPath path_BrushDonut(drawBrushDonutPath(0,0));
//    painter.fillPath(path_BrushDonut, Qt::blue);
//    painter.drawPath(path_BrushDonut);

    QList<QPixmap> pixmaps (m_resourceManager->favoriteBrushPixmaps());

    //painting favorite brushes
    for (int pos = 0; pos < pixmaps.size(); pos++)
    {
        QPixmap pixmap(pixmaps.at(pos));
        QPointF pixmapOffset(pixmap.width()/2, pixmap.height()/2);

        float angle = pos*PI*2.0/pixmaps.size();
        QPointF pointTemp(BRUSH_RADIUS*sin(angle),BRUSH_RADIUS*cos(angle));
        painter.drawPixmap(QPoint(pointTemp.x()-pixmapOffset.x(), pointTemp.y()-pixmapOffset.y()), pixmap);
    }

    //painting recent colors
    painter.setPen(Qt::NoPen);
    QQueue <KisRecentColorData*>* colors (m_resourceManager->recentColorsList());
    
    for (int pos = 0; pos < colors->size(); pos++)
    {
        QPainterPath path;
        path.setFillRule(Qt::OddEvenFill);
        path.moveTo(0,0);
        path.arcTo(10.0 - width()/2, 10.0 - height()/2, 2*(width()/2 - 10),2*(height()/2 - 10),-90.0 - 180/colors->size(),
            360/colors->size());
        path.lineTo(0,0);
        path.arcTo(30.0 - width()/2, 30.0 - height()/2, 2*(width()/2 - 30),2*(height()/2 - 30),-90.0 - 180/colors->size(),
            360/colors->size());
        path.closeSubpath();
    
        //accessing recent color of index pos
        painter.fillPath(path, *(colors->at(pos)->color()));
    
        painter.drawPath(path);
        painter.rotate(360.0/colors->size());
    }

    QPainterPath path_ColorDonut(drawColorDonutPath(0,0));
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawPath(path_ColorDonut);

    //temporary new color 'button'
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(0,0), 30,30);
    painter.drawText(QPoint(-15,0), "color");
}

void KisPopupPalette::mouseMoveEvent(QMouseEvent* e)
{
    e->accept();
}

void KisPopupPalette::mousePressEvent(QMouseEvent* e)
{
    e->accept();
}

KisPopupPalette::~KisPopupPalette()
{
    m_resourceManager = 0;
}

#include "kis_popup_palette.moc"
