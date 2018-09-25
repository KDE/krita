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

#include <KoColorDisplayRendererInterface.h>

#include "kritawidgets_export.h"
#include <KoColorSet.h>
#include <QScopedPointer>

class KoColorSet;

/**
 * @brief The KisPaletteModel class
 * This, together with kis_palette_view and kis_palette_delegate forms a mvc way to access kocolorsets.
 */
class KRITAWIDGETS_EXPORT KisPaletteModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    KisPaletteModel(QObject* parent = 0);
    ~KisPaletteModel() override;

    enum AdditionalRoles {
        IsHeaderRole       = Qt::UserRole + 1,
        ExpandCategoryRole = Qt::UserRole + 2,
        RetrieveEntryRole  = Qt::UserRole + 3
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    void setColorSet(KoColorSet* colorSet);
    KoColorSet* colorSet() const;

    /**
     * Installs a display renderer object for a palette that will
     * convert the KoColor to the displayable QColor. Default is the
     * dumb renderer.
     */
    void setDisplayRenderer(KoColorDisplayRendererInterface *displayRenderer);

    /**
     * @brief indexFromId
     * convenience function to get the tableindex from the global palette color.
     * used by lazybrush.
     * @param i
     * @return index in table.
     */
    QModelIndex indexFromId(int i) const;
    /**
     * @brief idFromIndex
     * convenience function to get the global colorset entry id from the table index.
     * If you just want to use this to get the kocolorsetentry, use colorsetEntryFromIndex instead.
     * @param index
     * @return
     */
    int idFromIndex(const QModelIndex &index) const;

    /**
     * @brief colorSetEntryFromIndex
     * This gives the colorset entry for the given table model index.
     * @param index the QModelIndex
     * @return the kocolorsetentry
     */
    KoColorSetEntry colorSetEntryFromIndex(const QModelIndex &index) const;

    /**
     * @brief addColorSetEntry
     * proper function to handle adding entries.
     * @return whether successful.
     */
    bool addColorSetEntry(KoColorSetEntry entry, QString groupName=QString());
    /**
     * @brief removeEntry
     * proper function to remove the colorsetentry at the given index.
     * The consolidtes both removeentry and removegroup.
     * @param keepColors: This bool determines whether, when deleting a group,
     * the colors should be added to the default group. This is usually desirable,
     * so hence the default is true.
     * @return if successful
     */
    bool removeEntry(QModelIndex index, bool keepColors=true);
    /**
     * @brief addGroup
     * Adds a group to the list.
     * @param groupName
     * @return if successful
     */
    bool addGroup(QString groupName = QString());

    bool removeRows(int row, int count, const QModelIndex &parent) override;

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



private Q_SLOTS:
    void slotDisplayConfigurationChanged();

private:
    KoColorSet* m_colorSet;
    QPointer<KoColorDisplayRendererInterface> m_displayRenderer;
    QModelIndex getLastEntryIndex();
};

#endif
