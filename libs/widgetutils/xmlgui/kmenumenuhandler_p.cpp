/* This file is part of the KDE project
   Copyright (C) 2006  Olivier Goffart  <ogoffart@kde.org>

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

#include "kmenumenuhandler_p.h"

#include "kxmlguibuilder.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kactioncollection.h"
#include "kmainwindow.h"
#include "ktoolbar.h"
#include "kshortcutwidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <QDomDocument>
#include <QDomNode>
#include <QMenu>
#include <QVBoxLayout>
#include <QDebug>

#include <kselectaction.h>
#include <klocalizedstring.h>

namespace KDEPrivate
{

KMenuMenuHandler::KMenuMenuHandler(KXMLGUIBuilder *builder)
    : QObject(), m_builder(builder), m_popupMenu(0), m_popupAction(0), m_contextMenu(0)
{
    m_toolbarAction = new KSelectAction(i18n("Add to Toolbar"), this);
    connect(m_toolbarAction, SIGNAL(triggered(int)), this, SLOT(slotAddToToolBar(int)));
}

void KMenuMenuHandler::insertMenu(QMenu *popup)
{
    popup->installEventFilter(this);
}

bool KMenuMenuHandler::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (m_contextMenu && m_contextMenu->isVisible()) {
            m_contextMenu->hide();
            return true;
        }
        break;

    case QEvent::MouseButtonRelease:
        if (m_contextMenu && m_contextMenu->isVisible()) {
            return true;
        }
        break;

    case QEvent::ContextMenu: {
        QContextMenuEvent *e = static_cast<QContextMenuEvent *>(event);
        QMenu *menu = static_cast<QMenu *>(watched);
        if (e->reason() == QContextMenuEvent::Mouse) {
            showContextMenu(menu, e->pos());
        } else if (menu->activeAction()) {
            showContextMenu(menu, menu->actionGeometry(menu->activeAction()).center());
        }
    }
    event->accept();
    return true;

    default:
        break;
    }

    return false;
}

void KMenuMenuHandler::buildToolbarAction()
{
    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (!window) {
        return;
    }
    QStringList toolbarlist;
    foreach (KToolBar *b, window->toolBars()) {
        toolbarlist << (b->windowTitle().isEmpty() ? b->objectName() : b->windowTitle());
    }
    m_toolbarAction->setItems(toolbarlist);
}

static KActionCollection *findParentCollection(KXMLGUIFactory *factory, QAction *action)
{
    foreach (KXMLGUIClient *client, factory->clients()) {
        KActionCollection *collection = client->actionCollection();
        // if the call to actions() is too slow, add KActionCollection::contains(QAction*).
        if (collection->actions().contains(action)) {
            return collection;
        }
    }
    return 0;
}

void KMenuMenuHandler::slotSetShortcut()
{
    if (!m_popupMenu || !m_popupAction) {
        return;
    }

    QDialog dialog(m_builder->widget());
    dialog.setLayout(new QVBoxLayout);

    KShortcutWidget swidget(&dialog);
    swidget.setShortcut(m_popupAction->shortcuts());
    dialog.layout()->addWidget(&swidget);

    QDialogButtonBox box(&dialog);
    box.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&box, SIGNAL(rejected()), &dialog, SLOT(reject()));
    dialog.layout()->addWidget(&box);

    KActionCollection *parentCollection = 0;
    if (dynamic_cast<KXMLGUIClient *>(m_builder)) {
        QList<KActionCollection *> checkCollections;
        KXMLGUIFactory *factory = dynamic_cast<KXMLGUIClient *>(m_builder)->factory();
        parentCollection = findParentCollection(factory, m_popupAction);
        foreach (KXMLGUIClient *client, factory->clients()) {
            checkCollections += client->actionCollection();
        }
        swidget.setCheckActionCollections(checkCollections);
    }

    if (dialog.exec()) {
        m_popupAction->setShortcuts(swidget.shortcut());
        swidget.applyStealShortcut();
        if (parentCollection) {
            parentCollection->writeSettings();
        }
    }
}

void KMenuMenuHandler::slotAddToToolBar(int tb)
{
    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (!window) {
        return;
    }

    if (!m_popupMenu || !m_popupAction) {
        return;
    }

    KXMLGUIFactory *factory = dynamic_cast<KXMLGUIClient *>(m_builder)->factory();
    QString actionName = m_popupAction->objectName(); // set by KActionCollection::addAction
    KActionCollection *collection = 0;
    if (factory) {
        collection = findParentCollection(factory, m_popupAction);
    }
    if (!collection) {
        qWarning() << "Cannot find the action collection for action " << actionName;
        return;
    }

    KToolBar *toolbar = window->toolBars()[tb];
    toolbar->addAction(m_popupAction);

    const KXMLGUIClient *client = collection->parentGUIClient();
    QString xmlFile = client->localXMLFile();
    QDomDocument document;
    document.setContent(KXMLGUIFactory::readConfigFile(client->xmlFile(), client->componentName()));
    QDomElement elem = document.documentElement().toElement();

    const QLatin1String tagToolBar("ToolBar");
    const QLatin1String attrNoEdit("noEdit");
    const QLatin1String attrName("name");

    QDomElement toolbarElem;
    QDomNode n = elem.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QDomElement elem = n.toElement();
        if (!elem.isNull() && elem.tagName() == tagToolBar && elem.attribute(attrName) == toolbar->objectName()) {
            if (elem.attribute(attrNoEdit) == QStringLiteral("true")) {
                qWarning() << "The toolbar is not editable";
                return;
            }
            toolbarElem = elem;
            break;
        }
    }
    if (toolbarElem.isNull()) {
        toolbarElem = document.createElement(tagToolBar);
        toolbarElem.setAttribute(attrName, toolbar->objectName());
        elem.appendChild(toolbarElem);
    }

    KXMLGUIFactory::findActionByName(toolbarElem, actionName, true);
    KXMLGUIFactory::saveConfigFile(document, xmlFile);

}

void KMenuMenuHandler::showContextMenu(QMenu *menu, const QPoint &pos)
{
    Q_ASSERT(!m_popupMenu);
    Q_ASSERT(!m_popupAction);
    Q_ASSERT(!m_contextMenu);

    QAction *action = menu->actionAt(pos);
    if (!action || action->isSeparator()) {
        return;
    }

    m_popupMenu = menu;
    m_popupAction = action;

    m_contextMenu = new QMenu;
    m_contextMenu->addAction(i18n("Configure Shortcut..."), this, SLOT(slotSetShortcut()));

    KMainWindow *window = qobject_cast<KMainWindow *>(m_builder->widget());
    if (window) {
        m_contextMenu->addAction(m_toolbarAction);
        buildToolbarAction();
    }

    m_contextMenu->exec(menu->mapToGlobal(pos));
    delete m_contextMenu;
    m_contextMenu = 0;

    m_popupAction = 0;
    m_popupMenu = 0;
}

} //END namespace KDEPrivate

