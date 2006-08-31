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

#include "KoToolBox.h"

// koffice includes
#include <KoToolFactory.h>

// kde + qt includes
#include <kdialog.h>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QMainWindow>
#include <QBoxLayout>

KoToolBox::KoToolBox() : QDockWidget() {
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
    while(parent) {
        QMainWindow *mw = dynamic_cast<QMainWindow *> (parent);
        parent = parentWidget();
        if(mw == 0)
            continue;
        switch (mw->dockWidgetArea(this)) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                orientation = Qt::Horizontal;
                break;
            default:
                break;
        }
        break; // found it, lets stop.
    }
    m_layout->setDirection(orientation == Qt::Horizontal ?
            QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    foreach(ToolArea *area, m_toolAreas)
        area->setOrientation(orientation);
    adjustSize(); // make the toolbox be a sane size not depending on the last place it was docked
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

void KoToolBox::enableTools(bool enable) {
    foreach(ToolArea *ta, m_toolBoxes)
        ta->setEnabled(enable);
}

// ----------------------------------------------------------------
//                         class ToolArea
KoToolBox::ToolArea::ToolArea(QWidget *parent)
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

KoToolBox::ToolArea::~ToolArea()
{
}

void KoToolBox::ToolArea::add(QWidget *button)
{
    if (m_left)
        m_leftLayout->addWidget(button);
    else
        m_rightLayout->addWidget(button);
    button->show();
    m_left = !m_left;
}

QWidget* KoToolBox::ToolArea::getNextParent()
{
    if (m_left)
        return m_leftRow;
    return m_rightRow;
}

void KoToolBox::ToolArea::setOrientation ( Qt::Orientation o )
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
