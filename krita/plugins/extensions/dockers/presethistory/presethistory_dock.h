/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _PRESETHISTORY_DOCK_H_
#define _PRESETHISTORY_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

class QListWidget;
class QListWidgetItem;
class KisCanvas2;

class PresetHistoryDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    PresetHistoryDock( );
    QString observerName() { return "PresetHistoryDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; setEnabled(false); }

private Q_SLOTS:
    void presetSelected(QListWidgetItem* item);
    void canvasResourceChanged(int key, const QVariant& v);
private:
    KisCanvas2 *m_canvas;
    QListWidget *m_presetHistory;
    bool m_block;
};


#endif
