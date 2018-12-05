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
#include <KConfigGroup>

#include "kis_debug.h"
#include "KoResourcePaths.h"
#include "kis_icon_utils.h"

#include "kis_action_registry.h"
#include "kshortcutschemeshelper_p.h"


namespace {

    /**
     * We associate several pieces of information with each shortcut. The first
     * piece of information is a QDomElement, containing the raw data from the
     * .action XML file. The second and third are QKeySequences, the first of
     * which is the default shortcut, the last of which is any custom shortcut.
     * The last two are the KActionCollection and KActionCategory used to
     * organize the shortcut editor.
     */
    struct ActionInfoItem {
        QDomElement  xmlData;

        QString      collectionName;
        QString      categoryName;

        inline QList<QKeySequence> defaultShortcuts() const {
            return m_defaultShortcuts;
        }

        inline void setDefaultShortcuts(const QList<QKeySequence> &value) {
            m_defaultShortcuts = value;
        }

        inline QList<QKeySequence> customShortcuts() const {
            return m_customShortcuts;
        }

        inline void setCustomShortcuts(const QList<QKeySequence> &value, bool explicitlyReset) {
            m_customShortcuts = value;
            m_explicitlyReset = explicitlyReset;
        }

        inline QList<QKeySequence> effectiveShortcuts() const {
            return m_customShortcuts.isEmpty() && !m_explicitlyReset ?
                m_defaultShortcuts : m_customShortcuts;
        }


    private:
        QList<QKeySequence> m_defaultShortcuts;
        QList<QKeySequence> m_customShortcuts;
        bool m_explicitlyReset = false;
    };

    // Convenience macros to extract text of a child node.
    QString getChildContent(QDomElement xml, QString node) {
        return xml.firstChildElement(node).text();
    }

    // Use Krita debug logging categories instead of KDE's default qDebug() for
    // harmless empty strings and translations
    QString quietlyTranslate(const QString &s) {
        if (s.isEmpty()) {
            return s;
        }
        QString translatedString = i18nc("action", s.toUtf8());
        if (translatedString == s) {
            translatedString = i18n(s.toUtf8());
        }
        if (translatedString.isEmpty()) {
            dbgAction << "No translation found for" << s;
            return s;
        }

        return translatedString;
    }
}



class Q_DECL_HIDDEN KisActionRegistry::Private
{
public:

    Private(KisActionRegistry *_q) : q(_q) {}

    // This is the main place containing ActionInfoItems.
    QMap<QString, ActionInfoItem> actionInfoList;
    void loadActionFiles();
    void loadCustomShortcuts(QString filename = QStringLiteral("kritashortcutsrc"));

    // XXX: this adds a default item for the given name to the list of actioninfo objects!
    ActionInfoItem &actionInfo(const QString &name) {
        if (!actionInfoList.contains(name)) {
            dbgAction << "Tried to look up info for unknown action" << name;
        }
        return actionInfoList[name];
    }

    KisActionRegistry *q;
    QSet<QString> sanityPropertizedShortcuts;
};


Q_GLOBAL_STATIC(KisActionRegistry, s_instance)

KisActionRegistry *KisActionRegistry::instance()
{
    if (!s_instance.exists()) {
        dbgRegistry << "initializing KoActionRegistry";
    }
    return s_instance;
}

bool KisActionRegistry::hasAction(const QString &name) const
{
    return d->actionInfoList.contains(name);
}


KisActionRegistry::KisActionRegistry()
    : d(new KisActionRegistry::Private(this))
{
    KConfigGroup cg = KSharedConfig::openConfig()->group("Shortcut Schemes");
    QString schemeName = cg.readEntry("Current Scheme", "Default");
    loadShortcutScheme(schemeName);
    loadCustomShortcuts();
}

KisActionRegistry::ActionCategory KisActionRegistry::fetchActionCategory(const QString &name) const
{
    if (!d->actionInfoList.contains(name)) return ActionCategory();

    const ActionInfoItem info = d->actionInfoList.value(name);
    return ActionCategory(info.collectionName, info.categoryName);
}

