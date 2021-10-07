/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <kis_dom_utils.h>

#include "KisLevelsCurve.h"

KisLevelsCurve::KisLevelsCurve()
    : KisLevelsCurve(defaultInputBlackPoint(), defaultInputWhitePoint(), defaultInputGamma(),
                     defaultOutputBlackPoint(), defaultOutputWhitePoint())
{}

KisLevelsCurve::KisLevelsCurve(qreal inputBlackPoint, qreal inputWhitePoint, qreal inputGamma,
                               qreal outputBlackPoint, qreal outputWhitePoint)
    : m_inputBlackPoint(inputBlackPoint)
    , m_inputWhitePoint(inputWhitePoint)
    , m_inputGamma(inputGamma)
    , m_outputBlackPoint(outputBlackPoint)
    , m_outputWhitePoint(outputWhitePoint)
    , m_inputLevelsDelta(inputWhitePoint - inputBlackPoint)
    , m_inverseInputGamma(1.0 / m_inputGamma)
    , m_outputLevelsDelta(outputWhitePoint - outputBlackPoint)
    , m_mustRecomputeU16Transfer(true)
    , m_mustRecomputeFTransfer(true)
{}
                
bool KisLevelsCurve::operator==(const KisLevelsCurve& rhs) const
{
    return
        &rhs == this ||
        (
            m_inputBlackPoint == rhs.m_inputBlackPoint &&
            m_inputWhitePoint == rhs.m_inputWhitePoint &&
            m_inputGamma == rhs.m_inputGamma &&
            m_outputBlackPoint == rhs.m_outputBlackPoint &&
            m_outputWhitePoint == rhs.m_outputWhitePoint
        );
}

qreal KisLevelsCurve::value(qreal x) const
{
    if (x <= m_inputBlackPoint)
        return m_outputBlackPoint;
    else if (x < m_inputWhitePoint) {
        return m_outputBlackPoint + m_outputLevelsDelta * std::pow((x - m_inputBlackPoint) / m_inputLevelsDelta, m_inverseInputGamma);
    } else {
        return m_outputWhitePoint;
    }
}

qreal KisLevelsCurve::inputBlackPoint() const
{
    return m_inputBlackPoint;
}

qreal KisLevelsCurve::inputWhitePoint() const
{
    return m_inputWhitePoint;
}

qreal KisLevelsCurve::inputGamma() const
{
    return m_inputGamma;
}

qreal KisLevelsCurve::outputBlackPoint() const
{
    return m_outputBlackPoint;
}

qreal KisLevelsCurve::outputWhitePoint() const
{
    return m_outputWhitePoint;
}

void KisLevelsCurve::setInputBlackPoint(qreal newInputBlackPoint)
{
    m_inputBlackPoint = newInputBlackPoint;
    m_inputLevelsDelta = m_inputWhitePoint - m_inputBlackPoint;
    invalidate();
}

void KisLevelsCurve::setInputWhitePoint(qreal newInputWhitePoint)
{
    m_inputWhitePoint = newInputWhitePoint;
    m_inputLevelsDelta = m_inputWhitePoint - m_inputBlackPoint;
    invalidate();
}

void KisLevelsCurve::setInputGamma(qreal newInputGamma)
{
    m_inputGamma = newInputGamma;
    m_inverseInputGamma = 1.0 / m_inputGamma;
    invalidate();
}

void KisLevelsCurve::setOutputBlackPoint(qreal newOutputBlackPoint)
{
    m_outputBlackPoint = newOutputBlackPoint;
    m_outputLevelsDelta = m_outputWhitePoint - m_outputBlackPoint;
    invalidate();
}

void KisLevelsCurve::setOutputWhitePoint(qreal newOutputWhitePoint)
{
    m_outputWhitePoint = newOutputWhitePoint;
    m_outputLevelsDelta = m_outputWhitePoint - m_outputBlackPoint;
    invalidate();
}

