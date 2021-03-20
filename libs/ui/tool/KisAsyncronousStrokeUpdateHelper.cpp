/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncronousStrokeUpdateHelper.h"
#include "kis_image_interfaces.h"

KisAsyncronousStrokeUpdateHelper::KisAsyncronousStrokeUpdateHelper()
    : m_strokesFacade(0)
{
    m_updateThresholdTimer.setSingleShot(false);
    m_updateThresholdTimer.setInterval(80 /* ms */);
    connect(&m_updateThresholdTimer, SIGNAL(timeout()), SLOT(slotAsyncUpdateCame()));
}

KisAsyncronousStrokeUpdateHelper::~KisAsyncronousStrokeUpdateHelper()
{

}

void KisAsyncronousStrokeUpdateHelper::startUpdateStream(KisStrokesFacade *strokesFacade, KisStrokeId strokeId)
{
    m_strokesFacade = strokesFacade;
    m_strokeId = strokeId;
    m_updateThresholdTimer.start();
}

void KisAsyncronousStrokeUpdateHelper::endUpdateStream()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(isActive());

    slotAsyncUpdateCame(true);
    cancelUpdateStream();
}

void KisAsyncronousStrokeUpdateHelper::cancelUpdateStream()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(isActive());

    m_updateThresholdTimer.stop();
    m_strokeId.clear();
    m_strokesFacade = 0;
}

bool KisAsyncronousStrokeUpdateHelper::isActive() const
{
    return m_strokeId;
}

void KisAsyncronousStrokeUpdateHelper::setCustomUpdateDataFactory(KisAsyncronousStrokeUpdateHelper::UpdateDataFactory factory)
{
    m_customUpdateFactory = factory;
}

void KisAsyncronousStrokeUpdateHelper::slotAsyncUpdateCame(bool forceUpdate)
{
    if (!m_strokeId || !m_strokesFacade) return;
    m_strokesFacade->addJob(m_strokeId,
                            m_customUpdateFactory ?
                                m_customUpdateFactory(forceUpdate) :
                                new UpdateData(forceUpdate));
}
