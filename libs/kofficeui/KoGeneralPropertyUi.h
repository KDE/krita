/* This file is part of the KDE project
   Copyright (C) 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KOGENERALPROPERTYUI_H
#define KOGENERALPROPERTYUI_H

#include "koffice_export.h"

#include "ui_KoGeneralPropertyUi.h"

// NOTE Stefan: just a small wrapper to get a visible symbol

class KOFFICEUI_EXPORT KoGeneralPropertyUI : public QWidget, public Ui::KoGeneralPropertyUI
{
public:
  KoGeneralPropertyUI(QWidget* parent = 0, Qt::WFlags fl = 0);
};

#endif // KOGENERALPROPERTYUI_H

