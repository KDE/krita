/* This file is part of the KDE project
 * Copyright (C) 2013 Thorsten Zachmann <zachmann@kde.org>
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

#include "StylesManagerModel.h"

#include <KoCharacterStyle.h>
#include <KoStyleThumbnailer.h>

#include <QDebug>

StylesManagerModel::StylesManagerModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_styleThumbnailer(0)
{
}

QVariant StylesManagerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= m_styles.size()) {
        return QVariant();
    }
    //qDebug() << Q_FUNC_INFO << row << role;

    QVariant retval;
    switch (role) {
    case Qt::DisplayRole:
        retval = m_styles[row]->name();
        break;
    case Qt::DecorationRole:
        if (!m_styleThumbnailer) {
            retval = QPixmap();
        } else {
            retval = m_styleThumbnailer->thumbnail(m_styles[row]);
        }
        break;
    case StylePointer:
        retval = QVariant::fromValue(m_styles[row]);
        break;
    case Qt::SizeHintRole:
        retval = QVariant(QSize(250, 48));
        break;
    default:
        break;
    };
    return retval;
}

int StylesManagerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_styles.size();
}

void StylesManagerModel::setStyleThumbnailer(KoStyleThumbnailer *thumbnailer)
{
    m_styleThumbnailer = thumbnailer;
}

void StylesManagerModel::setStyles(const QList<KoCharacterStyle *> &styles)
{
    beginResetModel();
    m_styles = styles;
    endResetModel();
}

void StylesManagerModel::addStyle(KoCharacterStyle *style)
{
    if (m_styles.indexOf(style) == -1) {
        beginInsertRows(QModelIndex(), m_styles.size(), m_styles.size());
        m_styles.append(style);
        endInsertRows();
    }
}

void StylesManagerModel::removeStyle(KoCharacterStyle *style)
{
    int row = m_styles.indexOf(style);
    Q_ASSERT(row != -1);
    if (row != -1) {
        beginRemoveRows(QModelIndex(), row, row);
        m_styles.removeAt(row);
        endRemoveRows();
    }
}

void StylesManagerModel::replaceStyle(KoCharacterStyle *oldStyle, KoCharacterStyle *newStyle)
{
    qDebug() << Q_FUNC_INFO << oldStyle << "->" << newStyle;
    int row = m_styles.indexOf(oldStyle);
    Q_ASSERT(row != -1);
    if (row != -1) {
        m_styles[row] = newStyle;
        QModelIndex index = this->index(row);
        emit dataChanged(index, index);
    }
}

void StylesManagerModel::updateStyle(KoCharacterStyle *style)
{
    int row = m_styles.indexOf(style);
    Q_ASSERT(row != -1);
    if (row != -1) {
        qDebug() << Q_FUNC_INFO << style << style->name();
        m_styleThumbnailer->removeFromCache(style);
        QModelIndex index = this->index(row);
        emit dataChanged(index, index);
    }
}

QModelIndex StylesManagerModel::styleIndex(KoCharacterStyle *style)
{
    QModelIndex index;
    int row = m_styles.indexOf(style);
    if (row != -1) {
        index = this->index(row);
    }
    return index;
}

