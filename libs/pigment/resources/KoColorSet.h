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
#include "KoColorSetEntry.h"

/**
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KRITAPIGMENT_EXPORT KoColorSet : public QObject, public KoResource
{
    Q_OBJECT
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
    explicit KoColorSet(const QString &filename);

    /// Create an empty color set
    KoColorSet();

    /// Explicit copy constructor (KoResource copy constructor is private)
    KoColorSet(const KoColorSet& rhs);

    ~KoColorSet() override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    QString defaultFileExtension() const override;

    void setColumnCount(int columns);
    int columnCount();
    /**
     * @brief comment
     * @return the comment.
     */
    QString comment();

    void setComment(QString comment);

public:

    /**
     * @brief add Add a color to the palette.
     * @param groupName color to add the group to. If empty, it will be added to the unsorted.
     */
    void add(const KoColorSetEntry &, QString groupName = QString());

    /**
     * @brief insertBefore insert color before index into group.
     * @param index
     * @param groupName name of the group that the color goes into.
     * @return new index of index after the prepending.
     */
    quint32 insertBefore(const KoColorSetEntry &, qint32 index, const QString &groupName = QString());

    void removeAt(quint32 index, QString groupName = QString());

    /**
     * @brief getColorGlobal
     * A function for getting a color based on a global index. Useful for itterating through all color entries.
     * @param globalIndex the global index over the whole palette.
     * @return the entry.
     */
    KoColorSetEntry getColorGlobal(quint32 globalIndex);
    /**
     * @brief getColorGroup
     * A function for getting the color from a specific group.
     * @param groupName the name of the group, will give unosrted when not defined.
     * @param index the index within the group.
     * @return the entry
     */
    KoColorSetEntry getColorGroup(quint32 index, QString groupName = QString());

    QString findGroupByGlobalIndex(quint32 globalIndex, quint32 *index);
    QString findGroupByColorName(const QString &name, quint32 *index);
    QString findGroupByID(const QString &id,quint32 *index);

    /**
     * @brief getGroupNames
     * @return returns a list of group names, excluding the unsorted group.
     */
    QStringList getGroupNames();

    bool changeGroupName(QString oldGroupName, QString newGroupName);

    bool changeColorSetEntry(KoColorSetEntry entry, QString groupName, quint32 index);

    /**
     * @brief nColorsGroup
     * @param groupName string name of the group, when not specified, returns unsorted colors.
     * @return the amount of colors in this group.
     */
    quint32 nColorsGroup(QString groupName = QString());
    /**
     * @brief nColors
     * @return total colors in palette.
     */
    quint32 nColors();

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
    bool moveGroup(const QString &groupName, const QString &groupNameInsertBefore = QString());
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
     * @param color the color you wish to compare.
     * @param useGivenColorSpace whether to use the color space of the color given
     * when the two colors' colorspaces don't match. Else it'll use the entry's colorspace.
     * @return returns the int of the closest match.
     */
    quint32 getIndexClosestColor(KoColor color, bool useGivenColorSpace = true);

    /**
     * @brief closestColorName
     * convenience function to get the name of the closest match.
     * @param color
     * @param useGivenColorSpace
     * @return
     */
    QString closestColorName(KoColor color, bool useGivenColorSpace = true);

private:


    bool init();

    bool saveGpl(QIODevice *dev) const;
    bool loadGpl();

    bool loadAct();
    bool loadRiff();
    bool loadPsp();
    bool loadAco();
    bool loadXml();
    bool loadSbz();

    bool saveKpl(QIODevice *dev) const;
    bool loadKpl();



    struct Private;
    const QScopedPointer<Private> d;

};
#endif // KOCOLORSET

