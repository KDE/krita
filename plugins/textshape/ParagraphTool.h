/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#ifndef PARAGRAPHTOOL_H
#define PARAGRAPHTOOL_H

#include "ShapeSpecificData.h"
#include "Ruler.h"

#include <KoTool.h>

#include <QTextCursor>
#include <QTextBlock>
#include <QVector>

class TextShape;

class KoParagraphStyle;
class KoCanvasBase;

class QAbstractTextDocumentLayout;
class QTextLayout;

/**
 * This is the tool for editing paragraph formatting
 * It displays all paragraph formatting parameters directly on the canvas
 * and allows to modify them, too.
 */
class ParagraphTool : public KoTool {
    Q_OBJECT
public:
    explicit ParagraphTool(KoCanvasBase *canvas);
    ~ParagraphTool();

    void initializeRuler(Ruler &ruler, int options = 0);

    // reimplemented from superclass
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    // reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event );

    // reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );

    // reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );

    // reimplemented from superclass
    virtual void keyPressEvent( QKeyEvent *event );

    // reimplemented from superclass
    virtual void keyReleaseEvent( QKeyEvent *event );

    // reimplemented from superclass
    virtual void activate (bool temporary=false);

    // reimplemented from superclass
    virtual void deactivate();

    virtual void repaintDecorations();

public slots:
    // should be called when any of the rulers needs a repaint
    void scheduleRepaint();
    
    // should be called when the value of any of the rulers changed
    void updateLayout();

    // Apply the parent's style to the active ruler
    // (essentially deletes the setting)
    void applyParentStyleToActiveRuler();

    // if the parargraph style is inherited from a named paragraph style
    // return that name, otherwise return QString()
    QString styleName();

    void toggleSmoothMovement() { m_smoothMovement = !m_smoothMovement; emit smoothMovementChanged(m_smoothMovement); }

    void setSmoothMovement(bool smoothMovement) { m_smoothMovement = smoothMovement; }

signals:
    void styleNameChanged(const QString&);
    void smoothMovementChanged(bool smooth);

protected:
    QWidget *createOptionWidget();

    void loadRulers();
    void saveRulers();

    // paint a label at the specified position
    void paintLabel(QPainter &painter, const KoViewConverter &converter) const;

    // paint all rulers for a given shape
    void paintRulers(QPainter &painter, const KoViewConverter &converter, const ShapeSpecificData &shape) const;

    bool createShapeList();

    // activate the paragraph below the specified cursor position
    bool activateTextBlockAt(const QPointF &point);

    // deactivate the current text block
    // (for example use clicked somwhere else)
    void deactivateTextBlock();

    bool hasActiveTextBlock() const { return !m_activeCursor.isNull(); }

    bool hasActiveRuler() const { return m_activeRuler != noRuler; }
    void activateRuler(RulerIndex ruler, const ShapeSpecificData &shape);
    bool activateRulerAt(const QPointF &point);
    void deactivateRuler();
    void resetActiveRuler();
    void moveActiveRulerTo(const QPointF &point);

    Ruler &focusedRuler() { return m_rulers[m_focusedRuler]; }
    bool hasFocusedRuler() const { return m_focusedRuler >= 0 && m_focusedRuler < maxRuler; }
    void focusRuler(RulerIndex ruler);
    void focusNextRuler() { focusRuler((RulerIndex)(((int)m_focusedRuler + 1)%maxRuler)); }
    void defocusRuler();

    bool hasHighlightedRuler() const {return m_highlightedRuler != noRuler;}
    void highlightRulerAt(const QPointF &point);
    void dehighlightRuler();

    bool smoothMovement() { return m_smoothMovement; }

    // internal convencience methods
    QTextBlock textBlock() const { Q_ASSERT(m_activeCursor.block().isValid()); return m_activeCursor.block(); }

    QTextBlockFormat blockFormat() const { return textBlock().blockFormat(); }
    QTextCharFormat charFormat() const { return textBlock().charFormat(); }
    QTextLayout *textLayout() const { return textBlock().layout(); }

private:
    QTextCursor m_activeCursor;
    KoParagraphStyle *m_paragraphStyle;

    QVector<ShapeSpecificData> m_shapes;

    const ShapeSpecificData *m_activeShape,
                            *m_highlightShape;

    Ruler m_rulers[maxRuler];

    RulerIndex m_activeRuler,
               m_focusedRuler,
               m_highlightedRuler;

    QPointF m_mousePosition;
    QRectF m_storedRepaintRectangle;

    bool m_needsRepaint,
         m_smoothMovement;

};

#endif

