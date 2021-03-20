/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBARHANDLER_H
#define KTOOLBARHANDLER_H

#include <QLinkedList>
#include <QObject>
#include <QPointer>

#include <kxmlguiclient.h>

class KXmlGuiWindow;

namespace KDEPrivate
{

class ToolBarHandler : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Creates a new tool bar handler for the supplied
     * @p mainWindow.
     */
    explicit ToolBarHandler(KXmlGuiWindow *mainWindow);

    /**
     * Creates a new tool bar handler for the supplied
     * @p mainWindow and with the supplied @p parent.
     */
    ToolBarHandler(KXmlGuiWindow *mainWindow, QObject *parent);

    /**
     * Destroys the tool bar handler.
     */
    ~ToolBarHandler() override;

    /**
     * Returns the action which is responsible for the tool bar menu.
     */
    QAction *toolBarMenuAction();

public Q_SLOTS:
    void setupActions();

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void clientAdded(KXMLGUIClient *))
};

} // namespace KDEPrivate

#endif // KTOOLBARHANDLER_H
