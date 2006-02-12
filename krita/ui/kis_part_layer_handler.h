/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_PART_LAYER_HANDLER_
#define KIS_PART_LAYER_HANDLER_

#include <qobject.h>
#include <KoQueryTrader.h> // KoDocumentEntry

#include "kis_types.h"
#include "kis_doc.h"
#include "kis_view.h"

class QKeyEvent;

class KisPartLayerHandler : public QObject {
Q_OBJECT
public:
    KisPartLayerHandler(KisView* view, const KoDocumentEntry& entry,
                        KisGroupLayerSP parent, KisLayerSP above);
signals:
    void sigGotMoveEvent(KisMoveEvent* event);
    void sigGotKeyPressEvent(QKeyEvent* event);
    void handlerDone();

protected slots:

    void gotMoveEvent(KisMoveEvent* event);
    void gotButtonPressEvent(KisButtonPressEvent* event);
    void gotButtonReleaseEvent(KisButtonReleaseEvent* event);
    void gotKeyPressEvent(QKeyEvent* event);
protected:
    void done();
    KisGroupLayerSP m_parent;
    KisLayerSP m_above;
    KisView* m_view;
    KoDocumentEntry m_entry;
    QPoint m_start;
    QPoint m_end;
    bool m_started;
};

#endif // KIS_PART_LAYER_HANDLER
