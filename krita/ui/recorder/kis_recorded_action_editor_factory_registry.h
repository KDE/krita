/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_ACTION_EDITOR_FACTORY_REGISTRY_H_
#define _KIS_RECORDED_ACTION_EDITOR_FACTORY_REGISTRY_H_

#include <krita_export.h>

class QWidget;
class KisRecordedAction;
class KisRecordedActionEditorFactory;

/**
 * This class allow to create an editor for a specific recorded action.
 *
 * If two editors can edit the same type of action, then the editor that
 * was added last is used in priority.
 */
class KRITAUI_EXPORT KisRecordedActionEditorFactoryRegistry
{
private:
    KisRecordedActionEditorFactoryRegistry();
    ~KisRecordedActionEditorFactoryRegistry();
public:
    static KisRecordedActionEditorFactoryRegistry* instance();
    /**
     * Add a factory of action editor.
     */
    void add(KisRecordedActionEditorFactory* factory);
    /**
     * @return an editor for the given action, or a null pointer if there is
     *         no factory for that action.
     */
    QWidget* createEditor(QWidget* parent, KisRecordedAction* action) const;
    /**
     * @return true if there is an editor for this action.
     */
    bool hasEditor(KisRecordedAction* action) const;
private:
    struct Private;
    Private* const d;
};

#endif
