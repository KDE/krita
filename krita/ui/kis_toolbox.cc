/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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


#include <qbuttongroup.h>
#include <qnamespace.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kparts/event.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionclasses.h>

#include <koMainWindow.h>
#include "kis_toolbox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KisToolBox::KisToolBox( KMainWindow *mainWin, const char* name, KInstance* instance, int numberOfTooltypes )
    : KToolBar( mainWin, Qt::DockLeft, false, name, true, true), m_instance(instance)
{
    setFullSize( false );
    setMargin(3);

    m_buttonGroup = new QButtonGroup( 0L );
    m_buttonGroup->setExclusive( true );
    connect( m_buttonGroup, SIGNAL( pressed( int ) ), this, SLOT( slotButtonPressed( int ) ) );

    // Create separate lists for the various sorts of tools
    for (int i = 0; i < numberOfTooltypes ; ++i) {
        ToolList * tl = new ToolList();
        m_tools.append(tl);
    }
    setBarPos(Left);
}

KisToolBox::~KisToolBox()
{
}


void KisToolBox::slotPressButton( int id )
{
    m_buttonGroup->setButton( id );
    slotButtonPressed( id );
}

void KisToolBox::slotButtonPressed( int id )
{
    if( id != m_buttonGroup->selectedId() && m_buttonGroup->selected() ) {
        m_buttonGroup->selected()->setDown( false );
    }
    m_idToActionMap.at(id)->activate();

}

void KisToolBox::registerTool( KAction *tool, int toolType, Q_UINT32 priority )
{
    uint prio = priority;
    ToolList * tl = m_tools.at(toolType);
    while( (*tl)[prio] ) prio++;
    (*tl)[prio] = tool;
}

QToolButton *KisToolBox::createButton(QWidget * parent,  const char* iconName, QString tooltip)
{
    QToolButton *button = new QToolButton(parent);

    if ( iconName != "" ) {
        QPixmap pixmap = BarIcon( iconName, m_instance );
        button->setPixmap( pixmap );
        button->setToggleButton( true );
    }

    if ( !tooltip.isEmpty() ) {
        QToolTip::add( button, tooltip );
    }
    return button;
}

void KisToolBox::setupTools()
{
    int id = 0;
    // Loop through tooltypes
    for (uint i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;
        if (tl->isEmpty()) continue;

        ToolArea *tools = new ToolArea( this );
        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
        {
            KAction *tool = it.data();
            if(! tool)
                continue;
            QToolButton *bn = createButton(tools->getNextParent(), tool->icon().latin1(), tool->toolTip());
            // XXX: Find out how to switch buttons on action activate
            tools->add(bn);
            m_buttonGroup->insert( bn, id++ );
            m_idToActionMap.append( tool );
        }
        addSeparator();
        m_toolBoxes.append(tools);
    }
    // select first (select tool)
    m_buttonGroup->setButton( 0 );
    m_numberOfButtons = id;
}

void KisToolBox::setOrientation ( Qt::Orientation o )
{
    if ( barPos() == Floating ) { // when floating, make it a standing toolbox.
        o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
    }

    QDockWindow::setOrientation( o );

    for (uint i = 0; i < m_toolBoxes.count(); ++i) {
        ToolArea *t = m_toolBoxes.at(i);
        t->setOrientation(o);
    }
}

void KisToolBox::enableTools(bool enable)
{
    if (m_tools.isEmpty()) return;
    if (!m_buttonGroup) return;
    if (m_numberOfButtons <= 0) return;

    for (uint i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);
    
        if (!tl) continue;

        
        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
            if (it != 0 && it.data())
                it.data()->setEnabled(enable);
    }
    m_buttonGroup->setEnabled(enable);
    for (Q_UINT32 i = 0; i < m_numberOfButtons; ++i) {
        m_buttonGroup->find( i )->setEnabled( enable );
    }
}


ToolArea::ToolArea(QWidget *parent)
    : QWidget(parent), m_left(true)
{
    m_layout = new QBoxLayout(this, QBoxLayout::LeftToRight, 0, 0, 0);
    QWidget *w = new QWidget(this);
    m_layout->addWidget(w);

    QGridLayout *grid = new QGridLayout(w, 2, 2);
    m_leftRow = new QWidget(w);
    grid->addWidget(m_leftRow, 0, 0);
    m_leftLayout = new QBoxLayout(m_leftRow, QBoxLayout::TopToBottom, 0, 1, 0);

    w = new QWidget(this);
    m_layout->addWidget(w);
    grid = new QGridLayout(w, 2, 2);
    m_rightRow = new QWidget(w);
    grid->addWidget(m_rightRow, 0, 0);

    m_rightLayout = new QBoxLayout(m_rightRow, QBoxLayout::TopToBottom, 0, 1, 0);
}

void ToolArea::add(QWidget *button)
{
    if (m_left)
        m_leftLayout->addWidget(button);
    else
        m_rightLayout->addWidget(button);
    button->show();
    m_left = !m_left;
}

QWidget* ToolArea::getNextParent()
{
    if (m_left)
        return m_leftRow;
    return m_rightRow;
}

void ToolArea::setOrientation ( Qt::Orientation o )
{
    QBoxLayout::Direction dir = o != Qt::Horizontal?QBoxLayout::TopToBottom:QBoxLayout::LeftToRight;
    m_leftLayout->setDirection(dir);
    m_rightLayout->setDirection(dir);
    m_layout->setDirection(o == Qt::Horizontal?QBoxLayout::TopToBottom:QBoxLayout::LeftToRight);
}

#include "kis_toolbox.moc"
