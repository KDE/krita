/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PATTERN_DOCK_H_
#define _PATTERN_DOCK_H_

#include <QDockWidget>
#include <kis_mainwindow_observer.h>

class KoPattern;
class KisPatternChooser;

class PatternDockerDock : public QDockWidget, public KisMainwindowObserver {
    Q_OBJECT
public:
    PatternDockerDock( );

    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    QString observerName() override { return "PatternDockerDock"; }
public Q_SLOTS:
    void patternChanged(KoPattern *pattern);
private Q_SLOTS:

private:
    KisPatternChooser* m_patternChooser;
};


#endif
