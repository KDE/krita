/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>

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
 * Boston, MA 02110-1301, USA.
*/

#include "Kolinewidthaction.h"

#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <q3whatsthis.h>
#include <qmenubar.h>
#include <QLayout>
#include <QLabel>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kmenu.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktoolbar.h>

#include <kiconloader.h>
#include <klocale.h>

#include <KoUnitWidgets.h>
#include <KoGlobal.h>

class KoLineWidthAction::KoLineWidthActionPrivate
{
  public:
    KoLineWidthActionPrivate()
    {
      m_currentWidth = 1.0;
      m_unit = KoUnit::U_PT;
    }

    ~KoLineWidthActionPrivate()
    {
    }

    double m_currentWidth;
    KoUnit::Unit m_unit;
};

KoLineWidthAction::KoLineWidthAction(const QString &text, const QString& icon,
  QObject* parent, const char* name) : KoSelectAction(text, icon, parent, name)
{
  d = new KoLineWidthActionPrivate;

  createMenu();
}

KoLineWidthAction::KoLineWidthAction(const QString &text, const QString& icon, const QObject* receiver,
  const char* slot, QObject* parent, const char* name) : KoSelectAction(text, icon, parent, name)
{
  d = new KoLineWidthActionPrivate;

  createMenu();

  connect(this, SIGNAL(lineWidthChanged(double)), receiver, slot);
}

KoLineWidthAction::~KoLineWidthAction()
{
  delete d;
}

void KoLineWidthAction::createMenu()
{
  KMenu* popup = popupMenu();
  QBitmap mask;
  QPixmap pix(70, 21);
  QPainter p(&pix, popup);
  int cindex = 0;
  QPen pen;

  for(int i = 1; i <= 10; i++) {
    pix.fill(Qt::white);
    pen.setWidth(qRound(i * POINT_TO_INCH(static_cast<double>(KoGlobal::dpiY()))));
    p.setPen(pen);
    p.drawLine(0, 10, pix.width(), 10);
    mask = pix;
    pix.setMask(mask);
    popup->insertItem(pix,cindex++);
  }

  popup->insertSeparator(cindex++);
  popup->insertItem(i18n("&Custom..."), cindex++);
}

void KoLineWidthAction::execute(int index)
{
  bool ok = false;

  if((index >= 0) && (index < 10)) {
    d->m_currentWidth = (double) index + 1.0;
    ok = true;
  } if(index == 11) { // Custom width dialog...
    KoLineWidthChooser dlg(qApp->activeWindow());
    dlg.setUnit(d->m_unit);
    dlg.setWidth(d->m_currentWidth);

    if(dlg.exec()) {
      d->m_currentWidth = dlg.width();
      ok = true;
    }
  }

  if(ok) {
    setCurrentSelection(index);
    emit lineWidthChanged(d->m_currentWidth);
  }
}

double KoLineWidthAction::currentWidth() const
{
  return d->m_currentWidth;
}

void KoLineWidthAction::setCurrentWidth(double width)
{
  d->m_currentWidth = width;

  // Check if it is a standard width...
  for(int i = 1; i <= 10; i++) {
    if(KoUnit::toPoint(width) == (double) i) {
      setCurrentSelection(i - 1);
      return;
    }
  }

  //Custom width...
  setCurrentSelection(11);
}

void KoLineWidthAction::setUnit(KoUnit::Unit unit)
{
  d->m_unit = unit;
}

//////////////////////////////////////////////////
//
// KoLineWidthChooser
//

class KoLineWidthChooser::KoLineWidthChooserPrivate
{
  public:
    KoUnitDoubleSpinBox* m_lineWidthUSBox;
};

KoLineWidthChooser::KoLineWidthChooser(QWidget* parent, const char* name)
 : KDialogBase(parent, name, true, i18n("Custom Line Width"), Ok|Cancel, Ok)
{
  d = new KoLineWidthChooserPrivate;

  // Create the ui
  QWidget* mainWidget = new QWidget(this);
  setMainWidget(mainWidget);
  Q3GridLayout* gl = new Q3GridLayout(mainWidget, 1, 2, KDialog::marginHint(), KDialog::spacingHint());
  QLabel* textLbl = new QLabel(i18n("Line width:"), mainWidget);
  d->m_lineWidthUSBox = new KoUnitDoubleSpinBox(mainWidget, 0.0, 1000.0, 0.1, 1.0, KoUnit::U_PT, 2);
  gl->addWidget(textLbl, 0, 0);
  gl->addWidget(d->m_lineWidthUSBox, 0, 1);
}

KoLineWidthChooser::~KoLineWidthChooser()
{
  delete d;
}

double KoLineWidthChooser::width() const
{
  return d->m_lineWidthUSBox->value();
}

void KoLineWidthChooser::setUnit(KoUnit::Unit unit)
{
  d->m_lineWidthUSBox->setUnit(unit);
}

void KoLineWidthChooser::setWidth(double width)
{
  d->m_lineWidthUSBox->changeValue(width);
}

#include "Kolinewidthaction.moc"
