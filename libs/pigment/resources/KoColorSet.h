/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */
#ifndef KOCOLORSET
#define KOCOLORSET

#include <QObject>
#include <QColor>
#include <QVector>
#include <QScopedPointer>

#include <resources/KoResource.h>
#include "KoColor.h"
#include "KisSwatch.h"
#include "KisSwatchGroup.h"

/**
 * Also called palette.
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KRITAPIGMENT_EXPORT KoColorSet : public QObject, public KoResource
{
    Q_OBJECT
public:
    static const QString GLOBAL_GROUP_NAME;
    static const QString KPL_VERSION_ATTR;
    static const QString KPL_GROUP_ROW_COUNT_ATTR;
    static const QString KPL_PALETTE_COLUMN_COUNT_ATTR;
    static const QString KPL_PALETTE_NAME_ATTR;
    static const QString KPL_PALETTE_COMMENT_ATTR;
    static const QString KPL_PALETTE_FILENAME_ATTR;
    static const QString KPL_PALETTE_READONLY_ATTR;
    static const QString KPL_COLOR_MODEL_ID_ATTR;
    static const QString KPL_COLOR_DEPTH_ID_ATTR;
    static const QString KPL_GROUP_NAME_ATTR;
    static const QString KPL_SWATCH_ROW_ATTR;
    static const QString KPL_SWATCH_COL_ATTR;
    static const QString KPL_SWATCH_NAME_ATTR;
    static const QString KPL_SWATCH_SPOT_ATTR;
    static const QString KPL_SWATCH_ID_ATTR;
    static const QString KPL_SWATCH_BITDEPTH_ATTR;
    static const QString KPL_PALETTE_PROFILE_TAG;
    static const QString KPL_SWATCH_POS_TAG;
    static const QString KPL_SWATCH_TAG;
    static const QString KPL_GROUP_TAG;
    static const QString KPL_PALETTE_TAG;

public:
    enum PaletteType {
        UNKNOWN = 0,
        GPL,                // GIMP
        RIFF_PAL,           // RIFF
        ACT,                // Photoshop binary
        PSP_PAL,            // PaintShop Pro
        ACO,                // Photoshop Swatches
        XML,                // XML palette (Scribus)
        KPL,                // KoColor-based XML palette
        SBZ                 // SwatchBooker
    };


    /**
     * Load a color set from a file. This can be a Gimp
     * palette, a RIFF palette, a Photoshop palette,
     * a Krita palette,
     * a Scribus palette or a SwatchBooker palette.
     */
    explicit KoColorSet(const QString &filename = QString());

    // Explicit copy constructor (KoResource copy constructor is private)
    KoColorSet(const KoColorSet& rhs);

public /* overridden methods */: // KoResource
    ~KoColorSet() override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;
    QString defaultFileExtension() const override;


public /* methods */:
    void setColumnCount(int columns);
    int columnCount() const;

    void setComment(QString comment);
    QString comment();

    int rowCount() const;
    quint32 colorCount() const;

    PaletteType paletteType() const;
    void setPaletteType(PaletteType paletteType);

    /**
     * @brief isGlobal
     * A global color set is a set stored in the config directory
     * Such a color set would be opened every time Krita is launched.
     *
     * A non-global color set, on contrary, would be stored in a kra file,
     * and would only be opened when that file is opened by Krita.
     * @return @c true if the set is global
     */
    bool isGlobal() const;
    void setIsGlobal(bool);

    bool isEditable() const;
    void setIsEditable(bool isEditable);

    QByteArray toByteArray() const;
    bool fromByteArray(QByteArray &data);

    /**
     * @brief Add a color to the palette.
     * @param c the swatch
     * @param groupName color to add the group to. If empty, it will be added to the unsorted.
     */
    void add(const KisSwatch &, const QString &groupName = GLOBAL_GROUP_NAME);
    void setEntry(const KisSwatch &e, int x, int y, const QString &groupName = GLOBAL_GROUP_NAME);

    /**
     * @brief getColorGlobal
     * A function for getting a color based on a global index. Useful for iterating through all color entries.
     * @param x the global x index over the whole palette.
     * @param y the global y index over the whole palette.
     * @return the entry.
     */
    KisSwatch getColorGlobal(quint32 x, quint32 y) const;

    /**
     * @brief getColorGroup
     * A function for getting the color from a specific group.
     * @param x the x index over the group.
     * @param y the y index over the group.
     * @param groupName the name of the group, will give unsorted when not defined.
     * @return the entry
     */
    KisSwatch getColorGroup(quint32 x, quint32 y, QString groupName);

    /**
     * @brief getGroupNames
     * @return returns a list of group names, excluding the unsorted group.
     */
    QStringList getGroupNames();

    /**
     * @brief getGroup
     * @param name
     * @return the group with the name given; global group if no parameter is given
     * null pointer if not found.
     */
    KisSwatchGroup *getGroup(const QString &name);
    KisSwatchGroup *getGlobalGroup();

    bool changeGroupName(const QString &oldGroupName, const QString &newGroupName);


    /**
     * @brief addGroup
     * Adds a new group.
     * @param groupName the name of the new group. When not specified, this will fail.
     * @return whether thegroup was made.
     */
    bool addGroup(const QString &groupName);

    /**
     * @brief moveGroup
     * Move a group in the internal stringlist.
     * @param groupName the groupname to move.
     * @param groupNameInsertBefore the groupname to insert before. Empty means it will be added to the end.
     * @return
     */
    bool moveGroup(const QString &groupName, const QString &groupNameInsertBefore = GLOBAL_GROUP_NAME);
    /**
     * @brief removeGroup
     * Remove a group from the KoColorSet
     * @param groupName the name of the group you want to remove.
     * @param keepColors Whether you wish to keep the colorsetentries. These will be added to the unsorted.
     * @return whether it could find the group to remove.
     */
    bool removeGroup(const QString &groupName, bool keepColors = true);

    void clear();

    /**
     * @brief getIndexClosestColor
     * function that matches the color to all colors in the colorset, and returns the index
     * of the closest match.
     * @param compare the color you wish to compare.
     * @param useGivenColorSpace whether to use the color space of the color given
     * when the two colors' colorspaces don't match. Else it'll use the entry's colorspace.
     * @return returns the int of the closest match.
     */
    KisSwatchGroup::SwatchInfo getClosestColorInfo(KoColor compare, bool useGivenColorSpace = true);

private:
    class Private;
    const QScopedPointer<Private> d;

};
#endif // KOCOLORSET
