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

#include "flake_export.h"

#include <QObject>

class QAction;
class KoCanvasBase;
class KoCopyControllerPrivate;

/**
 * This class takes care of the copy actions integration into flake.
 * Whenever the copy (KStandardAction::Copy) action is triggered the controller
 * will use the currently selected tool and try to copy to the clipboard using that tool.
 * Additionally; when the tool does not allow copying (KoToolBase::hasSelection() returns false)
 * the signal copyRequested will be emitted for applications to connect to.
 */
class FLAKE_EXPORT KoCopyController : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param canvas the canvas this controller will work on.
     * @param copyAction the action that we will listen to and respond to when it is activated.  Additionally, the
     *     action is used as a parent for the QObject for memory management purposes.
     */
    KoCopyController(KoCanvasBase *canvas, QAction *copyAction);
    virtual ~KoCopyController();

signals:
    /// emitted when the user pressed copy and the current tool had no selection.
    void copyRequested();

public slots:
    /**
     * Notify whether the application has a selection.
     * The copy-action will only be enabled when either the current tool or the application has a selection.
     * @param selection if true the application is marked to allow copying.
     * @see copyRequested()
     */
    void hasSelection(bool selection);

private:
    Q_PRIVATE_SLOT(d, void copy())
    Q_PRIVATE_SLOT(d, void cut())
    Q_PRIVATE_SLOT(d, void selectionChanged(bool))

protected:
    friend class KoCopyControllerPrivate;
    KoCopyControllerPrivate * const d;
};

#endif
