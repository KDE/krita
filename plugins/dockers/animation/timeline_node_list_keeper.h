/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __TIMELINE_NODE_LIST_KEEPER_H
#define __TIMELINE_NODE_LIST_KEEPER_H

#include <QObject>
#include <QList>
#include <QScopedPointer>
#include <QAbstractTableModel>

#include "kritaanimationdocker_export.h"

#include "kis_time_based_item_model.h"

class KisNodeDummy;
class KisDummiesFacadeBase;
class KisNodeDisplayModeAdapter;


class KRITAANIMATIONDOCKER_EXPORT TimelineNodeListKeeper : public QObject
{
    Q_OBJECT
public:
    struct ModelWithExternalNotifications;

    struct OtherLayer;
    typedef QList<OtherLayer> OtherLayersList;


    TimelineNodeListKeeper(ModelWithExternalNotifications *model,
                           KisDummiesFacadeBase *dummiesFacade,
                           KisNodeDisplayModeAdapter *displayModeAdapter);
    ~TimelineNodeListKeeper() override;

    KisNodeDummy* dummyFromRow(int row);
    int rowForDummy(KisNodeDummy *dummy);
    int rowCount();

    OtherLayersList otherLayersList() const;

    void updateActiveDummy(KisNodeDummy *dummy);

private Q_SLOTS:
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);
    void slotDummyChanged(KisNodeDummy *dummy);

    void slotUpdateDummyContent(QObject *dummy);

    void slotDisplayModeChanged();

public:
    struct ModelWithExternalNotifications : public KisTimeBasedItemModel {
        ModelWithExternalNotifications(QObject *parent)
            : KisTimeBasedItemModel(parent) {}

        void callBeginResetModel() {
            beginResetModel();
        }

        void callEndResetModel() {
            endResetModel();
        }

        void callBeginInsertRows(const QModelIndex &parent, int first, int last) {
            beginInsertRows(parent, first, last);
        }

        void callEndInsertRows() {
            endInsertRows();
        }

        void callBeginRemoveRows(const QModelIndex &parent, int first, int last) {
            beginRemoveRows(parent, first, last);
        }

        void callEndRemoveRows() {
            endRemoveRows();
        }

        void callIndexChanged(const QModelIndex &index0, const QModelIndex &index1) {
            emit dataChanged(index0, index1);
        }
    };

    struct OtherLayer {
        OtherLayer(const QString &_name, KisNodeDummy *_dummy)
            : name(_name),
              dummy(_dummy)
        {
        }

        QString name;
        KisNodeDummy *dummy;
    };

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

Q_DECLARE_METATYPE( TimelineNodeListKeeper::OtherLayersList )

#endif /* __TIMELINE_NODE_LIST_KEEPER_H */
