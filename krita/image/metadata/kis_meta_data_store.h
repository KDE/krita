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


#ifndef _KIS_META_DATA_STORE_H_
#define _KIS_META_DATA_STORE_H_

#include <krita_export.h>

#include <QHash>

namespace KisMetaData {
    class Schema;
    class Entry;
    /**
     * This class holds the list of metadata entries and schemas.
     */
    class KRITAIMAGE_EXPORT Store {
        struct Private;
        public:
            Store();
        public:
            /**
             * @return true if there is no metadata in this store.
             */
            bool empty() const;
            /**
             * Insert a new entry.
             * @param entry the new entry to insert in the metadata store, it must be a key which doesn't allready exist
             * @return false if the entry couldn't be included wether because the key allready
             *  exist
             */
            bool addEntry(const Entry& entry);
            /**
             * Give access to a metadata entry
             * @param entryKey the entryKey as the qualified name of the entry
             */
            Entry& getEntry(QString entryKey);
            /**
             * Give access to a metadata entry
             * @param uri the uri of the schema
             * @param entryName the name of the entry
             */
            Entry& getEntry(QString uri, QString entryName);
            
            QHash<QString, Entry>::const_iterator begin() const;
            QHash<QString, Entry>::const_iterator end() const;
            
            /**
             * @param entryKey the entryKey as the qualified name of the entry
             * @return true if an entry with the given key exist in the store
             */
            bool containsEntry(QString entryKey) const;
            /**
             * @param uri
             * @param entryName
             * @return true if an entry with the given uri and entry name exist in the store
             */
            bool containsEntry(QString uri, QString entryName) const;
            /**
             * @return the schema for this uri
             */
            const Schema* schemaFromUri(QString uri) const;
            /**
             * @return the schema for this prefix
             */
            const Schema* schemaFromPrefix(QString prefix) const;
            /**
             * Creates a new schema.
             * @param uri the name of the schema
             * @param prefix the namespace prefix used for this schema
             * @return the schema associated with the uri (it can return 0, if no schema exist
             * for the uri, but the prefix was allready used, and it can be an allready existing
             * schema if the uri was allready included)
             */
            const KisMetaData::Schema* createSchema(QString uri, QString prefix);
            const KisMetaData::Schema* createSchema(const KisMetaData::Schema*);
            /**
             * Dump on kdDebug the metadata store.
             */
            void debugDump() const;
        private:
            Private* const d;
    };
}

#endif
