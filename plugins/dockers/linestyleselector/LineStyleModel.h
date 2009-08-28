/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef LINESTYLEMODEL_H
#define LINESTYLEMODEL_H

#include <QPen>
#include <QPainter>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>

/// The line style model managing the style data
class LineStyleModel : public QAbstractListModel
{
public:
    LineStyleModel( QObject * parent = 0 );
    virtual ~LineStyleModel() {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /// adds the given style to the model
    bool addCustomStyle( const QVector<qreal> &style );
    /// selects the given style
    int setLineStyle( Qt::PenStyle style, const QVector<qreal> &dashes );
private:
    QList< QVector<qreal> > m_styles; ///< the added styles
    QVector<qreal> m_tempStyle; ///< a temporary added style
    bool m_hasTempStyle;        ///< state of the temporary style
};

#endif
