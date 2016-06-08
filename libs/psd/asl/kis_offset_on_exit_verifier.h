/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_OFFSET_ON_EXIT_VERIFIER_H
#define __KIS_OFFSET_ON_EXIT_VERIFIER_H

#include <kis_debug.h>
#include <QString>
#include <QIODevice>

//#define DEBUG_OFFSET_ON_EXIT

/**
 * Check if the position of \p device has moved further by
 * \p expectedOffset and correct it if needed. It also issues
 * a warning if needed.
 */

class KisOffsetOnExitVerifier
{
public:

    KisOffsetOnExitVerifier(QIODevice *device,
                            qint64 expectedOffset,
                            int maxPadding,
                            const QString &objectName = "",
                            const QString &domain = "")
        : m_device(device),
          m_maxPadding(maxPadding),
          m_domain(domain),
          m_objectName(objectName)
    {
        m_expectedPos = m_device->pos() + expectedOffset;
    }

    ~KisOffsetOnExitVerifier() {
        if (m_device->pos() < m_expectedPos - m_maxPadding ||
            m_device->pos() > m_expectedPos) {

#ifdef DEBUG_OFFSET_ON_EXIT

            QString msg =
                QString("Incorrect offset on exit %1, expected %2!")
                .arg(m_device->pos())
                .arg(m_expectedPos);

            warnKrita << "*** |" << m_objectName << msg;
            warnKrita << "    |" << m_domain;

#endif /* DEBUG_OFFSET_ON_EXIT */

            m_device->seek(m_expectedPos);
        }
    }

private:
    QIODevice *m_device;
    int m_maxPadding;
    qint64 m_expectedPos;
    QString m_domain;
    QString m_objectName;
};

#define SETUP_OFFSET_VERIFIER(name, device, expectedOffset, maxPadding) KisOffsetOnExitVerifier name(device, expectedOffset, maxPadding, QString(#name), QString(__FILE__) + ":" + QString::number(__LINE__))

#endif /* __KIS_OFFSET_ON_EXIT_VERIFIER_H */