void KisActionRegistry::notifySettingsUpdated()
{
    d->loadCustomShortcuts();
}

void KisActionRegistry::loadCustomShortcuts()
{
    d->loadCustomShortcuts();
}

void KisActionRegistry::loadShortcutScheme(const QString &schemeName)
{
    // Load scheme file
    if (schemeName != QStringLiteral("Default")) {
        QString schemeFileName = KShortcutSchemesHelper::schemeFileLocations().value(schemeName);
        if (schemeFileName.isEmpty()) {
            return;
        }
        KConfig schemeConfig(schemeFileName, KConfig::SimpleConfig);
        applyShortcutScheme(&schemeConfig);
    } else {
        // Apply default scheme, updating KisActionRegistry data
        applyShortcutScheme();
    }
}

QAction * KisActionRegistry::makeQAction(const QString &name, QObject *parent)
{
    QAction * a = new QAction(parent);
    if (!d->actionInfoList.contains(name)) {
        qWarning() << "Warning: requested data for unknown action" << name;
        a->setObjectName(name);
        return a;
    }

    propertizeAction(name, a);
    return a;
}

void KisActionRegistry::settingsPageSaved()
{
   // For now, custom shortcuts are dealt with by writing to file and reloading.
   loadCustomShortcuts();

   // Announce UI should reload current shortcuts.
   emit shortcutsUpdated();
}


void KisActionRegistry::applyShortcutScheme(const KConfigBase *config)
{
    // First, update the things in KisActionRegistry
    d->actionInfoList.clear();
    d->loadActionFiles();

    if (config == 0) {
        // Use default shortcut scheme. Simplest just to reload everything.
        loadCustomShortcuts();
    } else {
        const auto schemeEntries = config->group(QStringLiteral("Shortcuts")).entryMap();
        // Load info item for each shortcut, reset custom shortcuts
        auto it = schemeEntries.constBegin();
        while (it != schemeEntries.end()) {
            ActionInfoItem &info = d->actionInfo(it.key());
            info.setDefaultShortcuts(QKeySequence::listFromString(it.value()));
            it++;
        }
    }
}

void KisActionRegistry::updateShortcut(const QString &name, QAction *action)
{
    const ActionInfoItem &info = d->actionInfo(name);
    action->setShortcuts(info.effectiveShortcuts());
    action->setProperty("defaultShortcuts", qVariantFromValue(info.defaultShortcuts()));

    d->sanityPropertizedShortcuts.insert(name);
}

bool KisActionRegistry::sanityCheckPropertized(const QString &name)
{
    return d->sanityPropertizedShortcuts.contains(name);
}

QList<QString> KisActionRegistry::registeredShortcutIds() const
{
    return d->actionInfoList.keys();
}

bool KisActionRegistry::propertizeAction(const QString &name, QAction * a)
{
    if (!d->actionInfoList.contains(name)) {
        warnAction << "No XML data found for action" << name;
        return false;
    }

    const ActionInfoItem info = d->actionInfo(name);

    QDomElement actionXml = info.xmlData;
    if (!actionXml.text().isEmpty()) {
        // i18n requires converting format from QString.
        auto getChildContent_i18n = [=](QString node){return quietlyTranslate(getChildContent(actionXml, node));};

        // Note: the fields in the .action documents marked for translation are determined by extractrc.
        QString icon      = getChildContent(actionXml, "icon");
        QString text      = getChildContent_i18n("text");
        QString whatsthis = getChildContent_i18n("whatsThis");
        QString toolTip   = getChildContent_i18n("toolTip");
        QString statusTip = getChildContent_i18n("statusTip");
        QString iconText  = getChildContent_i18n("iconText");
        bool isCheckable  = getChildContent(actionXml, "isCheckable") == QString("true");

        a->setObjectName(name); // This is helpful, should be added more places in Krita
        a->setIcon(KisIconUtils::loadIcon(icon.toLatin1()));
        a->setText(text);
        a->setObjectName(name);
        a->setWhatsThis(whatsthis);
        a->setToolTip(toolTip);
        a->setStatusTip(statusTip);
        a->setIconText(iconText);
        a->setCheckable(isCheckable);
    }

    updateShortcut(name, a);
    return true;
}



