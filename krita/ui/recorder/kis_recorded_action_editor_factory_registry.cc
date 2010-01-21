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

#include "kis_recorded_action_editor_factory_registry.h"

#include <qalgorithms.h>
#include <QList>
#include <QWidget>

#include <kglobal.h>

#include <kis_debug.h>
#include "kis_recorded_action_editor_factory.h"
#include "kis_recorded_filter_action_editor.h"
#include "kis_recorded_paint_action_editor.h"

struct KisRecordedActionEditorFactoryRegistry::Private {
    QList< KisRecordedActionEditorFactory* > factories;
};

KisRecordedActionEditorFactoryRegistry::KisRecordedActionEditorFactoryRegistry()
        : d(new Private)
{
    add(new KisRecordedFilterActionEditorFactory);
    add(new KisRecordedPaintActionEditorFactory);
}

KisRecordedActionEditorFactoryRegistry::~KisRecordedActionEditorFactoryRegistry()
{
    dbgRegistry << "Deleting KisRecordedActionEditorFactoryRegistry";
    qDeleteAll(d->factories);
    delete d;
}

KisRecordedActionEditorFactoryRegistry* KisRecordedActionEditorFactoryRegistry::instance()
{
    K_GLOBAL_STATIC(KisRecordedActionEditorFactoryRegistry, s_instance);
    return s_instance;
}

void KisRecordedActionEditorFactoryRegistry::add(KisRecordedActionEditorFactory* factory)
{
    if (d->factories.contains(factory)) return;
    d->factories.push_front(factory);
}

QWidget* KisRecordedActionEditorFactoryRegistry::createEditor(QWidget* parent, KisRecordedAction* action) const
{
    foreach(KisRecordedActionEditorFactory* factory, d->factories) {
        if (factory->canEdit(action)) {
            QWidget* editor = factory->createEditor(parent, action);
            Q_ASSERT(editor);
            Q_ASSERT(editor->metaObject()->indexOfSignal("actionEdited()") != -1);
            return editor;
        }
    }
    return 0;
}

bool KisRecordedActionEditorFactoryRegistry::hasEditor(KisRecordedAction* action) const
{
    foreach(KisRecordedActionEditorFactory* factory, d->factories) {
        if (factory->canEdit(action)) {
            return true;
        }
    }
    return false;
}
