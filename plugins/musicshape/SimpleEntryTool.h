/* This file is part of the KDE project
 * Copyright 2007,2009 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef SIMPLEENTRY_TOOL
#define SIMPLEENTRY_TOOL

#include <QPointF>
#include <KoTool.h>
#include "core/Chord.h"

class MusicShape;
class QUndoCommand;
class AbstractMusicAction;
class MusicCursor;
class QMenu;

namespace MusicCore {
    class Staff;
}

/**
 * Tool that provides functionality to insert/remove notes/rests. Named after Finale's Simple Entry tool.
 */
class SimpleEntryTool : public KoTool
{
    Q_OBJECT
public:
    explicit SimpleEntryTool( KoCanvasBase* canvas );
    ~SimpleEntryTool();

    virtual void paint( QPainter& painter, const KoViewConverter& converter );

    virtual void mousePressEvent( KoPointerEvent* event ) ;
    virtual void mouseMoveEvent( KoPointerEvent* event );
    virtual void mouseReleaseEvent( KoPointerEvent* event );

    virtual void keyPressEvent( QKeyEvent *event );

    void activate (bool temporary=false);
    void deactivate();

    void addCommand(QUndoCommand* command);

    MusicShape* shape();
    int voice();
    
    void setSelection(int startBar, int endBar, MusicCore::Staff* startStaff, MusicCore::Staff* endStaff);
protected:
    virtual QWidget * createOptionWidget();
protected slots:
    void activeActionChanged(QAction* action);
    void voiceChanged(int voice);
    void addBars();
    void actionTriggered();
    void importSheet();
    void exportSheet();
private:
    MusicShape *m_musicshape;
    AbstractMusicAction* m_activeAction;
    QPointF m_point;
    int m_voice;

    MusicCore::Staff* m_contextMenuStaff;
    int m_contextMenuBar;
    QPointF m_contextMenuPoint;

    int m_selectionStart, m_selectionEnd;
    MusicCore::Staff *m_selectionStaffStart, *m_selectionStaffEnd;

    MusicCursor* m_cursor;
    QList<QMenu*> m_menus; 
};

#endif

