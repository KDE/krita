/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BOOKMARKED_CONFIGURATION_MANAGER_H_
#define _KIS_BOOKMARKED_CONFIGURATION_MANAGER_H_

#include <QList>
#include "kis_serializable_configuration.h"

class QString;
class KLocalizedString;

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisBookmarkedConfigurationManager
{
public:
    static const char ConfigDefault[];
    static const char ConfigLastUsed[];
public:
    /**
     * @param configEntryGroup name of the configuration entry with the
     * bookmarked configurations.
     */
    KisBookmarkedConfigurationManager(const QString & configEntryGroup, KisSerializableConfigurationFactory*);
    ~KisBookmarkedConfigurationManager();
    /**
     * Load the configuration.
     */
    KisSerializableConfigurationSP load(const QString & configname) const;
    /**
     * Save the configuration.
     */
    void save(const QString & configname, const KisSerializableConfigurationSP);
    /**
     * @return true if the configuration configname exists
     */
    bool exists(const QString & configname) const;
    /**
     * @return the list of the names of configurations.
     */
    QList<QString> configurations() const;
    /**
     * @return the default configuration
     */
    KisSerializableConfigurationSP defaultConfiguration() const;
    /**
     * Remove a bookmarked configuration
     */
    void remove(const QString & name);
    /**
     * Generate an unique name, for instance when the user is creating a new
     * entry.
     * @param base the base of the new name, including a "%1" for incrementing
     *      the number, for instance : "New Configuration %1", then this function
     *      will return the string where %1 will be replaced by the lowest number
     *      and be inexistant in the lists of configuration
     */
    QString uniqueName(const KLocalizedString & base);



private:
    QString configEntryGroup() const;
private:
    struct Private;
    Private* const d;
};

#endif
