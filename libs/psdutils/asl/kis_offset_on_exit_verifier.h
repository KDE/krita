/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OFFSET_ON_EXIT_VERIFIER_H
#define __KIS_OFFSET_ON_EXIT_VERIFIER_H

#include "kritapsdutils_export.h"

#include <QIODevice>
#include <QString>
#include <kis_debug.h>

//#define DEBUG_OFFSET_ON_EXIT

/**
 * Check if the position of \p device has moved further by
 * \p expectedOffset and correct it if needed. It also issues
 * a warning if needed.
 */

class KRITAPSDUTILS_EXPORT KisOffsetOnExitVerifier
{
public:
    KisOffsetOnExitVerifier(QIODevice &device, qint64 expectedOffset, int maxPadding, const QString &objectName = "", const QString &domain = "")
        : m_device(device)
        , m_maxPadding(maxPadding)
        , m_domain(domain)
        , m_objectName(objectName)
    {
        m_expectedPos = m_device.pos() + expectedOffset;
    }

    ~KisOffsetOnExitVerifier()
    {
        if (m_device.pos() < m_expectedPos - m_maxPadding || m_device.pos() > m_expectedPos) {
#ifdef DEBUG_OFFSET_ON_EXIT

            QString msg = QString("Incorrect offset on exit %1, expected %2!").arg(m_device.pos()).arg(m_expectedPos);

            warnKrita << "*** |" << m_objectName << msg;
            warnKrita << "    |" << m_domain;

#endif /* DEBUG_OFFSET_ON_EXIT */

            m_device.seek(m_expectedPos);
        }
    }

private:
    QIODevice &m_device;
    int m_maxPadding;
    qint64 m_expectedPos;
    QString m_domain;
    QString m_objectName;
};

#define SETUP_OFFSET_VERIFIER(name, device, expectedOffset, maxPadding)                                                                                        \
    KisOffsetOnExitVerifier name(device, expectedOffset, maxPadding, QString(#name), QString(__FILE__) + ":" + QString::number(__LINE__))

#endif /* __KIS_OFFSET_ON_EXIT_VERIFIER_H */
