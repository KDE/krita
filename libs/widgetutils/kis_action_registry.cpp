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
#include <utility>
#include <KSharedConfig>
#include <QGlobalStatic>
#include <QDomElement>
#include <kis_debug.h>
#include <tuple>
#include "kis_debug.h"
#include <KoResourcePaths.h>
#include <QFile>



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
    void loadActions();
    actionInfoItem actionInfo(QString name) {
        return actionInfoList.value(name, emptyActionInfo);
    };
};


Q_GLOBAL_STATIC(KisActionRegistry, s_instance);

KisActionRegistry *KisActionRegistry::instance()
{
    return s_instance;
};


KisActionRegistry::KisActionRegistry()
    : d(new KisActionRegistry::Private())
{
    d->cfg = KSharedConfig::openConfig("kritashortcutsrc");
    d->loadActions();
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

void KisActionRegistry::Private::loadActions()
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
