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

#include "palettemodel.h"

#include <QBrush>

#include <KoColorSpace.h>
#include <KoColorSet.h>
#include <KoColorDisplayRendererInterface.h>

#include <kis_layer.h>
#include <kis_paint_layer.h>

PaletteModel::PaletteModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_colorSet(0),
      m_displayRenderer(KoDumbColorDisplayRenderer::instance())
{
}

PaletteModel::~PaletteModel()
{
}

void PaletteModel::setDisplayRenderer(KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        if (m_displayRenderer) {
            disconnect(m_displayRenderer, 0, this, 0);
        }
        m_displayRenderer = displayRenderer;
        connect(m_displayRenderer, SIGNAL(displayConfigurationChanged()),
                SLOT(slotDisplayConfigurationChanged()));
    } else {
        m_displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void PaletteModel::slotDisplayConfigurationChanged()
{
    reset();
}

QVariant PaletteModel::data(const QModelIndex& index, int role) const
{
    if (m_colorSet) {
        int i = index.row()*columnCount()+index.column();
        if (i < m_colorSet->nColors()) {
            switch (role) {
                case Qt::BackgroundRole: {
                    QColor color = m_displayRenderer->toQColor(m_colorSet->getColor(i).color);
                    return QBrush(color);
                }
                break;
            }
        }
    }
    return QVariant();
}

int PaletteModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (!m_colorSet) {
        return 0;
    }
    if (m_colorSet->columnCount() > 0) {
        return m_colorSet->nColors()/m_colorSet->columnCount() + 1;
    } 
    return m_colorSet->nColors()/15 + 1;
}

int PaletteModel::columnCount(const QModelIndex& /*parent*/) const
{
    if (m_colorSet && m_colorSet->columnCount() > 0) {
        return m_colorSet->columnCount();
    }
    return 15;
}

Qt::ItemFlags PaletteModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

QModelIndex PaletteModel::index(int row, int column, const QModelIndex& parent) const
{
    int index = row*columnCount()+column;
    if (m_colorSet && index < m_colorSet->nColors()) {
        return QAbstractTableModel::index(row, column, parent);
    }
    return QModelIndex();
}


void PaletteModel::setColorSet(KoColorSet* colorSet)
{
    m_colorSet = colorSet;
    reset();
}

