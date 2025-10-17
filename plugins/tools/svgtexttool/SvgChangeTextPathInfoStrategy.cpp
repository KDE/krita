/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgChangeTextPathInfoStrategy.h"

#include "SvgTextPathInfoChangeCommand.h"
#include <KoPathSegment.h>
#include <KoPathShape.h>
#include <KoToolBase.h>
#include "SvgTextTool.h"
#include <qmath.h>
#include <QDebug>

SvgChangeTextPathInfoStrategy::SvgChangeTextPathInfoStrategy(SvgTextTool *tool, KoSvgTextShape *shape, const QPointF &clicked, int textCursorPos)
    :KoInteractionStrategy(tool)
    , m_shape(shape)
    , m_currentMousePos(clicked)
    , m_textCursorPos(textCursorPos)
{
    KoSvgTextNodeIndex index = m_shape->topLevelNodeForPos(m_textCursorPos);
    m_oldInfo = *(index.textPathInfo());

}

void SvgChangeTextPathInfoStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)

    m_currentMousePos = mouseLocation;

    KUndo2Command *cmd = createCommand();
    if (cmd) {
        cmd->redo();
    }
    tool()->repaintDecorations();
}

KUndo2Command *SvgChangeTextPathInfoStrategy::createCommand()
{
    SvgTextTool *const tool = qobject_cast<SvgTextTool *>(this->tool());
    KoSvgTextNodeIndex index = m_shape->topLevelNodeForPos(m_textCursorPos);
    KoShape *shape = index.textPath();
    if (!shape) {
        return nullptr;
    }

    KoPathShape *path = dynamic_cast<KoPathShape*>(shape);
    KoSvgText::TextOnPathInfo info;

    const qreal grab = tool->grabSensitivityInPt();
    /*QRectF roi = path->boundingRect();
    roi.adjust(-grab, -grab, grab, grab);*/
    KoPathSegment segment = path->segmentAtPoint(m_currentMousePos, tool->handleGrabRect(m_currentMousePos));

    if (!segment.isValid()) {
        return nullptr;
    }

    QList<KoPathSegment> segments = path->segmentsAt(path->outlineRect().adjusted(-grab, -grab, grab, grab));

    Q_FOREACH(KoPathSegment s, segments) {
        if (s == segment) {
            const qreal t = segment.nearestPoint(path->documentToShape(m_currentMousePos));
            info.startOffset += (t * segment.length());

            QPainterPath p = path->transformation().map(path->outline());
            qDebug() << info.startOffset/p.length();
            const qreal angle = p.angleAtPercent(fmod(info.startOffset/p.length(), 1.0))+90;
            if (fabs(QLineF(segment.pointAt(t), m_currentMousePos).angle() - angle) < 90) {
                info.side = KoSvgText::TextPathSideRight;
            } else {
                info.side = KoSvgText::TextPathSideLeft;
            }
            if (info.side == KoSvgText::TextPathSideRight) {
                info.startOffset = p.length() - info.startOffset;
            }
            break;
        }
        info.startOffset += s.length();
    }

    KUndo2Command *cmd = new SvgTextPathInfoChangeCommand(m_shape, m_textCursorPos, info);
    cmd->setText(kundo2_i18n("Change Text On Path Position"));
    return cmd;
}

void SvgChangeTextPathInfoStrategy::cancelInteraction()
{
    KUndo2Command *cmd = new SvgTextPathInfoChangeCommand(m_shape, m_textCursorPos, m_oldInfo);
    if (cmd) {
        cmd->undo();
    }
    tool()->repaintDecorations();
}

void SvgChangeTextPathInfoStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{

}
