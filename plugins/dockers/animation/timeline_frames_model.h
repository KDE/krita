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

#ifndef __TIMELINE_FRAMES_MODEL_H
#define __TIMELINE_FRAMES_MODEL_H


#include <QScopedPointer>
#include <QIcon>

#include "kritaanimationdocker_export.h"
#include "kis_node_model.h"
#include "kis_types.h"
#include "kis_node.h"
#include "timeline_node_list_keeper.h"

class KisNodeDummy;
class KisDummiesFacadeBase;
class KisAnimationPlayer;


class KRITAANIMATIONDOCKER_EXPORT TimelineFramesModel : public TimelineNodeListKeeper::ModelWithExternalNotifications
{
    Q_OBJECT

public:
    enum MimeCopyPolicy {
        UndefinedPolicy = 0,
        MoveFramesPolicy,
        CopyFramesPolicy
    };

public:
    TimelineFramesModel(QObject *parent);
    ~TimelineFramesModel() override;

    bool hasConnectionToCanvas() const;

    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageSP image);

    bool canDropFrameData(const QMimeData *data, const QModelIndex &index);
    bool insertOtherLayer(int index, int dstRow);
    int activeLayerRow() const;

    bool createFrame(const QModelIndex &dstIndex);
    bool copyFrame(const QModelIndex &dstIndex);

    bool insertFrames(int dstColumn, const QList<int> &dstRows, int count, int timing = 1);

    bool insertHoldFrames(QModelIndexList selectedIndexes, int count);

    QString audioChannelFileName() const;
    void setAudioChannelFileName(const QString &fileName);

    bool isAudioMuted() const;
    void setAudioMuted(bool value);

    qreal audioVolume() const;
    void setAudioVolume(qreal value);

    void setFullClipRangeStart(int column);
    void setFullClipRangeEnd(int column);

    void setLastClickedIndex(const QModelIndex &index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QMimeData *mimeDataExtended(const QModelIndexList &indexes, const QModelIndex &baseIndex, MimeCopyPolicy copyPolicy) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    bool dropMimeDataExtended(const QMimeData *data, Qt::DropAction action, const QModelIndex &parent, bool *dataMoved = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    enum ItemDataRole
    {
        ActiveLayerRole = KisTimeBasedItemModel::UserRole,
        TimelinePropertiesRole,
        OtherLayersRole,
        LayerUsedInTimelineRole,
        FrameColorLabelIndexRole
    };

    // metatype is added by the original implementation
    typedef KisBaseNode::Property Property;
    typedef KisBaseNode::PropertyList PropertyList;

    typedef TimelineNodeListKeeper::OtherLayer OtherLayer;
    typedef TimelineNodeListKeeper::OtherLayersList OtherLayersList;


    struct NodeManipulationInterface {
        virtual ~NodeManipulationInterface() {}
        virtual KisLayerSP addPaintLayer() const = 0;
        virtual void removeNode(KisNodeSP node) const = 0;
        virtual bool setNodeProperties(KisNodeSP node, KisImageSP image, KisBaseNode::PropertyList properties) const = 0;
    };

    /**
     * NOTE: the model has an ownership over the interface, that is it'll
     *       be deleted automatically later
     */
    void setNodeManipulationInterface(NodeManipulationInterface *iface);

protected:
    KisNodeSP nodeAt(QModelIndex index) const override;
    QMap<QString, KisKeyframeChannel *> channelsAt(QModelIndex index) const override;

private Q_SLOTS:
    void slotDummyChanged(KisNodeDummy *dummy);
    void slotImageContentChanged();
    void processUpdateQueue();

public Q_SLOTS:
    void slotCurrentNodeChanged(KisNodeSP node);

Q_SIGNALS:
    void requestCurrentNodeChanged(KisNodeSP node);
    void sigInfiniteTimelineUpdateNeeded();
    void sigAudioChannelChanged();
    void sigEnsureRowVisible(int row);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_MODEL_H */
