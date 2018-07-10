/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_ANIMATION_CURVE_DOCKER_H_
#define _KIS_ANIMATION_CURVE_DOCKER_H_

#include <QDockWidget>
#include <kis_mainwindow_observer.h>
#include <QScopedPointer>

class KisCanvas2;
class KisAction;

class KisAnimationCurveDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    KisAnimationCurveDocker();
    ~KisAnimationCurveDocker() override;

    QString observerName() override { return "AnimationCurveDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

private Q_SLOTS:
    void slotUpdateIcons();
    void slotListRowsInserted(const QModelIndex &parentIndex, int first, int last);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
