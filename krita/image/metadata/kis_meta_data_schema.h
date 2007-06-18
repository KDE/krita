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

#ifndef _KIS_META_DATA_SCHEMA_H_
#define _KIS_META_DATA_SCHEMA_H_

#include <krita_export.h>

class QString;

namespace KisMetaData {
    class SchemaRegistry;
    class KRITAIMAGE_EXPORT Schema {
            struct Private;
            friend class SchemaRegistry;
        public:
            static const QString TIFFSchemaUri;
            static const QString EXIFSchemaUri;
            static const QString DublinCoreSchemaUri;
            static const QString XMPSchemaUri;
            static const QString XMPRightsSchemaUri;
            static const QString MakerNoteSchemaUri;
            static const QString IPTCSchemaUri;
            static const QString PhotoshopSchemaUri;
        private:
            Schema(QString _uri, QString _ns);
        public:
            QString uri() const;
            QString prefix() const;
            QString generateQualifiedName(QString) const;
        private:
            Private* const d;
    };
    class KRITAIMAGE_EXPORT SchemaRegistry {
            struct Private;
            SchemaRegistry();
        public:
            /**
             * Creates a new schema.
             * @param uri the name of the schema
             * @param prefix the namespace prefix used for this schema
             * @return the schema associated with the uri (it can return 0, if no schema exist
             * for the uri, but the prefix was allready used, and it can be an allready existing
             * schema if the uri was allready included)
             */
            const KisMetaData::Schema* create(QString uri, QString prefix);
            /**
             * @return the schema for this uri
             */
            const Schema* schemaFromUri(QString uri) const;
            /**
             * @return the schema for this prefix
             */
            const Schema* schemaFromPrefix(QString prefix) const;
            /**
             * Return an instance of the SchemaRegistry.
             * Creates an instance if that has never happened before and returns
             * the singleton instance.
             * Intialize it with default schemas.
             */
            static KisMetaData::SchemaRegistry* instance();
        private:
            Private* const d;
    };
}

KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisMetaData::Schema &c);

#endif
