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

#ifndef MATRIXDATAMODEL_H
#define MATRIXDATAMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class MatrixDataModel : public QAbstractTableModel
{
public:
    /// Creates a new matrix data model
    explicit MatrixDataModel(QObject *parent = 0);

    /// Sets the matrix data and rows/columns to use
    void setMatrix(const QVector<qreal> &matrix, int rows, int cols);

    /// Returns the matrix data
    QVector<qreal> matrix() const;

    // reimplemented
    int rowCount(const QModelIndex &/*parent*/) const;
    // reimplemented
    int columnCount(const QModelIndex &/*parent*/) const;
    // reimplemented
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    // reimplemented
    bool setData(const QModelIndex &index, const QVariant &value, int /*role*/);
    // reimplemented
    Qt::ItemFlags flags(const QModelIndex &/*index*/) const;

private:
    QVector<qreal> m_matrix; ///< the matrix data to handle
    int m_rows; ///< the number or rows in the matrix
    int m_cols; ///< the number of columns in the matrix
};

#endif // MATRIXDATAMODEL_H
