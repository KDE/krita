/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KisNodeDisplayModeAdapter;


struct TimelineSelectionEntry {
    KisRasterKeyframeChannel* channel;
    int time;
    KisRasterKeyframeSP keyframe;
};

inline bool operator==(const TimelineSelectionEntry& lhs, const TimelineSelectionEntry& rhs){
    return (lhs.time == rhs.time) && (lhs.channel == rhs.channel) && (lhs.keyframe == rhs.keyframe);
}

inline uint qHash(const TimelineSelectionEntry &key)
{
    return reinterpret_cast<quint64>(key.channel) * reinterpret_cast<quint64>(key.keyframe.data()) * key.time;
}

class KRITAANIMATIONDOCKER_EXPORT KisAnimTimelineFramesModel : public TimelineNodeListKeeper::ModelWithExternalNotifications
{
    Q_OBJECT

public:
    enum MimeCopyPolicy {
        UndefinedPolicy = 0,
        MoveFramesPolicy,
        CopyFramesPolicy,
        CloneFramesPolicy
    };

public:
    KisAnimTimelineFramesModel(QObject *parent);
    ~KisAnimTimelineFramesModel() override;

    bool hasConnectionToCanvas() const;

    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade,
                          KisImageSP image,
                          KisNodeDisplayModeAdapter *displayModeAdapter);

    bool canDropFrameData(const QMimeData *data, const QModelIndex &index);
    bool insertOtherLayer(int index, int dstRow);
    int activeLayerRow() const;

    bool createFrame(const QModelIndex &dstIndex);
    bool copyFrame(const QModelIndex &dstIndex);
    void makeClonesUnique(const QModelIndexList &indices);

    bool insertFrames(int dstColumn, const QList<int> &dstRows, int count, int timing = 1);
    bool insertHoldFrames(const QModelIndexList &selectedIndexes, int count);

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
        PinnedToTimelineRole,
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
    KisNodeSP nodeAt(QModelIndex index) const override;

protected:
    QMap<QString, KisKeyframeChannel *> channelsAt(QModelIndex index) const override;
    KisKeyframeChannel* channelByID(QModelIndex index, const QString &id) const;

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
    void sigFullClipRangeChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __TIMELINE_FRAMES_MODEL_H */
