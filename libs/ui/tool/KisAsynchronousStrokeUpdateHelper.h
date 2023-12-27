/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISASYNCHRONOUSSTROKEUPDATEHELPER_H
#define KISASYNCHRONOUSSTROKEUPDATEHELPER_H

#include <QObject>
#include <QTimer>
#include "kis_types.h"
#include "kis_stroke_job_strategy.h"
#include "kritaui_export.h"

class KisStrokesFacade;


class KRITAUI_EXPORT KisAsynchronousStrokeUpdateHelper : public QObject
{
    Q_OBJECT
public:
    class UpdateData : public KisStrokeJobData {
    public:
        UpdateData(bool _forceUpdate,
                   Sequentiality sequentiality = SEQUENTIAL,
                   Exclusivity exclusivity = NORMAL)
            : KisStrokeJobData(sequentiality, exclusivity),
              forceUpdate(_forceUpdate)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new UpdateData(*this, levelOfDetail);
        }


    protected:
        UpdateData(const UpdateData &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs),
              forceUpdate(rhs.forceUpdate)
        {
            Q_UNUSED(levelOfDetail);
        }
    public:
        bool forceUpdate = false;
    };

    using UpdateDataFactory = std::function<KisStrokeJobData*(bool)>;

public:
    KisAsynchronousStrokeUpdateHelper();
    ~KisAsynchronousStrokeUpdateHelper();

    /**
     * Initialize the update helper and start the steam of the
     * update events. This is just a simple combination of
     * initUpdateStreamLowLevel() and startUpdateStreamLowLevel().
     */
    void startUpdateStream(KisStrokesFacade *strokesFacade, KisStrokeId strokeId);

    /**
     *  A low-level version of startUpdateStream(...), initializes
     *  the helper but doesn't start the stream of update events.
     *  That is needed for the tool to issue endUpdateStream()
     *  signals, when the action has been ended **before** the
     *  stroke actually managed to initialize itself.
     */
    void initUpdateStreamLowLevel(KisStrokesFacade *strokesFacade, KisStrokeId strokeId);

    /**
     * Start the stream of the update events on **already
     * initialized** helper. One should call
     * initUpdateStreamWithoutStart() before calling
     * startUpdateStream().
     */
    void startUpdateStreamLowLevel();

    void endUpdateStream();
    void cancelUpdateStream();

    bool isActive() const;

    void setCustomUpdateDataFactory(UpdateDataFactory factory);

private Q_SLOTS:
    void slotAsyncUpdateCame(bool forceUpdate = false);

private:
    KisStrokesFacade *m_strokesFacade;
    QTimer m_updateThresholdTimer;
    KisStrokeId m_strokeId;
    UpdateDataFactory m_customUpdateFactory;
};

#endif // KISASYNCHRONOUSSTROKEUPDATEHELPER_H
