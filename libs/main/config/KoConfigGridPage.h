/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOGRIDCONFIGPAGE_H
#define KOGRIDCONFIGPAGE_H

#include <QWidget>

#include "komain_export.h"

class KoDocument;
class KoGridData;

class KOMAIN_EXPORT KoConfigGridPage : public QWidget
{
    Q_OBJECT

public:
    explicit KoConfigGridPage(KoDocument* doc, char* name = 0L);
    ~KoConfigGridPage();

    void apply();

public slots:
    void slotDefault();
    void slotUnitChanged(int);
    void spinBoxHSpacingChanged(qreal);
    void spinBoxVSpacingChanged(qreal);

private:
    void setValuesFromGrid(const KoGridData &grid);

    class Private;
    Private * const d;
};

#endif // KOGRIDCONFIGPAGE_H
