/*
 *  kfloatingtabdialog.cc - part of KImageShop
 *
 *  based on KTabCtl Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qtabbar.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "kfloatingtabdialog.h"

KFloatingTabDialog::KFloatingTabDialog(QWidget *parent, const char *name)
  : KFloatingDialog(parent, name)
{
  tabs = new QTabBar(this, "_tabbar");
  connect(tabs, SIGNAL(selected(int)), this, SLOT(showTab(int)));
  tabs->move(_left(), _top());
}

KFloatingTabDialog::~KFloatingTabDialog()
{
  delete tabs;
}

void KFloatingTabDialog::resizeEvent(QResizeEvent *e)
{
  KFloatingDialog::resizeEvent(e);

  if (tabs)
	{
	  for (int i=0; i < (int)pages.size(); i++)
		pages[i]->setGeometry(getChildRect());

	  if((tabs->shape() == QTabBar::RoundedBelow) ||
		 (tabs->shape() == QTabBar::TriangularBelow))
		tabs->move(_left(), _height()-tabs->height());
	}
}

void KFloatingTabDialog::setFont(const QFont & font)
{
  QFont f(font);
  f.setWeight(QFont::Light);
  QWidget::setFont(f);

  setSizes();
}

void KFloatingTabDialog::setTabFont(const QFont & font)
{
  QFont f(font);
  tabs->setFont(f);

  setSizes();
}

void KFloatingTabDialog::show()
{
  unsigned int i;

  if(isVisible())
	return;

  setSizes();

  for(i = 0; i < pages.size(); i++)
	pages[i]->hide();

  QResizeEvent r(size(), size());
  resizeEvent(&r);

  KFloatingDialog::show();
}

bool KFloatingTabDialog::isTabEnabled(const QString& name)
{
  unsigned int i;

  for(i = 0; i < pages.size(); i++)
    if (pages[i]->name() == name)
      return tabs->isTabEnabled(i);
  return false;
}

void KFloatingTabDialog::setTabEnabled(const QString& name, bool state)
{
  unsigned i;

  if (name.isEmpty())
	return;

  for (i = 0; i < pages.size(); i++)
	if (pages[i]->name() == name)
	  tabs->setTabEnabled(i, state);
}

void KFloatingTabDialog::setSizes()
{
  QSize min(tabs->sizeHint());    /* the minimum required size for the tabbar */
  tabs->resize(min);         /* make sure that the tabbar does not require more space than actually needed. */
}

void KFloatingTabDialog::setShape(QTabBar::Shape shape)
{
  tabs->setShape(shape);
}

QSize KFloatingTabDialog::sizeHint(void) const
{
  /* desired size of the tabbar */
  QSize hint(tabs->sizeHint());

  /* overall desired size of all pages */
  QSize pageHint;
  for (unsigned int i = 0; i < pages.size(); i++)
	{
	  QSize sizeI(pages[i]->sizeHint());

	  if (sizeI.isValid())
		{
		  /* only pages with valid size are used */
		  if (sizeI.width() > pageHint.width())
			pageHint.setWidth(sizeI.width());

		  if (sizeI.height() > pageHint.height())
			pageHint.setHeight(sizeI.height());
		}
	}

  if (pageHint.isValid())
	{
	  /* use maximum of width of tabbar and pages */
	  if (pageHint.width() > hint.width())
		hint.setWidth(pageHint.width());

	  /* heights must just be added */
	  hint.setHeight(hint.height() + pageHint.height());

	  return (hint);
	}

  /*
   * If not at least a one page has a valid sizeHint we have to return
   * an invalid size as well.
   */
  return (pageHint);
}

QRect KFloatingTabDialog::getChildRect()
{
  if((tabs->shape() == QTabBar::RoundedBelow) || (tabs->shape() == QTabBar::TriangularBelow))
	{
	  return QRect(_left(), _top(), _width(), _height() - tabs->height());
    }
  else
	{
	  return QRect(_left(), _top() + tabs->height() + 1, _width(), _height() - tabs->height());
    }
}

void KFloatingTabDialog::showTab(int i)
{
  unsigned int j;
  for (j = 0; j < pages.size(); j++)
	{
	  if (j != (unsigned)i)
		{
		  pages[j]->hide();
		}
	}

  if((unsigned)i < pages.size())
	{
	  emit(tabSelected(i));
	  if( pages.size() >= 2 )
		{
		  pages[i]->raise();
		}
	  tabs->setCurrentTab(i);
	  pages[i]->setGeometry(getChildRect());
	  pages[i]->show();
	}
}

void KFloatingTabDialog::addTab(QWidget *w, const QString& name)
{
  QTab *t = new QTab();
  t->setText(name);
  t->setEnabled(TRUE);
  int id = tabs->addTab(t);
  if (id == (int)pages.size())
	{
	  pages.resize(id + 1);
	  pages[id] = w;
    }
  setSizes();
}
#include "kfloatingtabdialog.moc"
