/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisProofingOptionsWidget.h"

#include <KisWidgetConnectionUtils.h>
#include <dialogs/KisProofingConfigModel.h>
#include <KoColorProfile.h>
#include <kis_signal_compressor.h>


#include "ui_wdgproofingoptions.h"

struct KisProofingOptionsWidget::Private
{
    Ui_WdgProofingOptions ui;
    KisProofingConfigModel proofingModel;
    KisSignalCompressor *softProofConfigCompressor {nullptr};
};

KisProofingOptionsWidget::KisProofingOptionsWidget(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->ui.setupUi(this);

    m_d->ui.proofSpaceSelector->showDepth(false);

    connect(m_d->ui.proofSpaceSelector,
            qOverload<const KoColorSpace *>(&KisColorSpaceSelector::colorSpaceChanged),
            this,
            [this](const KoColorSpace *cs) {
                m_d->proofingModel.setProofingColorSpaceIdAtomic(cs->colorModelId().id(),
                                                                 cs->colorDepthId().id(),
                                                                 cs->profile() ? cs->profile()->name() : "");
            });

    connect(&m_d->proofingModel,
            &KisProofingConfigModel::proofingSpaceTupleChanged,
            this,
            [this](const KisProofingConfigModel::ColorSpaceId &tuple) {
                QSignalBlocker b(m_d->ui.proofSpaceSelector);
                m_d->ui.proofSpaceSelector->setCurrentColorSpace(
                    KoColorSpaceRegistry::instance()->colorSpace(std::get<0>(tuple),
                                                                 std::get<1>(tuple),
                                                                 std::get<2>(tuple)));
            });
    m_d->proofingModel.LAGER_QT(proofingSpaceTuple).nudge();


    m_d->softProofConfigCompressor = new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this);

    KisWidgetConnectionUtils::connectControl(m_d->ui.ckbBlackPointComp, &m_d->proofingModel, "convBlackPointCompensation");
    KisWidgetConnectionUtils::connectControl(m_d->ui.gamutAlarm, &m_d->proofingModel, "warningColor");
    KisWidgetConnectionUtils::connectControlState(m_d->ui.cmbDisplayTransformState, &m_d->proofingModel, "displayTransformModeState", "displayTransformMode");
    KisWidgetConnectionUtils::connectControlState(m_d->ui.chkDoChromaticAdaptation, &m_d->proofingModel, "adaptationSwitchState", "adaptationSwitch");
    KisWidgetConnectionUtils::connectControlState(m_d->ui.cmbIntent, &m_d->proofingModel, "conversionIntentState", "conversionIntent");
    KisWidgetConnectionUtils::connectControlState(m_d->ui.cmbDisplayIntent, &m_d->proofingModel, "effectiveDisplayIntentState", "displayIntent");
    KisWidgetConnectionUtils::connectControlState(m_d->ui.chkDisplayBlackPointCompensation, &m_d->proofingModel, "effectiveDispBlackPointCompensationState", "dispBlackPointCompensation");

    connect(&m_d->proofingModel, &KisProofingConfigModel::modelChanged,
            m_d->softProofConfigCompressor, &KisSignalCompressor::start);
    connect(m_d->softProofConfigCompressor, &KisSignalCompressor::timeout,
            this, &KisProofingOptionsWidget::slotProofingConfigChanged);
}

KisProofingOptionsWidget::~KisProofingOptionsWidget()
{
}

KisProofingConfigurationSP KisProofingOptionsWidget::currentProofingConfig() const
{
    return toQShared(new KisProofingConfiguration(m_d->proofingModel.data.get()));
}

void KisProofingOptionsWidget::setProofingConfig(KisProofingConfigurationSP config)
{
    m_d->proofingModel.data.set(*config);
}

void KisProofingOptionsWidget::setDisplayConfigOptions(const KisDisplayConfig::Options &options)
{
    m_d->proofingModel.updateDisplayConfigOptions(options);
}

void KisProofingOptionsWidget::slotProofingConfigChanged()
{
    Q_EMIT sigProofingConfigChanged(currentProofingConfig());
}

void KisProofingOptionsWidget::stopPendingUpdates()
{
    m_d->softProofConfigCompressor->stop();
}