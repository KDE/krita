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

#ifndef KIS_ACTION_REGISTRY_H
#define KIS_ACTION_REGISTRY_H

#include <QString>
#include <QKeySequence>
#include <QDomElement>
#include <QAction>
#include <QList>

#include "kritawidgetutils_export.h"


class KActionCollection;
class QDomElement;
class KConfigBase;
class KisShortcutsDialog;

/**
 * KisActionRegistry is intended to manage the global action configuration data
 * for Krita. The data come from four sources:
 * - .action files, containing static action configuration data in XML format,
 * - .rc configuration files, originally from XMLGUI and now in WidgetUtils,
 * - kritashortcutsrc, containing temporary shortcut configuration, and
 * - .shortcuts scheme files providing sets of default shortcuts, also from XMLGUI
 *
 * This class can be used as a factory by calling makeQAction. It can be used to
 * add standard properties such as default shortcuts and default tooltip to an
 * existing action with propertizeAction. If you have a custom action class
 * which needs to add other properties, you can use propertizeAction to add any
 * sort of data you wish to the .action configuration file.
 *
 * This class is also in charge of displaying the shortcut configuration dialog.
 * The interplay between this class, KActionCollection, KisShortcutsEditor and
 * so on can be complex, and is sometimes synchronized by file I/O by reading
 * and writing the configuration files mentioned above.
 *
 * It is a global static.  Grab an ::instance().
 */
class KRITAWIDGETUTILS_EXPORT KisActionRegistry : public QObject
{
    Q_OBJECT

public:
    static KisActionRegistry *instance();

    /**
     * @return true if the given action exists
     */
    bool hasAction(const QString &name) const;


    /**
     * @return value @p property for an action @p name.
     *
     * Allow flexible info structure for KisActions, etc.
     */
    QString getActionProperty(const QString &name, const QString &property);

    /**
     * Produces a new QAction based on the .action data files.
     *
     * N.B. this action will not be saved in the registry.
     */
    QAction *makeQAction(const QString &name, QObject *parent = 0);

    /**
     * Fills the standard QAction properties of an action.
     *
     * @return true if the action was loaded successfully.
     */
    bool propertizeAction(const QString &name, QAction *a);

    /**
     * Called when "OK" button is pressed in settings dialog.
     */
    void settingsPageSaved();

    /**
     * Reload custom shortcuts from kritashortcutsrc
     */
    void loadCustomShortcuts();

    /**
     * Call after settings are changed.
     */
    void notifySettingsUpdated();

    // If config == 0, reload defaults
    void applyShortcutScheme(const KConfigBase *config = 0);

    struct ActionCategory {
        ActionCategory();
        ActionCategory(const QString &_componentName, const QString &_categoryName);
        QString componentName;
        QString categoryName;

        bool isValid() const;

    private:
        bool m_isValid = false;
    };

    ActionCategory fetchActionCategory(const QString &name) const;

    /**
     * Constructor.  Please don't touch!
     */
    KisActionRegistry();
    ~KisActionRegistry();

    /**
     * @brief loadShortcutScheme
     * @param schemeName
     */
    void loadShortcutScheme(const QString &schemeName);

    // Undocumented
    void updateShortcut(const QString &name, QAction *ac);

    bool sanityCheckPropertized(const QString &name);

    QList<QString> registeredShortcutIds() const;

Q_SIGNALS:
    void shortcutsUpdated();

private:

    class Private;
    const QScopedPointer<Private> d;
};

#endif /* KIS_ACTION_REGISTRY_H */
