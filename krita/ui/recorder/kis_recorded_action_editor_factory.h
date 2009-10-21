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

#ifndef _KIS_RECORDED_ACTION_EDITOR_FACTORY_H_
#define _KIS_RECORDED_ACTION_EDITOR_FACTORY_H_

#include <krita_export.h>

class QWidget;
class KisRecordedAction;
class QString;

class KRITAUI_EXPORT KisRecordedActionEditorFactory
{
public:
    KisRecordedActionEditorFactory();
    virtual ~KisRecordedActionEditorFactory();
    /**
     * Create an editor for the action.
     * The widget is expected to have a 'actionChanged' signal that is emitted
     * when the editor has changed one of the parameter of the action.
     */
    virtual QWidget* createEditor(QWidget* parent, KisRecordedAction* action) const = 0;
    /**
     * @return true if this factory can create an editor for the given action.
     */
    virtual bool canEdit(const KisRecordedAction* action) const = 0;
private:
    struct Private;
    Private* const d;
};

#endif
