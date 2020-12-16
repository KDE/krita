/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_WINDOW_H
#define LIBKIS_WINDOW_H

#include <QObject>
#include <QAction>
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

    /**
     * @brief createAction creates a QAction object and adds it to the action
     * manager for this Window.
     * @param id The unique id for the action. This will be used to
     *     propertize the action if any .action file is present
     * @param text The user-visible text of the action. If empty, the text from the
     *    .action file is used.
     * @param menuLocation a /-separated string that describes which menu the action should
     *     be places in. Default is "tools/scripts"
     * @return the new action.
     */
    QAction *createAction(const QString &id, const QString &text = QString(), const QString &menuLocation = QString("tools/scripts"));

Q_SIGNALS:
    /// Emitted when the window is closed.
    void windowClosed();

    ///  Emitted when we change the color theme
    void themeChanged();

    /// Emitted when the active view changes
    void activeViewChanged();

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_WINDOW_H
