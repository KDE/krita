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

    // Use Krita debug logging categories instead of KDE's default qDebug() << "Unnamed action in definitions file " << actionDefinition;
                    }

                    else if (actionInfoList.contains(name)) {
                        qWarning() << "NOT COOL: Duplicated action name from xml data: " << name;
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
