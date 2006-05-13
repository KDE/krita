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

#include "Kolinestyleaction.h"

#include <QPainter>
#include <QPixmap>
#include <QBitmap>

#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>

class KoLineStyleAction::KoLineStyleActionPrivate
{
  public:
    KoLineStyleActionPrivate()
    {
      m_currentStyle = Qt::SolidLine;
    }
    
    ~KoLineStyleActionPrivate()
    {
    }
    
    int m_currentStyle;
};

KoLineStyleAction::KoLineStyleAction(const QString &text, const QString& icon,
  QObject* parent, const char* name) : KoSelectAction(text, icon, parent, name)
{
  d = new KoLineStyleActionPrivate;
   
  createMenu();
}

KoLineStyleAction::KoLineStyleAction(const QString &text, const QString& icon, const QObject* receiver,
  const char* slot, QObject* parent, const char* name) : KoSelectAction(text, icon, receiver, slot, parent, name)
{
  d = new KoLineStyleActionPrivate;
  
  createMenu();
}

KoLineStyleAction::~KoLineStyleAction()
{
  delete d;
}

void KoLineStyleAction::createMenu()
{
  KMenu* popup = popupMenu();
  QBitmap mask;
  QPixmap pix(70, 21);
  QPainter p(&pix, popup);
  int cindex = 0;
  QPen pen;
  pen.setWidth(2);
  popup->insertItem(i18n("None"),cindex++);

  for(int i = 1; i < 6; i++) {
    pix.fill(Qt::white);
    pen.setStyle(static_cast<Qt::PenStyle>(i));
    p.setPen(pen);
    p.drawLine(0, 10, pix.width(), 10);
    mask = pix;
    pix.setMask(mask);
    popup->insertItem(pix,cindex++);
  }
}

#include "Kolinestyleaction.moc"
