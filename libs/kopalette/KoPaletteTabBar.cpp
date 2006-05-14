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

#include "KoPaletteTabBar.h"

#include <QList>
#include <QIcon>
#include <QString>
#include <QRect>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QToolTip>

#include <kdebug.h>

class KoPaletteTabBarPrivate
{
  public:
    typedef struct Tab {
      Tab()
        : selected(false),
          underMouse(false),
          hidden(false)
      {
      }

      QIcon icon;
      QString toolTip;
      QRect rect;

      bool selected;
      bool underMouse;
      bool hidden;
    };

    KoPaletteTabBarPrivate()
      : m_iconSize(16),
        m_dirtyTabList(false),
        m_visibleCount(0)
    {
    }

    ~KoPaletteTabBarPrivate()
    {
      foreach(KoPaletteTabBarPrivate::Tab* tab, m_tabList) {
        delete tab;
      }

      m_tabList.clear();
    }

    Tab* tabUnderMouse(const QPoint& position)
    {
      foreach(KoPaletteTabBarPrivate::Tab* tab, m_tabList) {
        if(tab->rect.contains(position)) {
          return tab;
        }
      }

      return 0;
    }

    int m_iconSize;

    QList<Tab*> m_tabList;
    bool m_dirtyTabList;
    int m_visibleCount;
};

KoPaletteTabBar::KoPaletteTabBar(QWidget* parent)
  : QWidget(parent)
{
  d = new KoPaletteTabBarPrivate;

  setIconSize(16);
  setMouseTracking(true); // Needed for mouse over effects
}

KoPaletteTabBar::~KoPaletteTabBar()
{
  delete d;
}

int KoPaletteTabBar::addTab(const QIcon& icon, const QString& toolTip)
{
  KoPaletteTabBarPrivate::Tab* tab = new KoPaletteTabBarPrivate::Tab;
  tab->icon = icon;
  tab->toolTip = toolTip;

  if(d->m_tabList.isEmpty()) {
    tab->selected = true;
  }

  d->m_tabList.append(tab);
  d->m_visibleCount++;

  // Relayout the tabs and update the widget
  d->m_dirtyTabList = true;
  update();

  return d->m_tabList.count() - 1;
}

void KoPaletteTabBar::insertTab(int index, const QIcon& icon, const QString& toolTip)
{
  KoPaletteTabBarPrivate::Tab* tab = new KoPaletteTabBarPrivate::Tab;
  tab->icon = icon;
  tab->toolTip = toolTip;

  if(d->m_tabList.isEmpty()) {
    tab->selected = true;
  }

  d->m_tabList.insert(index, tab);
  d->m_visibleCount++;

  // Relayout the tabs and update the widget
  d->m_dirtyTabList = true;
  update();
}

void KoPaletteTabBar::removeTab(int index)
{
  KoPaletteTabBarPrivate::Tab* tab = d->m_tabList.takeAt(index);

  if(tab) {
    if(!tab->hidden) {
      d->m_visibleCount--;
    }

    delete tab;
    tab = 0;
  }
}

void KoPaletteTabBar::setIconSize(int size)
{
  d->m_iconSize = size;
  d->m_dirtyTabList = true;
  setFixedHeight(d->m_iconSize + 5);
}

bool KoPaletteTabBar::isTabHidden(int index)
{
  return d->m_tabList.at(index)->hidden;
}

int KoPaletteTabBar::count() const
{
  return d->m_tabList.count();
}

int KoPaletteTabBar::visibleCount() const
{
  return d->m_visibleCount;
}

void KoPaletteTabBar::setTabHidden(int index, bool hide)
{
  if(!hide && d->m_tabList.at(index)->hidden) {
    d->m_visibleCount++;
  } else if(hide && !d->m_tabList.at(index)->hidden) {
    d->m_visibleCount--;
  }

  d->m_tabList.at(index)->hidden = hide;

  // Relayout the tabs and update the widget
  d->m_dirtyTabList = true;
  update();
}

void KoPaletteTabBar::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  QPainter painter(this);

  if(d->m_dirtyTabList) {
    layoutTabs();
  }

  painter.setPen(palette().color(QPalette::WindowText));
  QColor selectionColor = palette().color(QPalette::Highlight);
  selectionColor.setAlpha(128);
  QBrush selectionBrush(selectionColor, Qt::SolidPattern);
  QBrush hoverBrush(Qt::NoBrush);

  foreach(KoPaletteTabBarPrivate::Tab* tab, d->m_tabList) {
    if(!tab->hidden) {
      if(tab->selected) {
        painter.setBrush(selectionBrush);
        painter.drawRect(tab->rect);
      } else if(tab->underMouse) {
        painter.setBrush(hoverBrush);
        painter.drawRect(tab->rect);
      }

      tab->icon.paint(&painter, tab->rect.x() + 2, tab->rect.y() + 2,
                      d->m_iconSize, d->m_iconSize, Qt::AlignCenter,
                      tab->underMouse ? QIcon::Active : QIcon::Normal,
                      tab->selected ? QIcon::On : QIcon::Off);
    }
  }
}

void KoPaletteTabBar::mouseMoveEvent(QMouseEvent* event)
{
  foreach(KoPaletteTabBarPrivate::Tab* tab, d->m_tabList) {
    tab->underMouse = false;
  }

  KoPaletteTabBarPrivate::Tab* tab = d->tabUnderMouse(event->pos());

  if(tab) {
    tab->underMouse = true;
  }

  update();
}

void KoPaletteTabBar::mouseReleaseEvent(QMouseEvent* event)
{
  KoPaletteTabBarPrivate::Tab* tab = d->tabUnderMouse(event->pos());

  if(tab) {
    tab->selected = !tab->selected;

    if(!(event->modifiers() & Qt::ControlButton)) {
      foreach(KoPaletteTabBarPrivate::Tab* otherTab, d->m_tabList) {
        if((otherTab != tab) && otherTab->selected) {
          otherTab->selected = false;
          emit tabSelectionChanged(d->m_tabList.indexOf(otherTab),
                                   otherTab->selected);
        }
      }
    }

    update();
    emit tabSelectionChanged(d->m_tabList.indexOf(tab), tab->selected);
  }
}

void KoPaletteTabBar::layoutTabs()
{
  int offset = 0;
  int tabWidth = d->m_iconSize + 4;
  int tabHeight = d->m_iconSize + 4;

  foreach(KoPaletteTabBarPrivate::Tab* tab, d->m_tabList) {
    tab->rect = QRect(offset * tabWidth, 0, tabWidth, tabHeight);
    offset++;
  }

  d->m_dirtyTabList = false;
}

bool KoPaletteTabBar::event(QEvent* event)
{
  if(event->type() == QEvent::ToolTip) {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    KoPaletteTabBarPrivate::Tab* tab = d->tabUnderMouse(helpEvent->pos());

    if(tab) {
      QToolTip::showText(helpEvent->globalPos(), tab->toolTip, this);
    }

    event->accept();
    return true;
  }

  return QWidget::event(event);
}

#include "KoPaletteTabBar.moc"
