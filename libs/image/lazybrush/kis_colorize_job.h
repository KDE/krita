/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_COLORIZE_JOB_H
#define __KIS_COLORIZE_JOB_H

#include <QScopedPointer>
#include <QObject>

#include "kis_spontaneous_job.h"
#include "kis_multiway_cut.h"


class KisColorizeJob : public QObject, public KisSpontaneousJob
{
    Q_OBJECT
public:
    KisColorizeJob(KisPaintDeviceSP src,
                   KisPaintDeviceSP dst,
                   KisPaintDeviceSP filteredSource,
                   bool filteredSourceValid,
                   const QRect &boundingRect);
    ~KisColorizeJob();

    void addKeyStroke(KisPaintDeviceSP dev, const KoColor &color);

    void run();
    bool overrides(const KisSpontaneousJob *otherJob);
    int levelOfDetail() const;

Q_SIGNALS:
    void sigFinished();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLORIZE_JOB_H */
