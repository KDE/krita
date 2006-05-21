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

#include "KoIconTabPalette.h"

#include <QIcon>
#include <QVBoxLayout>

#include "KoPaletteTabWidget.h"

KoIconTabPalette::KoIconTabPalette(QWidget* parent, const char* name)
  : KoPalette(parent, name)
{
  setStyle(PALETTE_ICONTABS);

  QWidget* mainWidget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout(mainWidget);
  layout->setSpacing(0);
  layout->setMargin(0);
  mainWidget->setLayout(layout);

  m_tabWidget = new KoPaletteTabWidget(mainWidget);
  m_tabWidget->setFocusPolicy(Qt::TabFocus);

  layout->addWidget(m_tabWidget);
  layout->addStretch(1);
  setWidget(mainWidget);
}

KoIconTabPalette::~KoIconTabPalette()
{
}

void KoIconTabPalette::plug(QWidget* widget, const QString& name, int position)
{
  Q_UNUSED(name);

  if(!widget) return;

  m_tabWidget->setFont(QFont()); // Use the parent's font
  m_tabWidget->insertTab(position, widget, widget->windowIcon(),
                         widget->windowTitle());
  show();
}

void KoIconTabPalette::unplug(const QWidget* widget)
{
  m_tabWidget->takeTab(const_cast<QWidget*>(widget));

  if(m_tabWidget->visibleCount() == 0) {
    hide();
  }
}

void KoIconTabPalette::showPage(QWidget* widget)
{
  int index = indexOf(widget);

  if(index < 0) return;

  m_tabWidget->setTabHidden(index, false);

  if(m_tabWidget->visibleCount() == 0) {
    hide();
  }
}

void KoIconTabPalette::togglePageHidden(QWidget* widget)
{
  int index = indexOf(widget);

  if(index < 0) return;

  m_tabWidget->setTabHidden(index, !m_tabWidget->isTabHidden(index));

  if(m_tabWidget->visibleCount() == 0) {
    hide();
  }
}

void KoIconTabPalette::hidePage(QWidget* widget)
{
  int index = indexOf(widget);

  if(index < 0) return;

  m_tabWidget->setTabHidden(index, true);

  if(m_tabWidget->visibleCount() == 0) {
    hide();
  }
}

void KoIconTabPalette::makeVisible(bool visible)
{
  if(visible && (m_tabWidget->visibleCount() > 0))
  {
    show();
  } else {
    hide();
  }
}

bool KoIconTabPalette::isHidden(QWidget* widget)
{
  int index = indexOf(widget);

  if(index < 0) return true;

  return m_tabWidget->isTabHidden(index);
}

int KoIconTabPalette::indexOf(QWidget* widget)
{
  return m_tabWidget->indexOf(widget);
}

void KoIconTabPalette::resetFont()
{
  KoPalette::resetFont();
  m_tabWidget->setFont(QFont());
}

#include "KoIconTabPalette.moc"
