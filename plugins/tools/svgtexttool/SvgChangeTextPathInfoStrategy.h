/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGCHANGETEXTPATHINFOSTRATEGY_H
#define SVGCHANGETEXTPATHINFOSTRATEGY_H

#include <KoInteractionStrategy.h>

#include <KoSvgTextShape.h>
#include <KoSvgText.h>


class SvgTextTool;

class SvgChangeTextPathInfoStrategy : public KoInteractionStrategy
{
public:
    SvgChangeTextPathInfoStrategy(SvgTextTool *tool, KoSvgTextShape *shape, const QPointF &clicked, int textCursorPos);
    ~SvgChangeTextPathInfoStrategy() override = default;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void cancelInteraction() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
private:
    KoSvgTextShape *m_shape;
    QPointF m_currentMousePos;
    int m_textCursorPos;
    KoSvgText::TextOnPathInfo m_oldInfo;
};

#endif // SVGCHANGETEXTPATHINFOSTRATEGY_H
