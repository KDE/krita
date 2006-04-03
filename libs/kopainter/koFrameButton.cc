/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)
  Copyright (c) 2001 Igor Jansen (rm@kde.org)

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

#include "koFrameButton.h"
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QLabel>

KoFrameButton::KoFrameButton(QWidget *parent, const char *name):
QLabel(parent)
{
  setObjectName(name);
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  setMaximumHeight(8);
  mActive = false;
  mToggle = false;
}

KoFrameButton::KoFrameButton(const QString &text, QWidget *parent, const char *name):
QLabel(parent)
{
  setObjectName(name);
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  setText(text);
  setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  mActive = false;
  mToggle = false;
}

KoFrameButton::KoFrameButton(const QPixmap &pixmap, QWidget *parent, const char *name):
QLabel(parent)
{
  setObjectName(name);
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  setPixmap(pixmap);
  mActive = false;
  mToggle = false;
}

void KoFrameButton::mousePressEvent(QMouseEvent *)
{
  setFrameStyle(Panel | Sunken);
}

void KoFrameButton::mouseReleaseEvent(QMouseEvent *)
{
  if(mToggle)
  {
    mActive = !mActive;
    if(mActive)
      setFrameStyle(Panel | Sunken);
    else
      setFrameStyle(Panel | Raised);
  }
  else
    setFrameStyle(Panel | Raised);

  emit clicked();
  emit clicked(mText);
}

void KoFrameButton::setOn(bool v)
{
  if(!mToggle)
    return;

  mActive = v;

  if(mActive)
    setFrameStyle(Panel | Sunken);
  else
    setFrameStyle(Panel | Raised);	
}


void KoFrameButton::setToggleButton(bool v)
{
  mToggle = v;
}

void KoFrameButton::setText(const QString &t)
{
  mText = t;
  QLabel::setText(t);
  setFixedSize(QLabel::sizeHint());
}

#include "koFrameButton.moc"
