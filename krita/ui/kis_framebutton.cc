/*
 *  kis_framebutton.cc - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kis_framebutton.h"

KisFrameButton::KisFrameButton(QWidget *parent, const char *name )
  : QLabel( parent, name )
{
  setAutoResize(true);
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  m_active = false;
  m_toggle = false;
}

KisFrameButton::KisFrameButton( const QString& text, QWidget* parent, const char* name )
  : QLabel( parent, name )
{
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  setText(text);
  m_active = false;
  m_toggle = false;
}

KisFrameButton::KisFrameButton( const QPixmap& pixmap, QWidget* parent, const char* name )
  : QLabel( parent, name )
{
  setFrameStyle(Panel | Raised);
  setLineWidth(1);
  setPixmap(pixmap);
  m_active = false;
  m_toggle = false;
}

void KisFrameButton::mousePressEvent( QMouseEvent * )
{
  setFrameStyle(Panel | Sunken);
}

void KisFrameButton::mouseReleaseEvent( QMouseEvent * )
{
  if (m_toggle)
	{
	  m_active = !m_active;
	  
	  if (m_active)
		setFrameStyle(Panel | Sunken);
	  else
		setFrameStyle(Panel | Raised);
	}
  else
	setFrameStyle(Panel | Raised);

  emit clicked();
  emit clicked(m_text);
}

void KisFrameButton::setOn(bool v)
{
  if (!m_toggle)
	return;

  m_active = v;
  
  if (m_active)
	setFrameStyle(Panel | Sunken);
  else
	setFrameStyle(Panel | Raised);	
}


void KisFrameButton::setToggleButton(bool v)
{
  m_toggle = v;
}

void KisFrameButton::setText( const QString &t )
{
  m_text = t;
  QLabel::setText(t);
}

#include "kis_framebutton.moc"
