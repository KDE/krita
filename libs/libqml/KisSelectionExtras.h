/* This file is part of the KDE project
 * Copyright (C) 2013 Camilla Boemann <cbo@boemann.dk>
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

#ifndef KISSELECTIONEXTRAS_H
#define KISSELECTIONEXTRAS_H

#include <QObject>

class KisViewManager;

// This class prvides some extra kisselectionmanager stuff that in krita prober is in plugins
class KisSelectionExtras : public QObject
{
    Q_OBJECT
public:
    KisSelectionExtras(KisViewManager *view);
    virtual ~KisSelectionExtras();

    Q_INVOKABLE void grow(qint32 xradius, qint32 yradius);
    Q_INVOKABLE void shrink(qint32 xradius, qint32 yradius, bool edge_lock);
    Q_INVOKABLE void border(qint32 xradius, qint32 yradius);
    Q_INVOKABLE void feather(qint32 radius);

private:
    KisViewManager *m_view;
};

#endif // KISSELECTIONEXTRAS_H
