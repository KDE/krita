/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SCRIPT_MANAGER_H
#define KIS_SCRIPT_MANAGER_H

#include <QObject>

#include <kritaui_export.h>

class KisAction;

class KisActionManager;
class KisViewManager;
class KActionCollection;

/**
 * @brief The KisScriptManager class is responsible for adding scripts to the menu
 */
class KRITAUI_EXPORT KisScriptManager : public QObject
{
    Q_OBJECT
public:
    explicit KisScriptManager(KisViewManager * view);
    ~KisScriptManager() override;

    void setup(KActionCollection * ac, KisActionManager *actionManager);
    void updateGUI();

    void addAction(KisAction *action);

private:
    struct Private;
    Private * const d;
};

#endif // KIS_SCRIPT_MANAGER_H
