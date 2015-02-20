/*
 * Copyright (C) 2015  Radosław Wicik <radoslaw@wicik.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <QObject>

#include <marble/MarbleGlobal.h>
#include <marble/MarbleWidget.h>
#include <marble/RenderState.h>
#include <kdebug.h>

//#include "KoReportItemMaps.h"

class KoReportItemMaps;
namespace Marble{
    /**
     * Workaround Marble error in signal specification.
     *
     * renderStatusChange and renderStateChange signals are using params that are in
     * Marble namespace and are not prefixed with Marble:: in their declaration in
     * MarbleWidget.h this class is workaround of the bug.
     */
    class RenderStatusSignalProxy: public QObject{
        Q_OBJECT
    public:
        explicit RenderStatusSignalProxy(QObject* parent):QObject(parent){};
        void setConnection(MarbleWidget& marble){
            connect(&marble,SIGNAL(renderStatusChanged(RenderStatus)),this,SLOT(onRenderStatusChange(RenderStatus)));
            connect(&marble,SIGNAL(renderStateChanged(RenderState)),this,SLOT(onRenderStateChange(RenderState)));
        };
        void unconnect(MarbleWidget& marble){
            disconnect(&marble,SIGNAL(renderStatusChanged(RenderStatus)),this,SLOT(onRenderStatusChange(RenderStatus)));
            disconnect(&marble,SIGNAL(renderStateChanged(RenderState)),this,SLOT(onRenderStateChange(RenderState)));
        }
    public slots:
        void onRenderStatusChange(RenderStatus renderStatus){
            kDebug(44021) << "!!!!!!!!!!!!!!!!  STATUS change";
            emit renderStatusChanged(static_cast<int>(renderStatus));
        };
        void onRenderStateChange(const RenderState &state){
            kDebug(44021) << "################  STATE change";
            emit renderStatusChanged(static_cast<int>(state.status()));
        };
    signals:
        void renderStatusChanged(int renderStatus);
    };
}

class MapRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MapRenderer)
public:
    MapRenderer(QObject* parent = 0);
    virtual ~MapRenderer();
    void renderJob(KoReportItemMaps* reportItemMaps);
signals:
    void jobFinished();
private slots:
    void onRenderStatusChange(int renderStatus);
    void downloadProgres(int active, int queued);
    void downloadFinished();

private:
    Marble::MarbleWidget m_marble;
    KoReportItemMaps* m_currentJob;
    Marble::RenderStatusSignalProxy m_renderStatusProxy;
};

#endif // MAPRENDERER_H
