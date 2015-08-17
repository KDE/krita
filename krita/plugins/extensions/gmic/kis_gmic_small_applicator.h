/*
 * Copyright (c) 2014-2015 Lukáš Tvrdý <lukast.dev@gmail.com
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


#ifndef KIS_GMIC_SMALL_APPLICATOR_H
#define KIS_GMIC_SMALL_APPLICATOR_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include <kis_types.h>

#include "kis_gmic_data.h"

#include <QSize>
#include <QRect>

class KisGmicFilterSetting;
class KisGmicSmallApplicator : public QThread
{
    Q_OBJECT

public:
    KisGmicSmallApplicator(QObject *parent = 0);
    ~KisGmicSmallApplicator();

    void render(const QRect& canvasRect,
                       const QSize& previewSize,
                       KisNodeListSP layers,
                       KisGmicFilterSetting * settings,
                       const QByteArray& customCommands
                      );

    float progress() const;
    KisPaintDeviceSP preview();

Q_SIGNALS:
    void gmicFinished(bool successfully, int miliseconds = -1, const QString &msg = QString());
    void previewFinished(bool successfully);

protected:
    void run();

private:
    static KisNodeListSP createPreviewThumbnails(KisNodeListSP layers,const QSize &dstSize,const QRect &srcRect);

private:
    QRect m_canvasRect;
    QSize m_previewSize;
    KisNodeListSP m_layers;
    KisGmicFilterSetting * m_setting;
    QByteArray m_gmicCustomCommands;
    KisPaintDeviceSP m_preview;
    KisGmicDataSP m_gmicData;

    bool m_abort;
    bool m_restart;

    QMutex m_mutex;
    QWaitCondition m_condition;

};

#endif
