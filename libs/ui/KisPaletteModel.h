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

#include <kis_types.h>
#include "kritaui_export.h"
#include <KoColorSet.h>
#include <QScopedPointer>

class KoColorSet;

/**
 * @brief The KisPaletteModel class
 * This, together with kis_palette_view and kis_palette_delegate forms a mvc way to access kocolorsets.
 */
class KRITAUI_EXPORT KisPaletteModel : public QAbstractTableModel
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

    QModelIndex indexFromId(int i) const;
    int idFromIndex(const QModelIndex &index) const;

    /**
     * @brief colorSetEntryFromIndex
     * This gives the colorset entry for the given table model index.
     * @param index the QModelIndex
     * @return the kocolorsetentry
     */
    KoColorSetEntry colorSetEntryFromIndex(const QModelIndex &index);

private Q_SLOTS:
    void slotDisplayConfigurationChanged();

private:
    KoColorSet* m_colorSet;
    QPointer<KoColorDisplayRendererInterface> m_displayRenderer;
};

#endif
