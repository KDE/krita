/*
 *  newlayerdialog.h - part of KImageShop
 *
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
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

#ifndef __newlayerdialog_h__
#define __newlayerdialog_h__

#include <qspinbox.h>
#include <qlineedit.h>

#include <kdialog.h>

class NewLayerDialog : public KDialog
{
  Q_OBJECT

public:

  NewLayerDialog( QWidget *parent = 0, const char *name = 0 );

  QString name() { return m_name->text(); };
  int width() { return m_width->value(); };
  int height() { return m_height->value(); };

private:

  QLineEdit *m_name;
  QSpinBox  *m_width;
  QSpinBox  *m_height;
};

#endif // __newlayerdialog.h__
