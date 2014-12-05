/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ANIMATION_MODEL_H
#define KIS_ANIMATION_MODEL_H

#include <QAbstractTableModel>

class KisAnimationDoc;

#include <krita_export.h>
/**
 * @brief The KisAnimationModel class provides a view over the layers
 * and frames of an animation sequence.
 */
class KRITAUI_EXPORT KisAnimationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KisAnimationModel(KisAnimationDoc *document, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:

public slots:

private:

    KisAnimationDoc *m_document;

};

#endif // KIS_ANIMATION_MODEL_H
