/*
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */


#include "kis_tooltip_manager.h"

#include <QFile>
#include <QAction>
#include <QInputDialog>
#include <QDomDocument>
#include <QMenuBar>
#include <kactioncollection.h>
#include <kis_debug.h>

#include <KisMainWindow.h>
#include "KisViewManager.h"

KisTooltipManager::KisTooltipManager(KisViewManager* view) : QObject(view), m_view(view), m_recording(false)
{
    m_view->mainWindow()->menuBar()->installEventFilter(this);
}

KisTooltipManager::~KisTooltipManager()
{
    if (m_recording) {
        QFile f("tooltips.txt");
        f.open(QFile::WriteOnly);

        QDomDocument doc;
        QDomElement root;

        root = doc.createElement("tooltips");
        doc.appendChild(root);


        QMapIterator<QString, QString> it(m_tooltipMap);
        while (it.hasNext()) {
            it.next();
            QDomElement tooltip = doc.createElement("tooltip");
            tooltip.setAttribute("action", it.key());
            tooltip.appendChild(doc.createTextNode(it.value()));
            root.appendChild(tooltip);
        }
        QTextStream stream(&f);
        stream.setCodec("UTF-8");
        stream << doc.toString();

        f.close();
    }
}

void KisTooltipManager::record()
{
    m_recording = true;
    QList<QAction*> actions =  m_view->actionCollection()->actions();
    Q_FOREACH (KXMLGUIClient* client, m_view->mainWindow()->childClients() ) {
        actions.append(client->actionCollection()->actions());
    }

    Q_FOREACH (QAction* action, actions) {
        action->disconnect();
        connect(action, SIGNAL(triggered()), this, SLOT(captureToolip()));
    }
}

void KisTooltipManager::captureToolip()
{
    QString id = sender()->objectName();

    QString oldTooltip;
    if (m_tooltipMap.contains(id)) {
        oldTooltip = m_tooltipMap[id];
    }

    bool ok;
    QString tooltip = QInputDialog::getText(m_view->mainWindow(), "Add Tooltip",
                                            "New Tooltip:", QLineEdit::Normal,
                                            oldTooltip, &ok);
    if (ok && !tooltip.isEmpty()) {
        dynamic_cast<QAction*>(sender())->setToolTip(tooltip);
        m_tooltipMap[id] = tooltip;
    }
}


