/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_UPDATE_OUTLINE_JOB_H
#define __KIS_UPDATE_OUTLINE_JOB_H

#include <QColor>

#include "kis_spontaneous_job.h"
#include "kis_selection.h"

class KRITAIMAGE_EXPORT KisUpdateOutlineJob : public KisSpontaneousJob
{
public:
    KisUpdateOutlineJob(KisSelectionSP selection, bool updateThumbnail, const QColor &maskColor);

    bool overrides(const KisSpontaneousJob *otherJob) override;
    void run() override;
    int levelOfDetail() const override;
    QString debugName() const override;

private:
    KisSelectionSP m_selection;
    bool m_updateThumbnail;
    QColor m_maskColor;
};

#endif /* __KIS_UPDATE_OUTLINE_JOB_H */
