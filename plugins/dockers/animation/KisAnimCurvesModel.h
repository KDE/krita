/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVES_MODEL_H
#define _KIS_ANIMATION_CURVES_MODEL_H

#include <QScopedPointer>
#include <QIcon>
#include <QAbstractItemModel>

#include "KisTimeBasedItemModel.h"
#include "kis_types.h"
#include "kundo2command.h"

class KisScalarKeyframeChannel;

class KisAnimationCurve {
public:
    KisAnimationCurve(KisScalarKeyframeChannel *channel, QColor color);
    KisScalarKeyframeChannel *channel() const;
    QColor color() const;

    void setVisible(bool visible);
    bool visible() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisAnimCurvesModel : public KisTimeBasedItemModel
{
    Q_OBJECT
public:
    KisAnimCurvesModel(QObject *parent);
    ~KisAnimCurvesModel() override;

    bool hasConnectionToCanvas() const;

    KisAnimationCurve *addCurve(KisScalarKeyframeChannel *channel);
    void removeCurve(KisAnimationCurve *curve);
    void setCurveVisible(KisAnimationCurve *curve, bool visible);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

    /**
     * Begins a block of commands. The following calls to setData will be grouped to a single undo step.
     * Note: MUST be followed by a call to endCommand().
     */
    void beginCommand(const KUndo2MagicString &text);
    void endCommand();

    bool adjustKeyframes(const QModelIndexList &indexes, int timeOffset, qreal valueOffset);

    enum ItemDataRole
    {
        ScalarValueRole = KisTimeBasedItemModel::UserRole + 101,
        InterpolationModeRole,
        TangentsModeRole,
        LeftTangentRole,
        RightTangentRole,
        CurveColorRole,
        CurveVisibleRole,
        PreviousKeyframeTime,
        NextKeyframeTime,
        ChannelIdentifier,
        ChannelLimits
    };

Q_SIGNALS:
    void dataAdded(const QModelIndex& index);

protected:
    KisNodeSP nodeAt(QModelIndex index) const override;
    QMap<QString, KisKeyframeChannel *> channelsAt(QModelIndex index) const override;
    KisKeyframeChannel* channelByID(QModelIndex index, const QString &id) const;

private Q_SLOTS:
    void slotKeyframeChanged(const KisKeyframeChannel *channel, int time);
    void slotKeyframeAdded(const KisKeyframeChannel *channel, int time);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

// Used to return a range as a QVariant (see KisAnimationCurvesModel::data)
typedef QPair<qreal,qreal> ChannelLimitsMetatype;
Q_DECLARE_METATYPE(ChannelLimitsMetatype);

#endif
