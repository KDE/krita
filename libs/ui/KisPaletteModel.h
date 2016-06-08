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

#include <QModelIndex>

#include <kis_types.h>
#include "kritaui_export.h"

class KoColorSet;
class KoColorDisplayRendererInterface;

class KRITAUI_EXPORT KisPaletteModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    KisPaletteModel(QObject* parent = 0);
    virtual ~KisPaletteModel();
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    void setColorSet(KoColorSet* colorSet);

    /**
     * Installs a display renderer object for a palette that will
     * convert the KoColor to the displayable QColor. Default is the
     * dumb renderer.
     */
    void setDisplayRenderer(KoColorDisplayRendererInterface *displayRenderer);

private Q_SLOTS:
    void slotDisplayConfigurationChanged();

private:
    KoColorSet* m_colorSet;
    KoColorDisplayRendererInterface *m_displayRenderer;
};

#endif
