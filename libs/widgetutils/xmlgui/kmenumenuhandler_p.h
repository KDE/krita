/* This file is part of the KDE project
   Copyright (C) 2006  Olivier Goffart  <ogoffart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kmenumenuhandler_p_h
#define kmenumenuhandler_p_h

#include <QObject>

class QAction;
class QMenu;

class KXMLGUIBuilder;
class KSelectAction;

namespace KDEPrivate
{

/**
 * @internal
 * This class handle the context menu of QMenu.
 * Used by KXmlGuiBuilder
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KMenuMenuHandler : public QObject
{
    Q_OBJECT
public:
    KMenuMenuHandler(KXMLGUIBuilder *b);
    ~KMenuMenuHandler() override {}
    void insertMenu(QMenu *menu);
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void slotSetShortcut();
    void buildToolbarAction();
    void slotAddToToolBar(int);

private:
    void showContextMenu(QMenu *menu, const QPoint &pos);

    KXMLGUIBuilder *m_builder;
    KSelectAction *m_toolbarAction;
    QMenu *m_popupMenu;
    QAction *m_popupAction;
    QMenu *m_contextMenu;

};

} //END namespace KDEPrivate

#endif
