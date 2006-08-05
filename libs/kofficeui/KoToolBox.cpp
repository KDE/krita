/*
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (c) 2005-2006 Thomas Zander <zander@kde.org>

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
#include <QBoxLayout>
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
#include <KoToolFactory.h>
#include "KoToolBox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KoToolBox::KoToolBox() {
    m_buttonGroup = new QButtonGroup(this);
    setFeatures(DockWidgetMovable | DockWidgetFloatable);
}

KoToolBox::~KoToolBox() {
}

void KoToolBox::addButton(QAbstractButton *button, const QString &section, int priority, int buttonGroupId) {
    QMap<int, QAbstractButton*> buttons = m_buttons[section];
    buttons.insert(priority, button);
    m_buttons.insert(section, buttons);
    if(buttonGroupId < 0)
        m_buttonGroup->addButton(button);
    else
        m_buttonGroup->addButton(button, buttonGroupId);
    button->setCheckable(true);
}

void KoToolBox::setup() {
    QWidget *widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, widget);
    m_layout->setMargin(0);
    m_layout->setSpacing(KDialog::spacingHint());

    // loop over all sections.
    QList<QString> sections = m_buttons.keys();
    // but first make the main and dynamic be the first and last respectively.
    sections.removeAll(KoToolFactory::mainToolType());
    sections.insert(0, KoToolFactory::mainToolType());
    sections.removeAll(KoToolFactory::dynamicToolType());
    sections.append(KoToolFactory::dynamicToolType());
    foreach(QString section, sections) {
        ToolArea *ta = m_toolAreas.value(section);
        if(ta == 0) {
            ta = new ToolArea(widget);
            m_toolAreas.insert(section, ta);
            m_layout->addWidget(ta);
            m_layout->setAlignment(ta, Qt::AlignLeft | Qt::AlignTop);
        }
        QMap<int, QAbstractButton*> buttons = m_buttons[section];
        foreach(QAbstractButton *button, buttons.values()) {
            ta->add(button);
        }
    }
    m_buttons.clear();
    layout()->addWidget(widget);
    layout()->setAlignment(widget, Qt::AlignLeft | Qt::AlignTop);
    layout()->setMargin(0);
}

void KoToolBox::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    Qt::Orientation orientation = Qt::Vertical;
    QWidget *parent = parentWidget();
    bool floating=false;
    while(parent) {
        QMainWindow *mw = dynamic_cast<QMainWindow *> (parent);
        if(mw) {
            switch (mw->dockWidgetArea(this)) {
                case Qt::TopDockWidgetArea:
                case Qt::BottomDockWidgetArea:
                    orientation = Qt::Horizontal;
                    break;
                case Qt::NoDockWidgetArea:
                    floating = true;
                    break;
                default:
                    break;
            }
            break;
        }
        parent = parentWidget();
    }
    m_layout->setDirection(orientation == Qt::Horizontal
            ? QBoxLayout::LeftToRight
            : QBoxLayout::TopToBottom);
    foreach(ToolArea *area, m_toolAreas)
        area->setOrientation(orientation);

    adjustSize();
}

void KoToolBox::setActiveTool(int id) {
    QAbstractButton *button = m_buttonGroup->button(id);
    if(button)
        button->setChecked(true);
    else
        kWarning(30004) << "KoToolBox::setActiveTool(" << id << "): no such button found\n";
}

void KoToolBox::setVisibilityCode(QAbstractButton *button, const QString &code) {
    m_visibilityCodes.insert(button, code);
}

void KoToolBox::setButtonsVisible(const QList<QString> &codes) {
    foreach(QAbstractButton *button, m_visibilityCodes.keys())
        button->setVisible( codes.contains(m_visibilityCodes.value(button)) );
}

#if 0
void KoToolBox::slotButtonPressed( QAbstractButton *b)
{
    m_actionMap.value(b)->trigger();
}

void KoToolBox::registerTool( KAction *tool, int toolType, quint32 priority )
{
    uint prio = priority;
    ToolList * tl = m_tools.at(toolType);
    while( (*tl)[prio] ) prio++;
    (*tl)[prio] = tool;
}

QToolButton *KoToolBox::createButton(QWidget * parent, const QIcon& icon, QString tooltip)
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


void KoToolBox::setupTools()
{
    bool first=true;
    // Loop through tooltypes
    for (int i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;
        if (tl->isEmpty()) continue;

//       if(!first)
//          addSeparator();
        ToolArea *tools = new ToolArea( this );
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
        //addWidget(tools);
        m_toolBoxes.append(tools);
    }
}


void KoToolBox::setOrientation ( Qt::Orientation o )
{
#if 0
    if ( barPos() == Floating ) { // when floating, make it a standing toolbox.
        o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
    }
#endif

    //QToolBar::setOrientation( o );

    for (int i = 0; i < m_toolBoxes.count(); ++i) {
        ToolArea *t = m_toolBoxes.at(i);
        t->setOrientation(o);
    }
}


void KoToolBox::enableTools(bool enable)
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

void KoToolBox::slotSetTool(const QString & toolname)
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
#endif

// ----------------------------------------------------------------
//                         class ToolArea


ToolArea::ToolArea(QWidget *parent)
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
    grid->setMargin(0);
    grid->setSpacing(0);
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
    grid->setMargin(0);
    grid->setSpacing(0);
    m_rightLayout = new QBoxLayout(QBoxLayout::TopToBottom, m_rightRow);
    m_rightLayout->setMargin(0);
    m_rightLayout->setSpacing(1);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


ToolArea::~ToolArea()
{
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
    QBoxLayout::Direction  dir = (o != Qt::Horizontal
            ? QBoxLayout::TopToBottom
            : QBoxLayout::LeftToRight);
    m_leftLayout->setDirection(dir);
    m_rightLayout->setDirection(dir);

    m_layout->setDirection(o == Qt::Horizontal
            ? QBoxLayout::TopToBottom
            : QBoxLayout::LeftToRight);
}

#include "KoToolBox.moc"
