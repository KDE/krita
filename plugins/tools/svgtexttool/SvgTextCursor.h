/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SVGTEXTCURSOR_H
#define SVGTEXTCURSOR_H

#include <KoSvgTextShape.h>
#include <KoToolSelection.h>
#include <QPainter>

class SvgTextTool;
class SvgTextInsertCommand;
class SvgTextRemoveCommand;
class KUndo2Command;

class SvgTextCursor : public KoToolSelection
{
    Q_OBJECT
public:
    explicit SvgTextCursor(SvgTextTool *tool);

    enum MoveMode {
        MoveNone,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        MoveWordLeft,
        MoveWordRight,
        MoveLineStart,
        MoveLineEnd,
        ParagraphStart,
        ParagraphEnd,
    };

    ~SvgTextCursor();

    void setShape(KoSvgTextShape *textShape);
    void setCaretSetting(int cursorWidth = 1, int cursorFlash = 1000, int cursorFlashLimit = 5000);

    int getPos();
    int getAnchor();

    void setPos(int pos, int anchor);

    void setPosToPoint(QPointF point);

    void moveCursor(MoveMode mode, bool moveAnchor = true);

    void insertText(QString text);
    void removeLast();
    void removeNext();

    /**
     * @brief removeSelection
     * if there's a selection, creates a text-removal command.
     * @param parent
     * @return the text-removal command, if possible, if there's no selection or shape, it'll return 0;
     */
    SvgTextRemoveCommand *removeSelection(KUndo2Command *parent = 0);

    /**
     * @brief copy
     * copies plain text into the clipboard between anchor and pos.
     */
    void copy() const;
    /**
     * @brief paste
     * @return pastes plain text in the clipboard at pos.
     */
    bool paste();

    void paintDecorations(QPainter &gc, QColor selectionColor);

    // Reimplemented.
    bool hasSelection() override;

Q_SIGNALS:

    void updateCursorDecoration(QRectF updateRect);
private Q_SLOTS:
    void blinkCursor();
    void stopBlinkCursor();

private:

    void updateCursor();
    void updateSelection();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // SVGTEXTCURSOR_H
