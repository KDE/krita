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
   1. How to automatically resize the widget, so there will be no scrollbars?
*/

#include "kis_popup_palette.h"
#include "kis_favorite_brush_data.h"
#include "kis_recent_color_data.h"
#include "flowlayout.h"
#include <QtGui>
#include <QDebug>
#include "kis_paintop_box.h"
#include <kis_types.h>
#include "ko_favorite_resource_manager.h"

#ifndef _MSC_EXTENSIONS
const int KisPopupPalette::BUTTON_SIZE;
#endif

KisPopupPalette::KisPopupPalette(KoFavoriteResourceManager* manager, QWidget *parent)
    : QToolBox(parent, Qt::FramelessWindowHint)
    , m_resourceManager (manager)
    , m_brushButtonLayout(0)
    , m_colorLayout(0)
{

    qDebug() << "[KisPopupPalette] I am constructed";
    colorFoo=0;

    //FAVORITE BRUSHES
    m_brushButtonLayout = new FlowLayout(5);

    for (int pos = 0; pos < m_resourceManager->favoriteBrushesTotal(); pos ++)
    {
        // for paintops, the tooloptionbutton appears to be never set
        QWidget* w = m_resourceManager->favoriteBrushButton(pos);
        if (w) {
            m_brushButtonLayout->addWidget(w);
        }
    }

    QVBoxLayout *tempLayout = new QVBoxLayout();
    tempLayout->addLayout(m_brushButtonLayout);
    tempLayout->setContentsMargins(5,0,5,0);

    QWidget* brushButtonWidget = new QWidget();
    brushButtonWidget->setLayout(tempLayout);
//    brushButtonWidget->setStyleSheet("* { background-color: rgba(0,0,0,128) }");

    //RECENT COLORS
    QToolButton* chooseColor = new QToolButton ();
    chooseColor->setMaximumSize(KisPopupPalette::BUTTON_SIZE,KisPopupPalette::BUTTON_SIZE);
    chooseColor->setMinimumSize(KisPopupPalette::BUTTON_SIZE,KisPopupPalette::BUTTON_SIZE);
    chooseColor->setIcon(* (new QIcon (":/images/change_color.gif")));
    chooseColor->setAutoFillBackground(false);
    connect(chooseColor, SIGNAL(clicked()), this, SLOT(slotPickNewColor()));

    m_colorLayout = new FlowLayout(5);
    m_colorLayout->addWidget(chooseColor);

    tempLayout = new QVBoxLayout();
    tempLayout->addLayout(m_colorLayout);
    tempLayout->setContentsMargins(5,0,5,0);

    QWidget* colorWidget = new QWidget();
    colorWidget->setLayout(tempLayout);
//    colorWidget->setStyleSheet("* { background-color: rgba(0,0,0,128) }");

    //adding items
    addItem(brushButtonWidget, "Favorite Brushes");
    addItem(colorWidget, "Recently Used Colors");

    /****************************REMOVE THIS LATER**********************************/
//    this->setCurrentIndex(1);
    /****************************REMOVE THIS LATER**********************************/

//    setStyleSheet("* { background-color: rgba(0,0,0,0) }");

    //clean up
    chooseColor = 0;
    tempLayout = 0;
    colorWidget = 0;
    brushButtonWidget = 0;

//    setAutoFillBackground(true);
//    setAttribute(Qt::WA_NoSystemBackground, true);
//    setAttribute(Qt::WA_OpaquePaintEvent, false);

//    for (int pos=0; pos< 2; pos++){
//        QWidget *w = widget(pos);
//        w->setAutoFillBackground(true);
//    }
}

void KisPopupPalette::paintEvent(QPaintEvent *event)
{
    QPainter painter (this);
    painter.setOpacity(0.5);
//    painter.fillRect(rect(), Qt::transparent);

    for (int pos=0; pos< 2; pos++){
        QWidget *w = widget(pos);
        QPalette palette(w->palette());
        palette.setColor(QPalette::Window, QColor(0,0,0,128));
        w->setPalette(palette);
    }
}

void KisPopupPalette::addFavoriteBrushButton(KisFavoriteBrushData* brush)
{
    if (brush->paintopButton()) {
        m_brushButtonLayout->addWidget(brush->paintopButton());
        updatePalette();
    }

}

void KisPopupPalette::removeFavoriteBrushButton(KisFavoriteBrushData* brush)
{
    if (brush->paintopButton()) {
        // qt has the concept of object hierarchy, i.e., one object owns another.
        // if you put a widget in a layout, like in addfavouritebrushbutton,
        // it becomes owned by its new parent. Now, if we delete the brush in
        // KisFavoriteBrushData::~KisFavoriteBrushData while it's still owned
        // by KisPopupPalette, you get a crash. So, set the parent to 0, and,
        // for added safety, check in the destructor KisFavoriteBrushData::~KisFavoriteBrushData
        // whether the button has a parent.
        brush->paintopButton()->setParent(0);
        // hiding it cannot hurt
        brush->paintopButton()->setVisible(false);
        // remove from the layout
        m_brushButtonLayout->removeWidget (brush->paintopButton());
        // delete the brush object itself. This isn't clean, the interaction between
        // KisView2's list of favourite brushes and the palette needs a bit of redesign,
        // these classes know way too much about each other.
        delete brush;
    }
    updatePalette();
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

}

QSize KisPopupPalette::sizeHint() const
{
    return QSize(200, 125);
}

 void KisPopupPalette::mousePressEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::LeftButton) {
         dragPosition = event->globalPos() - frameGeometry().topLeft();
         event->accept();
     }
 }

 void KisPopupPalette::mouseMoveEvent(QMouseEvent *event)
 {
     if (event->buttons() & Qt::LeftButton) {
         move(event->globalPos() - dragPosition);
         event->accept();
     }
 }

KisPopupPalette::~KisPopupPalette()
{
    m_resourceManager = 0;
    delete m_brushButtonLayout;
    delete m_colorLayout;
}

void KisPopupPalette::updatePalette()
{
    int tempIndex = currentIndex();
    this->setCurrentIndex(2/(tempIndex+1) - 1);
    this->setCurrentIndex(tempIndex);
}

void KisPopupPalette::addRecentColorButton(KisRecentColorData* newColor)
{
    m_colorLayout->addWidget(newColor->colorButton());
    updatePalette();
}

void KisPopupPalette::removeRecentColorButton(KisRecentColorData* color)
{
    m_colorLayout->removeWidget(color->colorButton());
    updatePalette();
}

#include "kis_popup_palette.moc"
