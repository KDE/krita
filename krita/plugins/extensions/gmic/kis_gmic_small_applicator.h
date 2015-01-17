/*
 * Copyright (c) 2014 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <QThread>
#include <kis_types.h>

class KisGmicFilterSetting;
class QRect;
class QSize;
class KisGmicSmallApplicator : public QThread
{
    Q_OBJECT

public:
    KisGmicSmallApplicator(QObject *parent = 0);
    ~KisGmicSmallApplicator();

    void setProperties(const QRect& canvasRect,
                       const QSize& previewSize,
                       KisNodeListSP layers,
                       KisGmicFilterSetting * settings,
                       const QByteArray& customCommands
                      );

    float getProgress() const;
    KisPaintDeviceSP preview();

signals:
    void gmicFinished(bool successfully, int miliseconds = -1, const QString &msg = QString());
    void previewReady();


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
    float * m_progress;
    KisPaintDeviceSP m_preview;
};

#endif
