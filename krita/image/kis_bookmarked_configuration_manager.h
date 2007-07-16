/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_BOOKMARKED_CONFIGURATION_MANAGER_H_
#define _KIS_BOOKMARKED_CONFIGURATION_MANAGER_H_

#include <QList>

class KisSerializableConfiguration;
class KisSerializableConfigurationFactory;
class KoID;
class QString;

#include "krita_export.h"

class KRITAIMAGE_EXPORT KisBookmarkedConfigurationManager {
    public:
        static const KoID ConfigDefault;
        static const KoID ConfigLastUsed;
    public:
        /**
         * @param configEntryGroup name of the configuration entry with the
         * bookmarked configurations.
         */
        KisBookmarkedConfigurationManager(QString configEntryGroup, KisSerializableConfigurationFactory*);
        ~KisBookmarkedConfigurationManager();
        /**
         * Load the configuration.
         */
        KisSerializableConfiguration* load(QString configname) const;
        /**
         * Save the configuration.
         */
        void save(QString configname, const KisSerializableConfiguration*);
        /**
         * @return true if the configuration configname exist
         */
        bool exist(QString configname) const;
        /**
         * @return the list of the names of configurations.
         */
        QList<QString> configurations() const;
        /**
         * @return the default configuration
         */
        KisSerializableConfiguration* defaultConfiguration();
    private:
        QString configEntryGroup() const;
    private:
        struct Private;
        Private* const d;
};

#endif
