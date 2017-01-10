/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef LIBKIS_WINDOW_H
#define LIBKIS_WINDOW_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KisMainWindow.h>
/**
 * Window
 */
class KRITALIBKIS_EXPORT Window : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Window)

    Q_PROPERTY(QList<View*> Views READ views WRITE setViews)
    Q_PROPERTY(QString ViewMode READ viewMode WRITE setViewMode)

public:
    explicit Window(KisMainWindow *window, QObject *parent = 0);
    virtual ~Window();

    QList<View*> views() const;
    void setViews(QList<View*> value);

    QString viewMode() const;


    void setViewMode(QString value);

public Q_SLOTS:

    /**
     * @brief activate activates this Window.
     */
    void activate();

    /**
     * @brief close the active window and all its Views. If there
     * are no Views left for a given Document, that Document will
     * also be closed.
     */
    void close();

Q_SIGNALS:
    /// Emitted when the window is closed.
    void windowClosed();

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_WINDOW_H
