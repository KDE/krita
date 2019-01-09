/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef KIS_PALETTEMODEL_H
#define KIS_PALETTEMODEL_H

#include <QPointer>
#include <QModelIndex>
#include <QMap>

#include <KoColorDisplayRendererInterface.h>

#include "kritawidgets_export.h"
#include <KoColorSet.h>
#include <QScopedPointer>

class KoColorSet;
class KisPaletteView;

/**
 * @brief The KisPaletteModel class
 * This, together with KisPaletteView and KisPaletteDelegate forms a mvc way to access kocolorsets.
 * A display renderer is given to this model to convert KoColor to QColor when
 * colors are requested
 */
class KRITAWIDGETS_EXPORT KisPaletteModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KisPaletteModel(QObject* parent = Q_NULLPTR);
    ~KisPaletteModel() override;

    enum AdditionalRoles {
        IsGroupNameRole = Qt::UserRole + 1,
        CheckSlotRole,
        GroupNameRole,
        RowInGroupRole
    };

public /* overridden methods */: // QAbstractTableModel
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /**
     * @brief index
     * @param row
     * @param column
     * @param parent
     * @return the index of for the data at row, column
     * if the data is a color entry, the internal pointer points to the group
     * the entry belongs to, and the row and column are row number and column
     * number inside the group.
     * if the data is a group, the row number and group number is Q_INFINIFY,
     * and the internal pointer also points to the group
     */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * @brief dropMimeData
     * This is an overridden function that handles dropped mimedata.
     * right now only colorsetentries and colorsetgroups are handled.
     * @return
     */
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;
    /**
     * @brief mimeData
     * gives the mimedata for a kocolorsetentry or a kocolorsetgroup.
     * @param indexes
     * @return the mimedata for the given indices
     */
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    QStringList mimeTypes() const override;

    Qt::DropActions supportedDropActions() const override;
    /**
     * @brief setData
     * setData is not used as KoColor is not a QVariant
     * use setEntry, addEntry and removeEntry instead
     */
    // TODO Used QVariant::setValue and QVariant.value<KoColor> to implement this
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;

Q_SIGNALS:
    /**
     * @brief sigPaletteModified
     * emitted when palette associated with the model is modified
     */
    void sigPaletteModified();
    /**
     * @brief sigPaletteChanged
     * emitted when the palette associated with the model is made another one
     */
    void sigPaletteChanged();

public /* methods */:
    /**
     * @brief addEntry
     * proper function to handle adding entries.
     * @return whether successful.
     */
    bool addEntry(const KisSwatch &entry,
                  const QString &groupName = KoColorSet::GLOBAL_GROUP_NAME);

    void setEntry(const KisSwatch &entry, const QModelIndex &index);

    /**
     * @brief removeEntry
     * proper function to remove the colorsetentry at the given index.
     * The consolidates both removeentry and removegroup.
     * @param index the given index
     * @param keepColors This bool determines whether, when deleting a group,
     * the colors should be added to the default group. This is usually desirable,
     * so hence the default is true.
     * @return if successful
     */
    bool removeEntry(const QModelIndex &index, bool keepColors=true);
    void removeGroup(const QString &groupName, bool keepColors);
    bool renameGroup(const QString &groupName, const QString &newName);
    void addGroup(const KisSwatchGroup &group);
    void setRowNumber(const QString &groupName, int rowCount);
    void clear();

    KisSwatch getEntry(const QModelIndex &index) const;

    void setPalette(KoColorSet* colorSet);
    KoColorSet* colorSet() const;

    QModelIndex indexForClosest(const KoColor &compare);
    int indexRowForInfo(const KisSwatchGroup::SwatchInfo &info);

public Q_SLOTS:

private Q_SLOTS:
    void slotDisplayConfigurationChanged();
    void slotPaletteModified();

private /* methods */:
    QVariant dataForGroupNameRow(const QModelIndex &idx, int role) const;
    QVariant dataForSwatch(const QModelIndex &idx, int role) const;
    int rowNumberInGroup(int rowInModel) const;
    int groupNameRowForRow(int rowInModel) const;
    int groupNameRowForName(const QString &groupName);
    void resetGroupNameRows();
    /**
     * Installs a display renderer object for a palette that will
     * convert the KoColor to the displayable QColor. Default is the
     * dumb renderer.
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);


private /* member variables */:
    QPointer<KoColorSet> m_colorSet;
    QPointer<const KoColorDisplayRendererInterface> m_displayRenderer;
    QMap<int, QString> m_rowGroupNameMap;

friend class KisPaletteView;
};

#endif