QString KisActionRegistry::getActionProperty(const QString &name, const QString &property)
{
    ActionInfoItem info = d->actionInfo(name);
    QDomElement actionXml = info.xmlData;
    if (actionXml.text().isEmpty()) {
        dbgAction << "No XML data found for action" << name;
        return QString();
    }

    return getChildContent(actionXml, property);

}


void KisActionRegistry::Private::loadActionFiles()
{
    QStringList actionDefinitions =
        KoResourcePaths::findAllResources("kis_actions", "*.action", KoResourcePaths::Recursive);

    // Extract actions all XML .action files.
    Q_FOREACH (const QString &actionDefinition, actionDefinitions)  {
        QDomDocument doc;
        QFile f(actionDefinition);
        f.open(QFile::ReadOnly);
        doc.setContent(f.readAll());

        QDomElement base       = doc.documentElement(); // "ActionCollection" outer group
        QString collectionName = base.attribute("name");
        QString version        = base.attribute("version");
        if (version != "2") {
            errAction << ".action XML file" << actionDefinition << "has incorrect version; skipping.";
            continue;
        }

        // Loop over <Actions> nodes. Each of these corresponds to a
        // KActionCategory, producing a group of actions in the shortcut dialog.
        QDomElement actions = base.firstChild().toElement();
        while (!actions.isNull()) {

            // <text> field
            QDomElement categoryTextNode = actions.firstChild().toElement();
            QString categoryName         = quietlyTranslate(categoryTextNode.text());

            // <action></action> tags
            QDomElement actionXml  = categoryTextNode.nextSiblingElement();

            // Loop over individual actions
            while (!actionXml.isNull()) {
                if (actionXml.tagName() == "Action") {
                    // Read name from format <Action name="save">
                    QString name      = actionXml.attribute("name");

                    // Bad things
                    if (name.isEmpty()) {
                        errAction << "Unnamed action in definitions file " << actionDefinition;
                    }

                    else if (actionInfoList.contains(name)) {
                        qWarning << "NOT COOL: Duplicated action name from xml data: " << name;
                    }

                    else {
                        ActionInfoItem info;
                        info.xmlData         = actionXml;

                        // Use empty list to signify no shortcut
                        QString shortcutText = getChildContent(actionXml, "shortcut");
                        if (!shortcutText.isEmpty()) {
                            info.setDefaultShortcuts(QKeySequence::listFromString(shortcutText));
                        }

                        info.categoryName    = categoryName;
                        info.collectionName  = collectionName;

                        actionInfoList.insert(name,info);
                    }
                }
                actionXml = actionXml.nextSiblingElement();
            }
            actions = actions.nextSiblingElement();
        }
    }
}

void KisActionRegistry::Private::loadCustomShortcuts(QString filename)
{
    const KConfigGroup localShortcuts(KSharedConfig::openConfig(filename),
                                      QStringLiteral("Shortcuts"));

    if (!localShortcuts.exists()) {
        return;
    }

    // Distinguish between two "null" states for custom shortcuts.
    for (auto i = actionInfoList.begin(); i != actionInfoList.end(); ++i) {
        if (localShortcuts.hasKey(i.key())) {
            QString entry = localShortcuts.readEntry(i.key(), QString());
            if (entry == QStringLiteral("none")) {
                i.value().setCustomShortcuts(QList<QKeySequence>(), true);
            } else {
                i.value().setCustomShortcuts(QKeySequence::listFromString(entry), false);
            }
        } else {
            i.value().setCustomShortcuts(QList<QKeySequence>(), false);
        }
    }
}

KisActionRegistry::ActionCategory::ActionCategory()
{
}

KisActionRegistry::ActionCategory::ActionCategory(const QString &_componentName, const QString &_categoryName)
    : componentName(_componentName),
      categoryName(_categoryName),
      m_isValid(true)
{
}

bool KisActionRegistry::ActionCategory::isValid() const
{
    return m_isValid && !categoryName.isEmpty() && !componentName.isEmpty();
}
