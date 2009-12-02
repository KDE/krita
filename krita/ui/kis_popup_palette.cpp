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
}

void KisPopupPalette::slotPickNewColor()
{
    //TODO:get currently used Color
//    KisRecentColorData *newColor;
//
//    /****************************REMOVE THIS LATER**********************************/
//    switch (colorFoo % 15){
//        case 0:
//            newColor = new KisRecentColorData(new QColor (255,0,0,255));
//            break;
//        case 1:
//            newColor = new KisRecentColorData(new QColor (0,197,42,255));
//            break;
//        case 2:
//            newColor = new KisRecentColorData(new QColor (192,0,255,255));
//            break;
//        case 3:
//            newColor = new KisRecentColorData(new QColor (0,30,255,255));
//            break;
//        case 4:
//            newColor = new KisRecentColorData(new QColor (116,227,255,255));
//            break;
//        case 5:
//            newColor = new KisRecentColorData(new QColor (255,240,0,255));
//            break;
//        case 6:
//            newColor = new KisRecentColorData(new QColor (119,156,110,255));
//            break;
//        case 7:
//            newColor = new KisRecentColorData(new QColor (144,56,91,255));
//            break;
//        case 8:
//            newColor = new KisRecentColorData(new QColor (162,201,255,255));
//            break;
//        case 9:
//            newColor = new KisRecentColorData(new QColor (250,162,255,255));
//            break;
//        case 10:
//            newColor = new KisRecentColorData(new QColor (255,215,162,255));
//            break;
//        case 11:
//            newColor = new KisRecentColorData(new QColor (162,255,245,255));
//            break;
//        case 12:
//            newColor = new KisRecentColorData(new QColor (234,255,162,255));
//            break;
//        case 13:
//            newColor = new KisRecentColorData(new QColor (105,111,123,255));
//            break;
//        default:
//            newColor = new KisRecentColorData(new QColor (255,162,162,255));
//            break;
//    }
//    colorFoo++;
//
//    qDebug() << "Color to be added: (r)" << newColor->color()->red() << "(g)" << newColor->color()->green()
//            << "(b)" << newColor->color()->blue();
//    /****************************REMOVE THIS LATER**********************************/
//
//    //TODO: develop this more!
//    m_resourceManager->addRecentColor(newColor);
//
//    qDebug() << "new color!!";

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

        qDebug() << "[KisPopupPalette] mouse position: " << point;

        if (pathBrush.contains(point))
        { //in favorite brushes area
            int pos = calculateFavoriteBrush(point);;
            qDebug() << "[KisPopupPalette] favorite brush position: " << pos;
            if (pos >= 0 && pos < m_resourceManager->favoriteBrushesTotal())
                emit changeActivePaintop(pos);
        }
        else if (pathColor.contains(point))
        {
            qDebug() << "[KisPopupPalette] in recent colour area";
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

int KisPopupPalette::calculateFavoriteBrush(QPointF point)
{
    //translate to (0,0)
    point.setX(point.x() - width()/2);
    point.setY(point.y() - height()/2);

    //rotate
    float smallerAngle = PI/2 + PI/m_resourceManager->favoriteBrushesTotal() - atan2 ( point.y(), point.x() );
    float radius = sqrt ( point.x()*point.x() + point.y()*point.y() );
    point.setX( radius * cos(smallerAngle) );
    point.setY( radius * sin(smallerAngle) );

    //calculate brush index
    int pos = floor (acos(point.x()/radius) * m_resourceManager->favoriteBrushesTotal()/ (2 * PI));
    if (point.y() < 0) pos =  m_resourceManager->favoriteBrushesTotal() - pos - 1;

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
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

    QPainterPath path_ColorDonut(drawColorDonutPath(0,0));
    painter.fillPath(path_ColorDonut, Qt::red);
    painter.drawPath(path_ColorDonut);

//    QPainterPath path_BrushDonut(drawBrushDonutPath(0,0));
//    painter.fillPath(path_BrushDonut, Qt::blue);
//    painter.drawPath(path_BrushDonut);

    QList<QPixmap> pixmaps (m_resourceManager->favoriteBrushPixmaps());

    for (int pos = 0; pos < pixmaps.size(); pos++)
    {
        QPixmap pixmap(pixmaps.at(pos));
        QPointF pixmapOffset(pixmap.width()/2, pixmap.height()/2);

        float angle = pos*PI*2.0/pixmaps.size();
        QPointF pointTemp(BRUSH_RADIUS*sin(angle),BRUSH_RADIUS*cos(angle));
        painter.drawPixmap(QPoint(pointTemp.x()-pixmapOffset.x(), pointTemp.y()-pixmapOffset.y()), pixmap);
    }
}

KisPopupPalette::~KisPopupPalette()
{
    m_resourceManager = 0;
}

#include "kis_popup_palette.moc"
