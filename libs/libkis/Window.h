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
#include <QMainWindow>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KisMainWindow.h>
/**
 * Window represents one Krita mainwindow. A window can have any number
 * of views open on any number of documents.
 */
class KRITALIBKIS_EXPORT Window : public QObject
{
    Q_OBJECT

public:
    explicit Window(KisMainWindow *window, QObject *parent = 0);
    ~Window() override;

    bool operator==(const Window &other) const;
    bool operator!=(const Window &other) const;

public Q_SLOTS:

    /**
     * Return a handle to the QMainWindow widget. This is useful
     * to e.g. parent dialog boxes and message box.
     */
    QMainWindow *qwindow() const;

    /**
     * @return a list of open views in this window
     */
    QList<View*> views() const;

    /**
     * Open a new view on the given document in this window
     */
    View *addView(Document *document);

    /**
     * Make the given view active in this window. If the view
     * does not belong to this window, nothing happens.
     */
    void showView(View *view);


    /**
     * @return the currently active view or 0 if no view is active
     */
    View *activeView() const;

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
