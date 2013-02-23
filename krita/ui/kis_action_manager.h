/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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


#ifndef KIS_ACTION_MANAGER_H
#define KIS_ACTION_MANAGER_H

#include <krita_export.h>

class KActionCollection;
class KisView2;
class KisAction;

class KRITAUI_EXPORT KisActionManager
{

public:
    KisActionManager(KisView2* view);
    virtual ~KisActionManager();

    void addAction(const QString& name, KisAction* action, KActionCollection* actionCollection);
    void addAction(KisAction* action);
    void takeAction(KisAction* action);

    KisAction *actionByName(const QString &name) const;

    void updateGUI();
private:
    void dumpActionFlags();

    class Private;
    Private* const d;
};

#endif // KIS_ACTION_MANAGER_H
