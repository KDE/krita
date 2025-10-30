/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVGTEXTTYPESETTINGSTRATEGY_H
#define SVGTEXTTYPESETTINGSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <QPointF>
#include <KoSvgTextShape.h>

class SvgTextCursor;
class KoSvgTextShape;
class QRectF;
class QPointF;

/**
 * @brief The SvgTextTypeSettingStrategy class
 * This class encompasses the typesetting mode.
 */
class SvgTextTypeSettingStrategy: public KoInteractionStrategy
{
public:
    SvgTextTypeSettingStrategy(KoToolBase *tool, KoSvgTextShape *textShape, SvgTextCursor *textCursor, const QRectF &regionOfInterest);

    // KoInteractionStrategy interface
public:
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void cancelInteraction() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;

private:
    KoSvgTextShape *m_shape;
    QPointF m_dragStart;
    QPointF m_dragCurrent;
    QPointF m_currentDelta;

    int m_cursorPos;
    int m_cursorAnchor;
    int m_editingType;

    QScopedPointer<KUndo2Command> m_previousCmd;
};

#endif // SVGTEXTTYPESETTINGSTRATEGY_H
