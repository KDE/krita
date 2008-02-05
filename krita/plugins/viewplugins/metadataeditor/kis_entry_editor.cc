/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_entry_editor.h"

#include <QString>
#include <QVariant>

#include "kis_meta_data_value.h"

struct KisEntryEditor::Private {
    QObject* object;
    QString propertyName;
    KisMetaData::Value* value;
};

KisEntryEditor::KisEntryEditor(QObject* obj, KisMetaData::Value* v, QString propertyName) : d(new Private)
{
    Q_ASSERT(obj);
    Q_ASSERT(v);
    d->object = obj;
    d->propertyName = propertyName;
    d->value = v;
    valueChanged();
}

KisEntryEditor::~KisEntryEditor()
{
    delete d;
}

void KisEntryEditor::valueChanged()
{
    bool blocked = d->object->blockSignals(true);
    d->object->setProperty(d->propertyName.toAscii(), d->value->asVariant());
    d->object->blockSignals(blocked);
}
        
void KisEntryEditor::valueEdited()
{
    d->value->setVariant( d->object->property(d->propertyName.toAscii()) );
    emit valueHasBeenEdited();
}

#include "kis_entry_editor.moc"
