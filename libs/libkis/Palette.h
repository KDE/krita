/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKIS_PALETTE_H
#define LIBKIS_PALETTE_H

#include <QObject>
#include <QList>

#include "kritalibkis_export.h"
#include "libkis.h"
#include "Resource.h"
#include "KoColorSet.h"
#include <Swatch.h>

class ManagedColor;


/**
 * @brief The Palette class
 * Palette is a resource object that stores organised color data.
 * It's purpose is to allow artists to save colors and store them.
 *
 * An example for printing all the palettes and the entries:
 *
 * @code
import sys
from krita import *

resources = Application.resources("palette")

for (k, v) in resources.items():
    print(k)
    palette = Palette(v)
    for x in range(palette.numberOfEntries()):
        entry = palette.colorSetEntryByIndex(x)
        print(x, entry.name(), entry.id(), entry.spotColor())
 * @endcode
 */

class KRITALIBKIS_EXPORT Palette : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Palette)

public:
    explicit Palette(Resource *resource, QObject *parent = 0);
    ~Palette() override;

    bool operator==(const Palette &other) const;
    bool operator!=(const Palette &other) const;

public Q_SLOTS:

    /**
     * @brief number of colors (swatches) in palette
     * NOTE: same as `colorsCountTotal()`
     *
     * @return total number of colors
     */
    int numberOfEntries() const;

    /**
     * @brief Palettes are defined in grids.
     * The number of column define grid width.
     * The number of rows will depend of columns and total number of entries.
     *
     * @return the number of columns this palette is set to use.
     */
    int columnCount();
    /**
     * @brief Palettes are defined in grids.
     * The number of column define grid width, this value can be defined.
     * The number of rows will depend of columns and total number of entries.
     *
     * @param columns Set the amount of columns this palette should use.
     */
    void setColumnCount(int columns);

    /**
     * @brief the comment or description associated with the palette.
     *
     * @return A string for which value contains the comment/description of palette.
     */
    QString comment();

    /**
     * @brief the comment or description associated with the palette.
     *
     * @param comment set the comment or description associated with the palette.
     */
    void setComment(QString comment);

    /**
     * @brief Palette content can be organized in groups.
     *
     * @return The list of group names (list of string). This list follow the order the groups are defined in palette.
     */
    QStringList groupNames() const;

    /**
     * @brief Palette content can be organized in groups.
     * This method allows to add a new group in palette.
     *
     * @param name The name of the new group to add.
     */
    void addGroup(QString name);

    /**
     * @brief Palette content can be organized in groups.
     * This method allows to remove an existing group from palette.
     *
     * @param name The name of the new group to remve.
     * @param keepColors whether or not to delete all the colors inside, or to move them to the default group.
     */
    void removeGroup(QString name, bool keepColors = true);

    /**
     * @brief number of colors (swatches) in palette
     * NOTE: same as `numberOfEntries()`
     *
     * @return total number of colors
     */
    int colorsCountTotal();

    /**
     * @brief colorSetEntryByIndex
     * get the colorsetEntry from the global index.
     *
     * DEPRECATED: use `entryByIndex()` instead
     *
     * @param index the global index
     * @return the colorset entry
     */
    Q_DECL_DEPRECATED Swatch *colorSetEntryByIndex(int index);

    /**
     * @brief get color (swatch) from the global index.
     *
     * @param index the global index
     * @return The Swatch color for given index.
     */
    Swatch *entryByIndex(int index);

    /**
     * @brief colorSetEntryFromGroup
     *
     * DEPRECATED: use `entryByIndexFromGroup()` instead
     *
     * @param index index in the group.
     * @param groupName the name of the group to get the color from.
     * @return the colorsetentry.
     */
    Q_DECL_DEPRECATED Swatch *colorSetEntryFromGroup(int index, const QString &groupName);

    /**
     * @brief get color (swatch) from the given group index.
     *
     * @param index index in the group.
     * @param groupName the name of the group to get the color from.
     * @return The Swatch color for given index within given group name.
     */
    Swatch *entryByIndexFromGroup(int index, const QString &groupName);

    /**
     * @brief add a color entry to a group.
     * Color is appended to the end.
     *
     * @param entry the entry
     * @param groupName the name of the group to add to.
     */
    void addEntry(Swatch entry, QString groupName = QString());

    /**
     * @brief remove from defined group a color entry designed by given index.
     *
     * @param index index in the group.
     * @param groupName the name of the group within which color have to be removed.
     */
    void removeEntry(int index, const QString &groupName);

    /**
     * @brief changeGroupName
     * change the group name.
     *
     * DEPRECATED: use `renameGroup()` instead
     *
     * @param oldGroupName the old groupname to change.
     * @param newGroupName the new name to change it into.
     * @return whether successful. Reasons for failure include not knowing have oldGroupName
     */
    Q_DECL_DEPRECATED void changeGroupName(QString oldGroupName, QString newGroupName);

    /**
     * @brief rename a group
     *
     * @param oldGroupName the old groupname to change.
     * @param newGroupName the new name to change it into.
     */
    void renameGroup(QString oldGroupName, QString newGroupName);

    /**
     * @brief Move the group `groupName` to position before group `groupNameInsertBefore`.
     *
     * @param groupName group to move.
     * @param groupNameInsertBefore reference group for which `groupName` have to be moved before.
     */
    void moveGroup(const QString &groupName, const QString &groupNameInsertBefore = QString());

    /**
     * @brief save the palette
     *
     * WARNING: this method does nothing and need to be implemented!
     *
     * @return always False
     */
    bool save();

private:
    friend class PaletteView;
    struct Private;
    Private *const d;

    /**
     * @brief colorSet
     * @return gives qa KoColorSet object back
     */
    KoColorSetSP colorSet();

};

#endif // LIBKIS_PALETTE_H
