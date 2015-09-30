/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KOPAGEWIDGETITEM_H
#define KOPAGEWIDGETITEM_H

#include "kritawidgets_export.h"

class QWidget;
class QString;

// This class can be implemented when we want to extend the
// dialog with new, specific pages.
class KRITAWIDGETS_EXPORT KoPageWidgetItem
{
public:
    virtual ~KoPageWidgetItem() {}

    virtual QWidget *widget() = 0;
    virtual QString name() const = 0;
    virtual QString iconName() const = 0;
    virtual bool shouldDialogCloseBeVetoed() = 0;
    virtual void apply() = 0;
};


#endif // KOPAGEWIDGETITEM_H
