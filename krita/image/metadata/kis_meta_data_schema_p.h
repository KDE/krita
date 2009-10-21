/*
 *  Copyright (c) 2007,2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_schema.h"

#include <QHash>

class QDomElement;

namespace KisMetaData
{
struct Schema::Private {
    QString uri;
    QString prefix;
    struct EntryInfo {
        const TypeInfo* propertyType;
        QHash<QString, TypeInfo*> qualifiers;
    };
    QHash<QString, EntryInfo> types;
    QHash<QString, const TypeInfo*> structures;
    bool load(const QString&);
private:
    void parseStructures(QDomElement&);
    void parseStructure(QDomElement&);
    void parseProperties(QDomElement&);
    bool parseEltType(QDomElement&, EntryInfo& entryInfo, QString& name, bool ignoreStructure, bool ignoreName);
    const TypeInfo* parseAttType(QDomElement&, bool ignoreStructure);
    const TypeInfo* parseEmbType(QDomElement&, bool ignoreStructure);
    const TypeInfo* parseChoice(QDomElement&);
};
}
