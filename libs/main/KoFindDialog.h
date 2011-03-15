/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



#ifndef KOFINDDIALOG_H
#define KOFINDDIALOG_H

#include <KDE/KFindDialog>
#include "komain_export.h"

class KActionCollection;
class KoFindBase;
class KOMAIN_EXPORT KoFindDialog : public KFindDialog
{
    Q_OBJECT
public:
    KoFindDialog(KoFindBase *find, KActionCollection *ac, QWidget *parent = 0);
    virtual ~KoFindDialog();

    virtual void accept();

private:
    class Private;
    Private * const d;
};

#endif // KOFINDDIALOG_H
