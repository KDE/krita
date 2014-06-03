/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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
#ifndef KIS_FLIPBOOK_H
#define KIS_FLIPBOOK_H

#include <QStandardItemModel>
#include <krita_export.h>

class KisFlipbookItem;
class QString;

class KRITAUI_EXPORT KisFlipbook : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit KisFlipbook(QObject *parent = 0);

    void setName(const QString &name);
    QString name() const;

    KisFlipbookItem *addItem(const QString &url);

    void load(const QString &url);
    void save(const QString &url);

private:

    QString m_name;

};

#endif // KIS_FLIPBOOK_H
