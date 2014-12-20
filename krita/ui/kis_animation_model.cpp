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
#include "kis_animation_model.h"

#include <kis_image.h>

#include "kis_animation_doc.h"
#include "kis_animation_layer.h"
#include "kis_animation_frame.h"

KisAnimationModel::KisAnimationModel(KisAnimationDoc *document, QObject *parent)
    : QAbstractTableModel(parent)
    , m_document(document)
{
}

int KisAnimationModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_document->numberOfLayers();
}

int KisAnimationModel::columnCount(const QModelIndex &/*parent*/) const
{
    return m_document->numberOfFrames() + 100; // XXX: make configurable?
}

QVariant KisAnimationModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (!index.isValid()) return result;
    if (role == Qt::DisplayRole) {
        KisAnimationLayer *layer = m_document->layer(index.row());
        if (layer) {
            result.setValue<QObject*>(layer->frameAt(index.column()));
        }
    }
    return result;
}

QVariant KisAnimationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    if (role == Qt::DisplayRole) {
        switch (orientation) {
        case Qt::Horizontal:
            // Only show every tenth frame number
            if (section % 10 == 0) {
                result = QString("%1").arg(section); // Frame number
            }
            break;
        case Qt::Vertical:
        {
            KisAnimationLayer *layer = m_document->layer(section);
            if (layer) {
                result = layer->name();
            }
            break;
        }
        default:
            break;
        };
    }

    return result;

}
