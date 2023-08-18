/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTCURSOR_H
#define KOSVGTEXTCURSOR_H

#include <KoSvgTextShape.h>
#include <QObject>
#include <QPainter>

class KRITAFLAKE_EXPORT KoSvgTextCursor : public QObject
{
    Q_OBJECT
public:
    explicit KoSvgTextCursor(QObject *parent, KoSvgTextShape *textShape, int cursorWidth = 1, int cursorFlash = 1000, int cursorFlashLimit = 5000);

    ~KoSvgTextCursor();

    int getPos();
    int getAnchor();

    void setPosToPoint(QPointF point);

    void moveCursor(bool forward, bool moveAnchor = true);

    void insertText(QString text);
    void removeLast();
    void removeNext();

    void paintDecorations(QPainter &gc, QColor selectionColor);
Q_SIGNALS:
    void decorationsChanged(QRectF);

public Q_SLOTS:
    void blinkCursor();
    void stopBlinkCursor();
private:

    void updateCursor();
    void updateSelection();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KOSVGTEXTCURSOR_H
