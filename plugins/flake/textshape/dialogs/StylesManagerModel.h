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

#ifndef STYLESMODELNEW_H
#define STYLESMODELNEW_H

#include <QAbstractListModel>
#include <QList>

class KoCharacterStyle;
class KoStyleThumbnailer;

class StylesManagerModel : public QAbstractListModel
{
public:
    enum Roles {
        StylePointer = Qt::UserRole + 1,
    };

    explicit StylesManagerModel(QObject *parent = 0);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void setStyleThumbnailer(KoStyleThumbnailer *thumbnailer);
    void setStyles(const QList<KoCharacterStyle *> &styles);
    void addStyle(KoCharacterStyle *style);
    void removeStyle(KoCharacterStyle *style);
    void replaceStyle(KoCharacterStyle *oldStyle, KoCharacterStyle *newStyle);
    void updateStyle(KoCharacterStyle *style);

    QModelIndex styleIndex(KoCharacterStyle *style);

private:
    QList<KoCharacterStyle *> m_styles;
    KoStyleThumbnailer *m_styleThumbnailer;
};

#endif /* STYLESMODELNEW_H */
