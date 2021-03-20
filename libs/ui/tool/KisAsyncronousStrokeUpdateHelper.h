/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISASYNCRONOUSSTROKEUPDATEHELPER_H
#define KISASYNCRONOUSSTROKEUPDATEHELPER_H

#include <QObject>
#include <QTimer>
#include "kis_types.h"
#include "kis_stroke_job_strategy.h"
#include "kritaui_export.h"

class KisStrokesFacade;


class KRITAUI_EXPORT KisAsyncronousStrokeUpdateHelper : public QObject
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
    KisAsyncronousStrokeUpdateHelper();
    ~KisAsyncronousStrokeUpdateHelper();

    void startUpdateStream(KisStrokesFacade *strokesFacade, KisStrokeId strokeId);
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

#endif // KISASYNCRONOUSSTROKEUPDATEHELPER_H
