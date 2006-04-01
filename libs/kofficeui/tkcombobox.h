/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef TKCOMBOBOX_H
#define TKCOMBOBOX_H

#include <q3combobox.h>
//Added by qt3to4:
#include <QPaintEvent>
class TKComboBox : public Q3ComboBox
{ Q_OBJECT
public:
  TKComboBox(QWidget* parent=0, const char* name=0);
  TKComboBox(bool isEditable, QWidget* parent=0, const char* name=0);
  ~TKComboBox();

  void activate();

protected:
  void paintEvent(QPaintEvent*);

private:
  class TKComboBoxPrivate;
  TKComboBoxPrivate *d;
};

#endif
