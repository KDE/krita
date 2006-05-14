/* This file is part of the KDE project
   Copyright (C)  2006 Peter Simonsson <peter.simonsson@gmail.com>

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

#include "KoPaletteTabWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include "KoPaletteTabBar.h"

KoPaletteTabWidget::KoPaletteTabWidget(QWidget* parent)
  : QWidget(parent)
{
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  m_tabBar = new KoPaletteTabBar(this);
  connect(m_tabBar, SIGNAL(tabSelectionChanged(int, bool)),
      this, SLOT(setTabActive(int, bool)));
  mainLayout->addWidget(m_tabBar);

  m_tabLayout = new QVBoxLayout(this);
  m_tabLayout->setMargin(0);
  m_tabLayout->setSpacing(0);
  mainLayout->addLayout(m_tabLayout);

  setLayout(mainLayout);
}

KoPaletteTabWidget::~KoPaletteTabWidget()
{
}

void KoPaletteTabWidget::addTab(QWidget* child, const QIcon& icon, const QString& toolTip)
{
  insertWidgetInLayout(child);
  m_tabBar->addTab(icon, toolTip);
  m_widgets.append(child);
}

void KoPaletteTabWidget::insertTab(int index, QWidget* child, const QIcon& icon, const QString& toolTip)
{
  insertWidgetInLayout(child);
  m_tabBar->insertTab(index, icon, toolTip);
  m_widgets.insert(index, child);
}

void KoPaletteTabWidget::removeTab(QWidget* child)
{
  removeTab(m_widgets.indexOf(child));
  child = 0;
}

void KoPaletteTabWidget::removeTab(int index)
{
  QWidget* child = takeTab(index);
  delete child;
  child = 0;
}

void KoPaletteTabWidget::takeTab(QWidget* child)
{
  takeTab(m_widgets.indexOf(child));
}

QWidget* KoPaletteTabWidget::takeTab(int index)
{
  setTabActive(index, false);
  m_tabBar->removeTab(index);
  QWidget* widget = m_widgets.takeAt(index);

  return widget;
}

int KoPaletteTabWidget::indexOf(QWidget* child)
{
  return m_widgets.indexOf(child);
}

bool KoPaletteTabWidget::isTabHidden(int index)
{
  return m_tabBar->isTabHidden(index);
}

int KoPaletteTabWidget::count() const
{
  return m_tabBar->count();
}

int KoPaletteTabWidget::visibleCount() const
{
  return m_tabBar->visibleCount();
}

void KoPaletteTabWidget::setTabHidden(int index, bool hide)
{
  if(hide) {
    setTabActive(index, false);
  }

  m_tabBar->setTabHidden(index, hide);
}

void KoPaletteTabWidget::setTabActive(int index, bool active)
{
  m_widgets.at(index)->setVisible(active);
}

void KoPaletteTabWidget::insertWidgetInLayout(QWidget* child)
{
  child->setParent(this);
  m_tabLayout->addWidget(child);

  if(m_widgets.isEmpty()) {
    child->show();
  } else {
    child->hide();
  }
}

#include "KoPaletteTabWidget.moc"
