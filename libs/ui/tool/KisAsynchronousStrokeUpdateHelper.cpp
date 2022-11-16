/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsynchronousStrokeUpdateHelper.h"
#include "kis_image_interfaces.h"

KisAsynchronousStrokeUpdateHelper::KisAsynchronousStrokeUpdateHelper()
    : m_strokesFacade(0)
{
    m_updateThresholdTimer.setSingleShot(false);
    m_updateThresholdTimer.setInterval(80 /* ms */);
    connect(&m_updateThresholdTimer, SIGNAL(timeout()), SLOT(slotAsyncUpdateCame()));
}

KisAsynchronousStrokeUpdateHelper::~KisAsynchronousStrokeUpdateHelper()
{

}

void KisAsynchronousStrokeUpdateHelper::startUpdateStream(KisStrokesFacade *strokesFacade, KisStrokeId strokeId)
{
    m_strokesFacade = strokesFacade;
    m_strokeId = strokeId;
    m_updateThresholdTimer.start();
}

void KisAsynchronousStrokeUpdateHelper::endUpdateStream()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(isActive());

    slotAsyncUpdateCame(true);
    cancelUpdateStream();
}

void KisAsynchronousStrokeUpdateHelper::cancelUpdateStream()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(isActive());

    m_updateThresholdTimer.stop();
    m_strokeId.clear();
    m_strokesFacade = 0;
}

bool KisAsynchronousStrokeUpdateHelper::isActive() const
{
    return m_strokeId;
}

void KisAsynchronousStrokeUpdateHelper::setCustomUpdateDataFactory(KisAsynchronousStrokeUpdateHelper::UpdateDataFactory factory)
{
    m_customUpdateFactory = factory;
}

void KisAsynchronousStrokeUpdateHelper::slotAsyncUpdateCame(bool forceUpdate)
{
    if (!m_strokeId || !m_strokesFacade) return;
    m_strokesFacade->addJob(m_strokeId,
                            m_customUpdateFactory ?
                                m_customUpdateFactory(forceUpdate) :
                                new UpdateData(forceUpdate));
}
