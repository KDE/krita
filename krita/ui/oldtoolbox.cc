/*
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QButtonGroup>
#include <qnamespace.h>
#include <QToolButton>
#include <QLabel>
#include <QToolTip>
#include <QLayout>
#include <QPixmap>
#include <QGridLayout>
#include <QSpacerItem>
#include <QSizePolicy>

#include <kdebug.h>
#include <kparts/event.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kaction.h>
#include <kactioncollection.h>

#include <KoMainWindow.h>
#include "oldtoolbox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

OldToolBox::OldToolBox( KMainWindow *mainWin, const char* name, KInstance* instance, int numberOfTooltypes )
    : KToolBar( name, mainWin, Qt::LeftToolBarArea, false, true, true ),
    m_instance( instance )
{
    setObjectName(name);

    m_buttonGroup = new QButtonGroup( );
    connect( m_buttonGroup, SIGNAL( buttonClicked( QAbstractButton *) ), this, SLOT( slotButtonPressed( QAbstractButton *) ) );
    connect( this, SIGNAL(  orientationChanged ( Qt::Orientation )), this, SLOT(setOrientation ( Qt::Orientation ) ) );

    // Create separate lists for the various sorts of tools
    for (int i = 0; i < numberOfTooltypes ; ++i) {
        ToolList * tl = new ToolList();
        m_tools.append(tl);
    }
}

OldToolBox::~OldToolBox()
{
    qDeleteAll(m_tools);
    delete m_buttonGroup;
}

void OldToolBox::slotButtonPressed( QAbstractButton *b)
{
    m_actionMap.value(b)->trigger();
}

void OldToolBox::registerTool( KAction *tool, int toolType, quint32 priority )
{
    uint prio = priority;
    ToolList * tl = m_tools.at(toolType);
    while( (*tl)[prio] ) prio++;
    (*tl)[prio] = tool;
}

QToolButton *OldToolBox::createButton(QWidget * parent, const QIcon& icon, QString tooltip)
{
    QToolButton *button = new QToolButton(parent);

    if ( !icon.isNull() ) {
        button->setIcon(icon);
        button->setCheckable( true );
    }

    if ( !tooltip.isEmpty() ) {
        button->setToolTip( tooltip );
    }
    return button;
}


void OldToolBox::setupTools()
{
    bool first=true;
    // Loop through tooltypes
    for (int i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;
        if (tl->isEmpty()) continue;

        if(!first)
           addSeparator();
        OldToolArea *tools = new OldToolArea( this );
        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
        {
            KAction *tool = it.value();
            if(! tool)
                continue;
            QToolButton *bn = createButton(tools->getNextParent(), tool->icon(), tool->toolTip());
            tools->add(bn);
            m_buttonGroup->addButton(bn);
            m_actionMap.insert( bn, tool );
            if(first)
                bn->setChecked(true);
            first=false;
        }
        tools->show();
        addWidget(tools);
        m_toolBoxes.append(tools);
    }
}


void OldToolBox::setOrientation ( Qt::Orientation o )
{
#if 0
    if ( barPos() == Floating ) { // when floating, make it a standing toolbox.
        o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
    }
#endif

    QToolBar::setOrientation( o );

    for (int i = 0; i < m_toolBoxes.count(); ++i) {
        OldToolArea *t = m_toolBoxes.at(i);
        t->setOrientation(o);
    }
}


void OldToolBox::enableTools(bool enable)
{
    if (m_tools.isEmpty()) return;
    if (!m_buttonGroup) return;

    for (int i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;

        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
            if (it.value())
                it.value()->setEnabled(enable);
    }
}

void OldToolBox::slotSetTool(const QString & toolname)
{
    QMapIterator<QAbstractButton *, KAction *> i(m_actionMap);
    while (i.hasNext()) {
        i.next();
        KAction * a = i.value();
        if (a && a->objectName() == toolname)
        {
            i.key()->setChecked(true);
            return;
        }
    }
}


// ----------------------------------------------------------------
//                         class OldToolArea


OldToolArea::OldToolArea(QWidget *parent)
    : QWidget(parent), m_left(true)
{
    m_layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    QWidget *w = new QWidget(this);
    m_layout->addWidget(w);
    QGridLayout *grid = new QGridLayout(w);
    m_leftRow = new QWidget(w);
    grid->addWidget(m_leftRow, 0, 0);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);
    m_leftLayout = new QBoxLayout(QBoxLayout::TopToBottom, m_leftRow);
    m_leftLayout->setMargin(0);
    m_leftLayout->setSpacing(1);

    w = new QWidget(this);
    m_layout->addWidget(w);
    grid = new QGridLayout(w);
    m_rightRow = new QWidget(w);
    grid->addWidget(m_rightRow, 0, 0);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);
    m_rightLayout = new QBoxLayout(QBoxLayout::TopToBottom, m_rightRow);
    m_rightLayout->setMargin(0);
    m_rightLayout->setSpacing(1);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


OldToolArea::~OldToolArea()
{
}


void OldToolArea::add(QWidget *button)
{
    if (m_left)
        m_leftLayout->addWidget(button);
    else
        m_rightLayout->addWidget(button);
    button->show();
    m_left = !m_left;
}


QWidget* OldToolArea::getNextParent()
{
    if (m_left)
        return m_leftRow;
    return m_rightRow;
}


void OldToolArea::setOrientation ( Qt::Orientation o )
{
    QBoxLayout::Direction  dir = (o != Qt::Horizontal
				  ? QBoxLayout::TopToBottom
				  : QBoxLayout::LeftToRight);
    m_leftLayout->setDirection(dir);
    m_rightLayout->setDirection(dir);

    m_layout->setDirection(o == Qt::Horizontal
			   ? QBoxLayout::TopToBottom
			   : QBoxLayout::LeftToRight);
}


#include "oldtoolbox.moc"
