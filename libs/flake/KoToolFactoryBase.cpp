/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolFactoryBase.h"

#include "KoToolBase.h"
#include <kactioncollection.h>

#include <kis_action_registry.h>
#include <KoToolManager.h>

#include <QKeySequence>
#include <QAction>
#include <QDebug>

class Q_DECL_HIDDEN KoToolFactoryBase::Private
{
public:
    Private(const QString &i)
        : priority(100),
          id(i)
    {
    }
    int priority;
    QString section;
    QString tooltip;
    QString activationId;
    QString iconName;
    const QString id;
    QKeySequence shortcut;
};


KoToolFactoryBase::KoToolFactoryBase(const QString &id)
    : d(new Private(id))
{
}

KoToolFactoryBase::~KoToolFactoryBase()
{
    delete d;
}

QList<QAction *> KoToolFactoryBase::createActions(KActionCollection *actionCollection)
{
    QList<QAction *> toolActions;

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction*> actions = createActionsImpl();
    QAction *action = actionRegistry->makeQAction(id());
    actionCollection->addAction(id(), action);
    connect(action, SIGNAL(triggered()), SLOT(activateTool()));
    //qDebug() << action << action->shortcut();


    Q_FOREACH(QAction *action, actions) {
        if (action->objectName().isEmpty()) {
            qWarning() << "Tool" << id() << "tries to add an action without a name";
            continue;
        }
        QAction *existingAction = actionCollection->action(action->objectName());
        if (existingAction) {
            delete action;
            action = existingAction;
        }

        QStringList tools;
        if (action->property("tool_action").isValid()) {
            tools = action->property("tool_action").toStringList();
        }
        tools << id();
        action->setProperty("tool_action", tools);
        if (!existingAction) {
            actionCollection->addAction(action->objectName(), action);
        }
        toolActions << action;
    }

    // Enable this to easily generate action files for tools
 #if 0
    if (toolActions.size() > 0) {

        QDomDocument doc;
        QDomElement e = doc.createElement("Actions");
        e.setAttribute("name", id);
        e.setAttribute("version", "2");
        doc.appendChild(e);

        Q_FOREACH (QAction *action, toolActions) {
            QDomElement a = doc.createElement("Action");
            a.setAttribute("name", action->objectName());

            // But seriously, XML is the worst format ever designed
            auto addElement = [&](QString title, QString content) {
                QDomElement newNode = doc.createElement(title);
                QDomText    newText = doc.createTextNode(content);
                newNode.appendChild(newText);
                a.appendChild(newNode);
            };

            addElement("icon", action->icon().name());
            addElement("text", action->text());
            addElement("whatsThis" , action->whatsThis());
            addElement("toolTip" , action->toolTip());
            addElement("iconText" , action->iconText());
            addElement("shortcut" , action->shortcut().toString());
            addElement("isCheckable" , QString((action->isChecked() ? "true" : "false")));
            addElement("statusTip", action->statusTip());
            e.appendChild(a);
        }
        QFile f(id()z + ".action");
        f.open(QFile::WriteOnly);
        f.write(doc.toString().toUtf8());
        f.close();

    }

    else {
        debugFlake << "Tool" << id() << "has no actions";
    }
#endif

//    qDebug() << "Generated actions for tool factory" << id();
//    Q_FOREACH(QAction *action, toolActions) {
//        qDebug() << "\taction:" << action->objectName() << "shortcut" << action->shortcuts() << "tools" << action->property("tool_action").toStringList();
//    }
    return toolActions;
}

QString KoToolFactoryBase::id() const
{
    return d->id;
}

int KoToolFactoryBase::priority() const
{
    return d->priority;
}

QString KoToolFactoryBase::section() const
{
    return d->section;
}

QString KoToolFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KoToolFactoryBase::iconName() const
{
    return d->iconName;
}

QString KoToolFactoryBase::activationShapeId() const
{
    return d->activationId;
}

QKeySequence KoToolFactoryBase::shortcut() const
{
    return d->shortcut;
}

void KoToolFactoryBase::setActivationShapeId(const QString &activationShapeId)
{
    d->activationId = activationShapeId;
}

void KoToolFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoToolFactoryBase::setSection(const QString & section)
{
    d->section = section;
}

void KoToolFactoryBase::setIconName(const char *iconName)
{
    d->iconName = QLatin1String(iconName);
}

void KoToolFactoryBase::setIconName(const QString &iconName)
{
    d->iconName = iconName;
}

void KoToolFactoryBase::setPriority(int newPriority)
{
    d->priority = newPriority;
}

void KoToolFactoryBase::setShortcut(const QKeySequence &shortcut)
{
    d->shortcut = shortcut;
}

QList<QAction *> KoToolFactoryBase::createActionsImpl()
{
    return QList<QAction *>();
}

void KoToolFactoryBase::activateTool()
{
    KoToolManager::instance()->switchToolRequested(sender()->objectName());
}

