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

/**
 * KisShortcutRegistry is intended to manage the global shortcut configuration
 * for Krita. It is intended to provide the user's choice of shortcuts
 * the .action files, the configuration files that were done with XMLGUI, and
 * the
 *
 * It is a global static.  Grab an ::instance.
 */
class KRITAWIDGETUTILS_EXPORT KisActionRegistry : public QObject
{
    Q_OBJECT

public:
    static KisActionRegistry *instance();


    /**
     * Get shortcut for an action
     */
    QKeySequence getPreferredShortcut(QString name);

    /**
     * Get shortcut for an action
     */
    QKeySequence getDefaultShortcut(QString name);

    /**
     * Get custom shortcut for an action
     */
    QKeySequence getCustomShortcut(QString name);

    /**
     * @return DOM info for an action @a name.
     *
     * Allows somewhat flexible info structure for KisActions, QActions,
     * whatever else we decide on doing later.
     */
    QDomElement getActionXml(QString name);


    /**
     * Saves action in a category. Note that this grabs ownership of the action.
     */
    void addAction(QString name, QAction *a, QString category = "Krita");


    /**
     * Produces a new QAction based on the .action data files.
     */
    QAction * makeQAction(QString name, QObject *parent, QString category = QString());

    KActionCollection * getDefaultCollection();

    /**
     * Fills the standard QAction properties of an action.
     *
     * @return true if the action was loaded successfully.
     */
    bool propertizeAction(QString name, QAction *a);


    /**
     * @return list of actions with data available.
     */
    QStringList allActions();

    /**
     * Save settings. Not implemented yet.
     */
    // void writeSettings(KActionCollection *ac);


    /**
     * Run shortcuts dialog.
     */
    void configureShortcuts(KActionCollection *ac);

    /**
     * Constructor.  Please don't touch!
     */
    KisActionRegistry();

private:
    class Private;
    Private * const d;
};

