/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KOCOPYCONTROLLER_H
#define KOCOPYCONTROLLER_H

#include "kofficeui_export.h"

#include <QObject>

class QAction;
class KoCanvasBase;

class KOFFICEUI_EXPORT KoCopyController : public QObject {
    Q_OBJECT
public:
    KoCopyController(KoCanvasBase *canvas, QAction *copyAction);
    ~KoCopyController();

signals:
    /// emitted when the user pressed paste and the current tool had no selection.
    void copyRequested();

public slots:
    void hasSelection(bool selection);

private:
    Q_PRIVATE_SLOT(d, void copy())
    Q_PRIVATE_SLOT(d, void selectionChanged(bool))

    class Private;
    Private * const d;
};

#endif
