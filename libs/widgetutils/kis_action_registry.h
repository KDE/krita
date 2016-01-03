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
#include <QKeySequence>
#include <QDomElement>
#include <QAction>

#include "kritawidgetutils_export.h"


class KActionCollection;
class QDomElement;
class KConfigBase;

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
     * Get shortcut for an action
     */
    QKeySequence getPreferredShortcut(const QString &name);

    /**
     * Get shortcut for an action
     */
    QKeySequence getDefaultShortcut(const QString &name);

    /**
     * Get custom shortcut for an action
     */
    QKeySequence getCustomShortcut(const QString &name);


    /**
     * Get category name
     */
    QKeySequence getCategory(const QString &name);

    /**
     * @return value @p property for an action @p name.
     *
     * Allow flexible info structure for KisActions, etc.
     */
    QString getActionProperty(const QString &name, const QString &property);


    /**
     * Saves action in a category. Note that this grabs ownership of the action.
     */
    void addAction(const QString &name, QAction *a);


    /**
     * Produces a new QAction based on the .action data files.
     *
     * N.B. this action will not be saved in the registry.
     */
    QAction * makeQAction(const QString &name, QObject *parent);

    /**
     * Fills the standard QAction properties of an action.
     *
     * @return true if the action was loaded successfully.
     */
    bool propertizeAction(const QString &name, QAction *a);


    /**
     * @return list of actions with data available.
     */
    QStringList allActions();

    /**
     * Display the shortcut configuration dialog.
     */
    void configureShortcuts();


    /**
     * Reload custom shortcuts from kritashortcutsrc
     */
    void loadCustomShortcuts(const QString &path = QString());


    /**
     * Write custom shortcuts to a specific file
     */
    void writeCustomShortcuts(KConfigBase *config) const;


    /**
     * Call after settings are changed.
     */
    void notifySettingsUpdated();

    /**
     * Constructor.  Please don't touch!
     */
    KisActionRegistry();

    // Undocumented
    void updateShortcut(const QString &name, QAction *ac);
    KActionCollection * getDefaultCollection();


    void loadShortcutScheme(const QString &schemeName);
    // If config == 0, reload defaults
    void applyShortcutScheme(const KConfigBase *config = 0);

Q_SIGNALS:
    void shortcutsUpdated();

private:
    class Private;
    Private * const d;
};
