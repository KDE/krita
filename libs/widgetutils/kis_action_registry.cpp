/*
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <QString>
#include <QHash>
#include <QGlobalStatic>
#include <QFile>
#include <QDomElement>
#include <KSharedConfig>
#include <klocalizedstring.h>
#include <KisShortcutsDialog.h>

#include "kis_debug.h"
#include "KoResourcePaths.h"
#include "kis_icon_utils.h"
#include "kactioncollection.h"


#include "kis_action_registry.h"


namespace {

    struct actionInfoItem {
        QDomElement xmlData;
        QKeySequence defaultShortcut;
        QKeySequence customShortcut;
    };

    QKeySequence getShortcutFromXml(QDomElement node) {
        return node.firstChildElement("shortcut").text();
    };
    actionInfoItem emptyActionInfo;  // Used as default return value
};



class Q_DECL_HIDDEN KisActionRegistry::Private
{
public:

    Private(KisActionRegistry *_q) : q(_q) {};

    /**
     * We associate three pieces of information with each shortcut name. The
     * first piece of information is a QDomElement, containing the raw data from
     * the .action XML file. The second and third are QKeySequences, the first
     * of which is the default shortcut, the last of which is any custom
     * shortcut.
     *
     * QHash is most efficient as long as the action name keys are kept relatively short.
     */
    QHash<QString, actionInfoItem> actionInfoList;
    KSharedConfigPtr cfg{0};
    void loadActionFiles();
    void loadActionCollections();
    actionInfoItem actionInfo(QString name) {
        return actionInfoList.value(name, emptyActionInfo);
    };

    KisActionRegistry *q;
    KActionCollection * defaultActionCollection;
    QHash<QString, KActionCollection*> actionCollections;
};


Q_GLOBAL_STATIC(KisActionRegistry, s_instance);

KisActionRegistry *KisActionRegistry::instance()
{
    return s_instance;
};


KisActionRegistry::KisActionRegistry()
    : d(new KisActionRegistry::Private(this))
{
    d->cfg = KSharedConfig::openConfig("krbitashortcutsrc");
    d->loadActionFiles();

    // Should change to , then translate
    d->defaultActionCollection = new KActionCollection(this, "Krita");
    d->actionCollections.insert("Krita", d->defaultActionCollection);

}

// No this isn't the most efficient logic, but it's nice and readable.
QKeySequence KisActionRegistry::getPreferredShortcut(QString name)
{
    QKeySequence customShortcut = getCustomShortcut(name);

    if (customShortcut.isEmpty()) {
        return getDefaultShortcut(name);
    } else {
        return getCustomShortcut(name);
    }
};

QKeySequence KisActionRegistry::getDefaultShortcut(QString name)
{
    return d->actionInfo(name).defaultShortcut;
};

QKeySequence KisActionRegistry::getCustomShortcut(QString name)
{
    return d->actionInfo(name).customShortcut;
};


QStringList KisActionRegistry::allActions()
{
    return d->actionInfoList.keys();
};

QDomElement KisActionRegistry::getActionXml(QString name)
{
    return d->actionInfo(name).xmlData;
};

KActionCollection * KisActionRegistry::getDefaultCollection()
{
    return d->defaultActionCollection;
};

void KisActionRegistry::addAction(QString name, QAction *a, QString category)
{
    KActionCollection *ac;
    if (d->actionCollections.contains(category)) {
        ac = d->actionCollections.value(category);
    } else {
        ac = new KActionCollection(this, category);
        d->actionCollections.insert(category, ac);
        dbgAction << "Adding a new KActionCollection - " << category;
    }

    if (!ac->action(name)) {
        ac->addAction(name, a);
    }
    else {
        dbgAction << "duplicate action" << name << a << "in collection" << ac->componentName();
    }

    // TODO: look into the loading/saving mechanism
    ac->readSettings();
};



QAction * KisActionRegistry::makeQAction(QString name, QObject *parent, QString category)
{

    QAction * a = new QAction(parent);
    if (!d->actionInfoList.contains(name)) {
        dbgAction << "Warning: requested data for unknown action" << name;
        return a;
    }

    propertizeAction(name, a);
    addAction(name, a, category);

    return a;
};


void KisActionRegistry::configureShortcuts(KActionCollection *ac)
{

    KisShortcutsDialog dlg;
    dlg.addCollection(ac);
    for (auto i = d->actionCollections.constBegin(); i != d->actionCollections.constEnd(); i++ ) {
        dlg.addCollection(i.value(), i.key());
    }

   dlg.configure();  // Show the dialog.
}



