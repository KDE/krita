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

#ifndef _KIS_ENTRY_EDITOR_H_
#define _KIS_ENTRY_EDITOR_H_

#include <QObject>

class QString;

namespace KisMetaData
{
class Store;
}

class KisEntryEditor : public QObject
{
    Q_OBJECT
    struct Private;
public:
    KisEntryEditor(QObject* obj, KisMetaData::Store* store, QString key, QString propertyName, QString structField, int arrayIndex);
    ~KisEntryEditor();
public slots:
    void valueEdited();
    void valueChanged();
signals:
    void valueHasBeenEdited();
private:
    Private* const d;
};

#endif
