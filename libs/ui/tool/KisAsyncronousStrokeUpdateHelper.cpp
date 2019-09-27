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
