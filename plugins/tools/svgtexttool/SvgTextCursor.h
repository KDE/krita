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

class SvgTextCursor : public KoToolSelection
{
    Q_OBJECT
public:
    explicit SvgTextCursor(SvgTextTool *tool);

    ~SvgTextCursor();

    void setShape(KoSvgTextShape *textShape);
    void setCaretSetting(int cursorWidth = 1, int cursorFlash = 1000, int cursorFlashLimit = 5000);

    int getPos();
    int getAnchor();

    void setPosToPoint(QPointF point);

    void moveCursor(bool forward, bool moveAnchor = true);

    void insertText(QString text);
    void removeLast();
    void removeNext();

    void paintDecorations(QPainter &gc, QColor selectionColor);

    bool hasSelection() override;
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
