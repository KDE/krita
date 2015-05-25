/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_VIEWMANAGER_H
#define LIBKIS_VIEWMANAGER_H

#include <QObject>

#include "libkis_export.h"

class KisViewManager;
class QAction;

class LIBKIS_EXPORT ViewManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewManager(KisViewManager *viewManager, QObject *parent = 0);
    QAction *createAction(const QString &text);

Q_SIGNALS:

public Q_SLOTS:

private:

    KisViewManager *m_viewManager;

};

#endif // LIBKIS_VIEW_H
