/* This file is part of the KDE project
* Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "MatrixDataModel.h"

MatrixDataModel::MatrixDataModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_rows(0)
    , m_cols(0)
{
}

void MatrixDataModel::setMatrix(const QVector<qreal> &matrix, int rows, int cols)
{
    m_matrix = matrix;
    m_rows = rows;
    m_cols = cols;
    Q_ASSERT(m_rows);
    Q_ASSERT(m_cols);
    Q_ASSERT(m_matrix.count() == m_rows * m_cols);
    reset();
}

QVector<qreal> MatrixDataModel::matrix() const
{
    return m_matrix;
}

int MatrixDataModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_rows;
}

int MatrixDataModel::columnCount(const QModelIndex &/*parent*/) const
{
    return m_cols;
}

QVariant MatrixDataModel::data(const QModelIndex &index, int role) const
{
    int element = index.row() * m_cols + index.column();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return QVariant(QString("%1").arg(m_matrix[element], 2));
        break;
    default:
        return QVariant();
    }
}

bool MatrixDataModel::setData(const QModelIndex &index, const QVariant &value, int /*role*/)
{
    int element = index.row() * m_cols + index.column();
    bool valid = false;
    qreal elementValue = value.toDouble(&valid);
    if (!valid) {
        return false;
    }
    m_matrix[element] = elementValue;
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags MatrixDataModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
