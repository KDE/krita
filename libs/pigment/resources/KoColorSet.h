/*  This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef KOCOLORSET
#define KOCOLORSET

#include <QObject>
#include <QColor>
#include <QVector>
#include <QScopedPointer>
#include <QSharedPointer>

#include <KoResource.h>
#include "KoColor.h"
#include "KisSwatch.h"
#include "KisSwatchGroup.h"

class KUndo2Stack;

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
        SBZ,                // SwatchBooker
        ASE,                // Adobe swatch exchange
        ACB,                // Adobe Color Book.
        CSS                 // CSS palette
    };


    /**
     * Load a color set from a file. This can be a Gimp
     * palette, a RIFF palette, a Photoshop palette,
     * a Krita palette,
     * a Scribus palette or a SwatchBooker palette.
     */
    explicit KoColorSet(const QString &filename = QString());

    KoColorSet(const KoColorSet& rhs);

    ~KoColorSet() override;

    KoColorSet &operator=(const KoColorSet &rhs) = delete;

    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;
    QString defaultFileExtension() const override;
    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::Palettes, "");
    }

    KUndo2Stack *undoStack() const;

    void updateThumbnail() override;

    void setLocked(bool lock);
    bool isLocked() const;

    void setColumnCount(int columns);
    int columnCount() const;

    void setComment(QString comment);
    QString comment();

    // Total number of rows, including empty rows in the groups, excluding rows
    // a model might use to show group headings
    int rowCount() const;

    int rowCountWithTitles() const;

    quint32 colorCount() const;

    PaletteType paletteType() const;
    void setPaletteType(PaletteType paletteType);

    bool fromByteArray(QByteArray &data, KisResourcesInterfaceSP resourcesInterface);

    /**
     * @brief Add a color to the palette.
     * @param c the swatch
     * @param groupName color to add the group to. If empty, it will be added to the unsorted.
     * @param column. The column in the group
     * @param row. The row in the group
     */
    void addSwatch(const KisSwatch &swatch, const QString &groupName = GLOBAL_GROUP_NAME, int column = -1, int row = -1);

    /**
     * @brief remove the swatch from the given group at column and row
     */
    void removeSwatch(int column, int row, KisSwatchGroupSP group);

    /**
     * @brief getGroup
     * @param name
     * @return the group with the name given; global group if no parameter is given
     * null pointer if not found.
     */
    KisSwatchGroupSP getGroup(const QString &name) const;

    /**
     * @brief getGroup get the group that covers this row
     * @param row
     * @return a swatch group
     */
    KisSwatchGroupSP getGroup(int row) const;

    /**
     * @brief getGlobalGroup
     * @return
     */
    KisSwatchGroupSP getGlobalGroup() const;

    /**
     * @brief changeGroupName
     * @param oldGroupName
     * @param newGroupName
     */
    void changeGroupName(const QString &oldGroupName, const QString &newGroupName);

    /**
     * @brief addGroup
     * Adds a new group.
     * @param groupName the name of the new group. When not specified, this will fail.
     * @return whether the group was made.
     */
    void addGroup(const QString &groupName, int columnCount = KisSwatchGroup::DEFAULT_COLUMN_COUNT, int rowCount = KisSwatchGroup::DEFAULT_ROW_COUNT);

    /**
     * @brief moveGroup
     * Move a group in the internal stringlist.
     * @param groupName the groupname to move.
     * @param groupNameInsertBefore the groupname to insert before. Empty means it will be added to the end.
     * @return
     */
    void moveGroup(const QString &groupName, const QString &groupNameInsertBefore = GLOBAL_GROUP_NAME);

    /**
     * @brief removeGroup
     * Remove a group from the KoColorSet
     * @param groupName the name of the group you want to remove.
     * @param keepColors Whether you wish to keep the colorsetentries. These will be added to the unsorted.
     * @return whether it could find the group to remove.
     */
    void removeGroup(const QString &groupName, bool keepColors = true);

    /**
     * @brief clears the complete colorset
     */
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
    KisSwatchGroup::SwatchInfo getClosestSwatchInfo(KoColor compare, bool useGivenColorSpace = true) const;

    /**
     * @brief getColorGlobal
     * A function for getting a color based on a global index. Useful for iterating through all color entries.
     * @param x the global x index over the whole palette.
     * @param y the global y index over the whole palette.
     * @return the entry.
     */
    KisSwatch getColorGlobal(quint32 column, quint32 row) const;

    /**
     * @brief getColorGroup
     * A function for getting the color from a specific group.
     * @param x the x index over the group.
     * @param y the y index over the group.
     * @param groupName the name of the group, will give unsorted when not defined.
     * @return the entry
     */
    KisSwatch getSwatchFromGroup(quint32 column, quint32 row, QString groupName = KoColorSet::GLOBAL_GROUP_NAME) const;

    /**
     * @brief getGroupNames
     * @return returns a list of group names, excluding the unsorted group.
     */
    QStringList swatchGroupNames() const;

    /**
     * @brief isGroupRow checks whether the current row is a group title
     * @param row the row to check
     * @return true if this is a group row
     */
    bool isGroupTitleRow(int row) const;

    /**
     * @brief rowForNamedGroup returns the row the group's title is on
     * @param groupName
     * @return
     */
    int startRowForGroup(const QString &groupName) const;

    /**
     * @brief rowNumberInGroup calculates the row number in the group from the global rownumber
     * @param rowNumber this is a row in rowCountWithTitles
     * @return -1 if the row is a group title row.
     */
    int rowNumberInGroup(int rowNumber) const;

Q_SIGNALS:

    void modified();


private Q_SLOTS:

    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);


private:

    void setModified(bool);

    friend struct AddSwatchCommand;
    friend struct RemoveSwatchCommand;
    friend struct ChangeGroupNameCommand;
    friend struct AddGroupCommand;
    friend struct RemoveGroupCommand;
    friend struct ClearCommand;
    friend struct SetColumnCountCommand;
    friend struct SetCommentCommand;
    friend struct SetPaletteTypeCommand;
    friend struct MoveGroupCommand;
    friend class TestKoColorSet;
    friend class TestKisPaletteModel;

    class Private;
    const QScopedPointer<Private> d;

};

typedef QSharedPointer<KoColorSet> KoColorSetSP;

#endif // KOCOLORSET