bool KisActionRegistry::propertizeAction(QString name, QAction * a)
{

    QStringList actionNames = allActions();
    QDomElement actionXml = getActionXml(name);


    // Convenience macros to extract text of a child node.
    auto getChildContent      = [=](QString node){return actionXml.firstChildElement(node).text();};
    // i18n requires converting format from QString.
    auto getChildContent_i18n = [=](QString node) {
        if (getChildContent(node).isEmpty()) {
            dbgAction << "Found empty string to translate for property" << node;
            return QString();
        }
        return i18n(getChildContent(node).toUtf8().constData());
    };


    QString icon      = getChildContent("icon");
    QString text      = getChildContent("text");

    // Note: these fields in the .action definitions are marked for translation.
    QString whatsthis = getChildContent_i18n("whatsThis");
    QString toolTip   = getChildContent_i18n("toolTip");
    QString statusTip = getChildContent_i18n("statusTip");
    QString iconText  = getChildContent_i18n("iconText");

    bool isCheckable             = getChildContent("isCheckable") == QString("true");
    QKeySequence shortcut        = QKeySequence(getChildContent("shortcut"));
    QKeySequence defaultShortcut = QKeySequence(getChildContent("defaultShortcut"));


    a->setObjectName(name); // This is helpful!!
    a->setIcon(KisIconUtils::loadIcon(icon.toLatin1()));
    a->setText(text);
    a->setObjectName(name);
    a->setWhatsThis(whatsthis);
    a->setToolTip(toolTip);
    a->setStatusTip(statusTip);
    a->setIconText(iconText);
    a->setShortcut(shortcut);
    a->setCheckable(isCheckable);


    // XXX: this totally duplicates KisAction::setDefaultShortcut
    QList<QKeySequence> listifiedShortcut;
    listifiedShortcut.append(shortcut);
    setProperty("defaultShortcuts", qVariantFromValue(listifiedShortcut));




    // TODO: check for colliding shortcuts, or make sure it happens smartly inside kactioncollection
    //
    // Ultimately we want to have more than one KActionCollection, so we can
    // have things like Ctrl+I be italics in the text editor widget, while not
    // complaining about conflicts elsewhere. Right now, we use only one
    // collection, and we don't make things like the text editor configurable,
    // so duplicate shortcuts are handled mostly automatically by the shortcut
    // editor.
    //
    // QMap<QKeySequence, QAction*> existingShortcuts;
    // foreach(QAction* action, actionCollection->actions()) {
    //     if(action->shortcut() == QKeySequence(0)) {
    //         continue;
    //     }
    //     if (existingShortcuts.contains(action->shortcut())) {
    //         dbgAction << QString("Actions %1 and %2 have the same shortcut: %3") \
    //             .arg(action->text())                                             \
    //             .arg(existingShortcuts[action->shortcut()]->text())              \
    //             .arg(action->shortcut());
    //     }
    //     else {
    //         existingShortcuts[action->shortcut()] = action;
    //     }
    // }


    return true;
}





void KisActionRegistry::Private::loadActionFiles()
{

    KoResourcePaths::addResourceType("kis_actions", "data", "krita/actions");
    auto searchType = KoResourcePaths::Recursive | KoResourcePaths::NoDuplicates;
    QStringList actionDefinitions =
        KoResourcePaths::findAllResources("kis_actions", "*.action", searchType);

    // Extract actions all XML .action files.
    foreach(const QString &actionDefinition, actionDefinitions)  {
        QDomDocument doc;
        QFile f(actionDefinition);
        f.open(QFile::ReadOnly);
        doc.setContent(f.readAll());

        QDomElement actions    = doc.documentElement(); // Whole document
        QString collection     = actions.attribute("name");
        QDomElement actionXml  = actions.firstChild().toElement(); // Single Action

        while (!actionXml.isNull()) {
            if (actionXml.tagName() == "Action") {
                // Read name from format <Action name="save">
                QString name      = actionXml.attribute("name");

                // Very bad things
                if (name.isEmpty()) {
                    errAction << "Unnamed action in definitions file " << actionDefinition;
                    continue;
                }

                if (actionInfoList.contains(name)) {
                    errAction << "Warning: Duplicated action name: " << name;
                }

                actionInfoItem info;
                info.xmlData = actionXml;
                info.defaultShortcut = getShortcutFromXml(actionXml);
                info.customShortcut = info.defaultShortcut;                 //TODO: Read from KisConfig


                dbgAction << "default shortcut for" << name << " - " << info.defaultShortcut;

                actionInfoList.insert(name,info);
            }
            actionXml = actionXml.nextSiblingElement();
        }
    }


};
