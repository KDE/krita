/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_widget_chooser.h"

#include <KoIcon.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QLabel>
#include <QButtonGroup>
#include <QStylePainter>
#include <QStyleOption>

#include "kis_config.h"

KisWidgetChooser::KisWidgetChooser(int id, QWidget* parent)
    : QFrame(parent)
    , m_chooserid(id)
{
//     QFrame::setFrameStyle(QFrame::StyledPanel|QFrame::Raised);
    
    m_acceptIcon  = koIcon("list-add");
    m_buttons     = new QButtonGroup();
    m_popup       = new QFrame(0, Qt::Popup);
    m_arrowButton = new QToolButton();
    
    m_popup->setFrameStyle(QFrame::Panel|QFrame::Raised);
    m_arrowButton->setIcon(arrowIcon());
    m_arrowButton->setAutoRaise(true);
    
    connect(m_arrowButton, SIGNAL(clicked(bool)), SLOT(slotButtonPressed()));
}

KisWidgetChooser::~KisWidgetChooser()
{
    delete m_buttons;
}

QIcon KisWidgetChooser::arrowIcon()
{
    QImage image(16, 16, QImage::Format_ARGB32);
    image.fill(0);
    
    QStylePainter painter(&image, this);
    QStyleOption  option;
    
    option.rect    = image.rect();
    option.palette = palette();
    option.state   = QStyle::State_Enabled;
    option.palette.setBrush(QPalette::ButtonText, Qt::black); // Force color to black
    
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
    painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, option);
    
    return QIcon(QPixmap::fromImage(image));
}

void KisWidgetChooser::addWidget(const QString& id, const QString& label, QWidget* widget)
{
    if(id.isEmpty()) {
        delete widget;
        return;
    }
    
    removeWidget(id);
    m_widgets.push_back(Data(id, widget, new QLabel(label)));
    
    delete m_popup->layout();
    m_popup->setLayout(createPopupLayout());
    m_popup->adjustSize();
    
    delete QWidget::layout();
    QWidget::setLayout(createLayout());
}

QLayout* KisWidgetChooser::createLayout()
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(2, 0, 0, 0);
    layout->setSpacing(0);
    
    for(Iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i) {
        if(i->choosen) {
            layout->addWidget(i->label);
            layout->addWidget(i->widget);
            break;
        }
    }
    
    layout->addWidget(m_arrowButton);
    return layout;
}

QLayout* KisWidgetChooser::createPopupLayout()
{
    QGridLayout* layout = new QGridLayout();
    int          row    = 0;
    int          idx    = 0;
    
    layout->setContentsMargins(2, 0, 0, 0);
    layout->setSpacing(0);
    
    QButtonGroup*           group   = new QButtonGroup();
    QList<QAbstractButton*> buttons = m_buttons->buttons();
    
    for(Iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i) {
        if(!i->choosen) {
            if(row == buttons.size()) {
                QToolButton* bn = new QToolButton();
                bn->setIcon(m_acceptIcon);
                bn->setAutoRaise(true);
                buttons.push_back(bn);
            }
            
            layout->addWidget(i->label    , row, 0);
            layout->addWidget(i->widget   , row, 1);
            layout->addWidget(buttons[row], row, 2);
            group->addButton(buttons[row], idx);
            ++row;
        }
        
        ++idx;
    }
    
    for(int i=row; i<buttons.size(); ++i)
        delete buttons[i];
    
    delete m_buttons;
    
    m_buttons = group;
    connect(m_buttons, SIGNAL(buttonClicked(int)), SLOT(slotWidgetChoosen(int)));
    
    return layout;
}

void KisWidgetChooser::removeWidget(const QString& id)
{
    Iterator data = qFind(m_widgets.begin(), m_widgets.end(), Data(id));
    
    if(data != m_widgets.end()) {
        if(!data->choosen) {
            delete m_popup->layout();
            m_popup->setLayout(createPopupLayout());
            m_popup->adjustSize();
        }
        else delete QWidget::layout();
        
        delete data->label;
        delete data->widget;
        m_widgets.erase(data);
    }
}

QWidget* KisWidgetChooser::chooseWidget(const QString& id)
{
    QWidget* choosenWidget = 0;
    
    for(Iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i) {
        if(i->id == id) {
            choosenWidget = i->widget;
            i->choosen    = true;
        }
        else i->choosen = false;
    }
    
    delete m_popup->layout();
    m_popup->setLayout(createPopupLayout());
    m_popup->adjustSize();
    
    delete QWidget::layout();
    QWidget::setLayout(createLayout());
    
    KisConfig cfg;
    cfg.setToolbarSlider(m_chooserid, id);

    return choosenWidget;
}

QWidget* KisWidgetChooser::getWidget(const QString& id) const
{
    ConstIterator data = qFind(m_widgets.begin(), m_widgets.end(), Data(id));
    
    if(data != m_widgets.end())
        return data->widget;
    
    return 0;
}

void KisWidgetChooser::showPopupWidget()
{
    QSize popSize = m_popup->size();
    QRect popupRect(QFrame::mapToGlobal(QPoint(0, QFrame::height())), popSize);
    
    // Get the available geometry of the screen which contains this KisPopupButton
    QRect screenRect = QApplication::desktop()->availableGeometry(this);
    
    // Make sure the popup is not drawn outside the screen area
    if(popupRect.right() > screenRect.right())
        popupRect.translate(screenRect.right() - popupRect.right(), 0);
    if(popupRect.left() < screenRect.left())
        popupRect.translate(screenRect.left() - popupRect.left(), 0);
    if(popupRect.bottom() > screenRect.bottom())
        popupRect.translate(0, -popupRect.height());
    
    m_popup->setGeometry(popupRect);
    m_popup->show();
}

void KisWidgetChooser::slotButtonPressed()
{
    showPopupWidget();
}

void KisWidgetChooser::slotWidgetChoosen(int index)
{
    chooseWidget(m_widgets[index].id);
    m_popup->hide();
}
