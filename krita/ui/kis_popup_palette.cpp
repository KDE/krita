/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <vla24@sfu.ca>

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

const int KisPopupPalette::BUTTON_SIZE;

KisPopupPalette::KisPopupPalette(KoFavoriteResourceManager* manager, QWidget *parent)
    :QToolBox(parent, Qt::FramelessWindowHint)
    , m_resourceManager (manager)
    , m_brushButtonLayout(0)
    , m_brushButtonWidget(0)
    , m_colorLayout(0)
    , m_colorWidget(0)

{
    colorFoo=0;

//    QAction *quitAction = new QAction(tr("E&xit"), this);
//    quitAction->setShortcut(tr("Ctrl+P"));
//    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
//    addAction(quitAction);

    //FAVORITE BRUSHES
    m_brushButtonLayout = new FlowLayout(5);

    for (int pos = 0; pos < m_resourceManager->favoriteBrushesTotal(); pos ++)
    {
        // for paintops, the tooloptionbutton appears to be never set
        QWidget* w = m_resourceManager->favoriteBrush(pos)->paintopButton();
        if (w) {
            m_brushButtonLayout->addWidget(w);
        }
    }

    QVBoxLayout *m_tempLayout = new QVBoxLayout();
    m_tempLayout->addLayout(m_brushButtonLayout);
    m_tempLayout->setContentsMargins(5,0,5,0);

    m_brushButtonWidget = new QWidget();
    m_brushButtonWidget->setLayout(m_tempLayout);


    //RECENT COLORS
    QToolButton* m_chooseColor = new QToolButton ();
    m_chooseColor->setMaximumSize(KisPopupPalette::BUTTON_SIZE,KisPopupPalette::BUTTON_SIZE);
    m_chooseColor->setMinimumSize(KisPopupPalette::BUTTON_SIZE,KisPopupPalette::BUTTON_SIZE);
    m_chooseColor->setIcon(* (new QIcon (":/images/change_color.gif")));
    connect(m_chooseColor, SIGNAL(clicked()), this, SLOT(slotPickNewColor()));

    m_colorLayout = new FlowLayout(5);
    m_colorLayout->addWidget(m_chooseColor);

    m_tempLayout = new QVBoxLayout();
    m_tempLayout->addLayout(m_colorLayout);
    m_tempLayout->setContentsMargins(5,0,5,0);

    m_colorWidget = new QWidget();
    m_colorWidget->setLayout(m_tempLayout);

    //adding items
    addItem(m_brushButtonWidget, "Favorite Brushes");
    addItem(m_colorWidget, "Recently Used Colors");

    /****************************REMOVE THIS LATER**********************************/
//    this->setCurrentIndex(1);
    /****************************REMOVE THIS LATER**********************************/

    //clean up
    m_chooseColor = 0;
    m_tempLayout = 0;
    m_colorWidget = 0;
    m_brushButtonWidget = 0;

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
    return QSize(250, 150);
}

 void ShapedClock::mousePressEvent(QMouseEvent *event)
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

 void KisPopupPalette::mouseDoubleClickEvent(QMouseEvent *event)
 {
     if (event->buttons() && Qt::LeftButton)
     {
         qDebug() << "double click! double click!! lalalalala";
         event->accept();
     }
 }


KisPopupPalette::~KisPopupPalette()
{
    m_resourceManager = 0;
    delete m_brushButtonLayout;
    delete m_brushButtonWidget;
    delete m_colorLayout;
    delete m_colorWidget;
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
