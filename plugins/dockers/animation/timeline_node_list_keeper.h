/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_NODE_LIST_KEEPER_H
#define __TIMELINE_NODE_LIST_KEEPER_H

#include <QObject>
#include <QList>
#include <QScopedPointer>
#include <QAbstractTableModel>

#include "kritaanimationdocker_export.h"

#include "KisTimeBasedItemModel.h"

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
