/*
 *  SPDX-FileCopyrightText: 2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_schema.h"

#include <QHash>

class QDomElement;

namespace KisMetaData
{
struct Q_DECL_HIDDEN Schema::Private {
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