void KisLevelsCurve::resetAll()
{
    m_inputBlackPoint = defaultInputBlackPoint();
    m_inputWhitePoint = defaultInputWhitePoint();
    m_inputGamma = defaultInputGamma();
    m_outputBlackPoint = defaultOutputBlackPoint();
    m_outputWhitePoint = defaultOutputWhitePoint();
    m_inputLevelsDelta = m_inputWhitePoint - m_inputBlackPoint;
    m_inverseInputGamma = 1.0 / m_inputGamma;
    m_outputLevelsDelta = m_outputWhitePoint - m_outputBlackPoint;
    invalidate();
}

void KisLevelsCurve::resetInputLevels()
{
    m_inputBlackPoint = defaultInputBlackPoint();
    m_inputWhitePoint = defaultInputWhitePoint();
    m_inputGamma = defaultInputGamma();
    m_inputLevelsDelta = m_inputWhitePoint - m_inputBlackPoint;
    m_inverseInputGamma = 1.0 / m_inputGamma;
    invalidate();
}

void KisLevelsCurve::resetOutputLevels()
{
    m_outputBlackPoint = defaultOutputBlackPoint();
    m_outputWhitePoint = defaultOutputWhitePoint();
    m_outputLevelsDelta = m_outputWhitePoint - m_outputBlackPoint;
    invalidate();
}

bool KisLevelsCurve::isIdentity() const
{
    return 
        m_inputBlackPoint == 0.0 &&
        m_inputWhitePoint == 1.0 &&
        m_inputGamma == 1.0 &&
        m_outputBlackPoint == 0.0 &&
        m_outputWhitePoint == 1.0;
}

const QString& KisLevelsCurve::name() const
{
    return m_name;
}

void KisLevelsCurve::setName(const QString &newName)
{
    m_name = newName;
}

const QVector<quint16>& KisLevelsCurve::uint16Transfer(int size) const
{

    if (!m_mustRecomputeU16Transfer && size == m_u16Transfer.size()) {
        return m_u16Transfer;
    }

    m_u16Transfer.resize(size);

    for (int i = 0; i < size; ++i) {
        const qreal x = static_cast<qreal>(i) / static_cast<qreal>(size - 1);
        m_u16Transfer[i] = static_cast<quint16>(qRound(value(x) * static_cast<qreal>(0xFFFF)));
    }

    m_mustRecomputeU16Transfer = false;
    return m_u16Transfer;
}

const QVector<qreal>& KisLevelsCurve::floatTransfer(int size) const
{
    if (!m_mustRecomputeFTransfer && size == m_fTransfer.size()) {
        return m_fTransfer;
    }

    m_fTransfer.resize(size);

    for (int i = 0; i < size; ++i) {
        m_fTransfer[i] = value(static_cast<qreal>(i) / static_cast<qreal>(size - 1));
    }

    m_mustRecomputeFTransfer = false;
    return m_fTransfer;
}

QString KisLevelsCurve::toString() const
{
    return
        KisDomUtils::toString(m_inputBlackPoint) + ";" +
        KisDomUtils::toString(m_inputWhitePoint) + ";" +
        KisDomUtils::toString(m_inputGamma) + ";" +
        KisDomUtils::toString(m_outputBlackPoint) + ";" +
        KisDomUtils::toString(m_outputWhitePoint);
}

void KisLevelsCurve::fromString(const QString &text, bool *ok)
{
    const QStringList data = text.split(';');

    if (data.size() != 5) {
        if (ok) {
            *ok = false;
        }
        return;
    }

    qreal values[5];
    bool ok_;
    for (int i = 0; i < 5; ++i) {
        ok_ = false;
        values[i] = KisDomUtils::toDouble(data.at(i), &ok_);
        if (!ok_) {
            if (ok) {
                *ok = false;
            }
            return;
        }
    }

    m_inputBlackPoint = values[0];
    m_inputWhitePoint = values[1];
    m_inputGamma = values[2];
    m_outputBlackPoint = values[3];
    m_outputWhitePoint = values[4];
    m_inputLevelsDelta = m_inputWhitePoint - m_inputBlackPoint;
    m_inverseInputGamma = 1.0 / m_inputGamma;
    m_outputLevelsDelta = m_outputWhitePoint - m_outputBlackPoint;
    invalidate();

    if (ok) {
        *ok = true;
    }
}

void KisLevelsCurve::invalidate()
{
    m_mustRecomputeU16Transfer = true;
    m_mustRecomputeFTransfer = true;
}
