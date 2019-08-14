/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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


    private:
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
