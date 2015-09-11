/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef ABSTRACTMUSICACTION_H
#define ABSTRACTMUSICACTION_H

#include <QAction>

class SimpleEntryTool;
class MusicCursor;
namespace MusicCore {
    class Staff;
}

class QIcon;

class AbstractMusicAction : public QAction
{
    Q_OBJECT
public:
    AbstractMusicAction(const QIcon& icon, const QString& text, SimpleEntryTool* tool);
    AbstractMusicAction(const QString& text, SimpleEntryTool* tool);

    virtual void renderPreview(QPainter& painter, const QPointF& point);
    virtual void mousePress(MusicCore::Staff* staff, int bar, const QPointF& pos) = 0;
    virtual void mouseMove(MusicCore::Staff* staff, int bar, const QPointF& pos);
    bool isVoiceAware();
    
    virtual void renderKeyboardPreview(QPainter& painter, const MusicCursor& cursor);
    virtual void keyPress(QKeyEvent* event, const MusicCursor& cursor);
protected:
    bool m_isVoiceAware;
    SimpleEntryTool* m_tool;
};

#endif // ABSTRACTMUSICACTION_H
