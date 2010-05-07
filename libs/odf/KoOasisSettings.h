/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
                      David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#ifndef KOOASISSETTINGS_H
#define KOOASISSETTINGS_H

#include <qdom.h>
#include "koodf_export.h"
#include <KoXmlReader.h>

/**
 * @brief Parse settings.xml file.
 *
 * This class helps parsing the settings.xml file of an OASIS document.
 *
 * For reference, the structure of settings.xml looks like:
 * <pre>
 *   \<office:settings\>
 *      \<config:config-item-set config:name="configure-settings"\>
 *      ....
 *      \</config:config-item-set\>
 *      \<config:config-item-set config:name="view-settings"\>
 *         \<config:config-item-map-indexed config:name="Views"\>
 *           \<config:config-item-map-entry\>
 *             \<config:config-item config:name="SnapLinesDrawing" config:type="string"\>value\</config:config-item\>
 *               ....
 *                \<config:config-item-map-named config:name="Tables"\>
 *                  \<config:config-item-map-entry config:name="Sheet1"\>
 *                    \<config:config-item config:name="CursorPositionX"\>
 *                    ......
 *                  \</config:config-item-map-entry\>
 *                  \<config:config-item-map-entry config:name="Sheet2"\>
 *                  ....
 *                  \</config:config-item-map-entry\>
 *                \</config:config-item-map-named\>
 *           .....
 *           \</config:config-item-map-entry\>
 *         \</config:config-item-map-indexed\>
 *         \<config:config-item-map-indexed config:name="Interface"\>
 *         .......
 *         \</config:config-item-map-indexed\>
 *      \</config:config-item-set\>
 *   \</office:settings\>
 * </pre>
 * Basically, an item-set is a set of named \<config-item\>s and/or maps.
 * There are two kinds of maps (by-index or by-name), and entries in the
 * maps contain \<config-item\>s too, or nested maps.
 *
 * The API of KoOasisSettings allows the caller to look for a given item-set
 * or item-map once, and then lookup multiple items inside it.
 * It also allows "drilling down" inside the tree in case of nesting.
 */
class KOODF_EXPORT KoOasisSettings
{
public:
    /**
     * Normal KoOasisSettings constructor, for an OASIS settings.xml
     */
    explicit KoOasisSettings(const KoXmlDocument &doc);

    /**
     * KoOasisSettings constructor for an OpenOffice-1.1 file
     */
    KoOasisSettings(const KoXmlDocument &doc, const char *officeNsUri, const char *configNsUri);

    ~KoOasisSettings();

    class Items;
    /**
     * Returns the toplevel item-set named @p itemSetName.
     * If not found, the returned items instance is null.
     */
    Items itemSet(const QString &itemSetName) const;

    class IndexedMap;
    class NamedMap;
    /// Represents a collection of items (config-item or maps).
    class KOODF_EXPORT Items
    {
        friend class KoOasisSettings;
        friend class IndexedMap;
        friend class NamedMap;
        Items(const KoXmlElement& elem, const KoOasisSettings* settings)
                : m_element(elem), m_settings(settings) {}
    public:
        bool isNull() const {
            return m_element.isNull();
        }

        /**
         * Look for the config-item-map-indexed named @p itemMapName and return it.
         *
         * An indexed map is an array (or sequence), i.e. items are supposed to
         * be retrieved by index. This is useful for e.g. "view 0", "view 1" etc.
         */
        IndexedMap indexedMap(const QString &itemMapName) const;

        /**
         * Look for the config-item-map-named named @p mapItemName and return it.
         *
         * A named map is a map where items are retrieved by entry name, @see selectItemMapEntry
         * @return false if no such map was found
         */
        NamedMap namedMap(const QString &itemMapName) const;

        int parseConfigItemInt(const QString &configName, int defaultValue = 0) const;
        qreal parseConfigItemDouble(const QString &configName, qreal defaultValue = 0) const;
        QString parseConfigItemString(const QString &configName, const QString &defaultValue = QString()) const;
        bool parseConfigItemBool(const QString &configName, bool defaultValue = false) const;
        short parseConfigItemShort(const QString &configName, short defaultValue = 0) const;
        long parseConfigItemLong(const QString &configName, long defaultValue = 0) const;
    private:
        /// @internal
        QString findConfigItem(const QString &item, bool *ok) const;
        /// @internal
        QString findConfigItem(const KoXmlElement &element, const QString &item, bool *ok) const;

        KoXmlElement m_element;
        const KoOasisSettings* m_settings;
    };

    /// Internal base class for IndexedMap and NamedMap
    class Map
    {
    public:
        bool isNull() const {
            return m_element.isNull();
        }
    protected:
        Map(const KoXmlElement &elem, const KoOasisSettings *settings)
                : m_element(elem), m_settings(settings) {}
        const KoXmlElement m_element;
        const KoOasisSettings *m_settings;
    };

    class KOODF_EXPORT IndexedMap : public Map
    {
        friend class Items;
        IndexedMap(const KoXmlElement& elem, const KoOasisSettings* settings)
                : Map(elem, settings) {}
      public:
        /// Returns an entry in an indexed map
        Items entry(int entryIndex) const;
    };

    class KOODF_EXPORT NamedMap : public Map
    {
        friend class Items;
        NamedMap(const KoXmlElement &elem, const KoOasisSettings *settings)
                : Map(elem, settings) {}
      public:
        /// Returns an entry in a named map
        Items entry(const QString& entryName) const;
    };

private:
    friend class Items;
    friend class IndexedMap;
    friend class NamedMap;
    const KoXmlElement m_settingsElement;
    const char *m_configNsUri;

    class Private;
    Private * const d;
};

#endif
