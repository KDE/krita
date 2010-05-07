/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOCONFIGMISCPAGE_H
#define KOCONFIGMISCPAGE_H

#include <kvbox.h>

#include "komain_export.h"

class KoDocument;

class KOMAIN_EXPORT KoConfigMiscPage : public KVBox
{
    Q_OBJECT

public:
    explicit KoConfigMiscPage(KoDocument* doc, char* name = 0L);
    ~KoConfigMiscPage();

    void apply();

signals:
    void unitChanged(int);

public slots:
    void slotDefault();
    void slotUnitChanged(int);

private:
    class Private;
    Private * const d;
};

#endif // KOCONFIGMISCPAGE_H
