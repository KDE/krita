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
     * @return
     */
    bool isGlobal() const;
    void setIsGlobal(bool);

    QByteArray toByteArray() const;
    bool fromByteArray(QByteArray &data);

    /**
     * @brief add Add a color to the palette.
     * @param groupName color to add the group to. If empty, it will be added to the unsorted.
     */
    void add(const KisSwatch &, const QString &groupName = QString());
    void setEntry(const KisSwatch &e, int x, int y, const QString &groupName = QString());

    /**
     * @brief getColorGlobal
     * A function for getting a color based on a global index. Useful for itterating through all color entries.
     * @param globalIndex the global index over the whole palette.
     * @return the entry.
     */
    KisSwatch getColorGlobal(quint32 x, quint32 y) const;

    /**
     * @brief getColorGroup
     * A function for getting the color from a specific group.
     * @param groupName the name of the group, will give unosrted when not defined.
     * @param index the index within the group.
     * @return the entry
     */
    KisSwatch getColorGroup(quint32 x, quint32 y, QString groupName = QString());

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
    KisSwatchGroup *getGroup(const QString &name = QString());

    bool changeGroupName(QString oldGroupName, QString newGroupName);


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
     * @brief closestColorName
     * convenience function to get the name of the closest match.
     * @param color
     * @param useGivenColorSpace
     * @return
     */
    QString closestColorName(KoColor color, bool useGivenColorSpace = true);

    /*
     * No one seems to be using these methods...
     */
    // QString findGroupByGlobalIndex(quint32 x, quint32 y);
    // QString findGroupByColorName(const QString &name, quint32 *x, quint32 *y);
    // QString findGroupByID(const QString &id,quint32 *index);
    // void removeAt(quint32 x, quint32 y, QString groupName = QString());
    /**
     * @brief getGroupByName
     * @param groupName
     * @param if success
     * @return group found
     */
    // const KisSwatchGroup &getGroupByName(const QString &groupName, bool &success) const;
    // bool changeColorSetEntry(KisSwatch entry, QString groupName);
    /**
     * @brief getIndexClosestColor
     * function that matches the color to all colors in the colorset, and returns the index
     * of the closest match.
     * @param color the color you wish to compare.
     * @param useGivenColorSpace whether to use the color space of the color given
     * when the two colors' colorspaces don't match. Else it'll use the entry's colorspace.
     * @return returns the int of the closest match.
     */
    // quint32 getIndexClosestColor(KoColor color, bool useGivenColorSpace = true);
    /**
     * @brief insertBefore insert color before index into group.
     * @param index
     * @param groupName name of the group that the color goes into.
     * @return new index of index after the prepending.
     */
    // quint32 insertBefore(const KisSwatch &, qint32 index, const QString &groupName = QString());


private:
    class Private;
    const QScopedPointer<Private> d;

};
#endif // KOCOLORSET

