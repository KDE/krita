/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#include "TextShape.h"
#include "ShapeSpecificData.h"
#include "Ruler.h"

#include <KoTool.h>

#include <QTextBlock>

class TextShape;

class KoParagraphStyle;
class KoCanvasBase;

class QAbstractTextDocumentLayout;
class QColor;
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

    void initializeRuler(Ruler *ruler, int options = 0);

    // reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    // select the text block and the given textShape
    void selectTextBlock(TextShape *textShape, QTextBlock textBlock);

    // deselect the current text block (for example use clicked somwhere else)
    void deselectTextBlock();

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

    // Apply the parent's style to the active ruler (essentially deletes the setting)
    void applyParentStyleToActiveRuler();

    // if the parargraph style is inherited from a named paragraph style return that name
    // otherwise return QString()
    QString styleName();

    void toggleSmoothMovement() {m_smoothMovement = !m_smoothMovement; emit smoothMovementChanged(m_smoothMovement); }
    void setSmoothMovement(bool smoothMovement) { m_smoothMovement = smoothMovement; }

signals:
    void styleNameChanged(const QString&);
    void smoothMovementChanged(bool smooth);

protected:
    QWidget *createOptionWidget();

    // get all necessary properties from the layout system and pass them to the controls
    void loadDimensions();
    void loadRulers();
    void saveRulers();

    void activateRuler(Ruler *ruler);
    void deactivateActiveRuler();
    void resetActiveRuler();

    bool smoothMovement() { return m_smoothMovement; }

    // paint a label at the specified position
    void paintLabel(QPainter &painter, const QMatrix &matrix, const Ruler *ruler) const;

    void paintRulers(QPainter &painter) const;

    // internal convencience methods
    const QTextDocument *document() const { return textBlock().document(); }
    QTextBlock textBlock() const { Q_ASSERT(m_textBlockValid); return m_block; }
    QTextBlockFormat blockFormat() const { return textBlock().blockFormat(); }
    QTextCharFormat charFormat() const { return textBlock().charFormat(); }
    QTextLayout *textLayout() const { return textBlock().layout(); }

private:
    // this single member will be replaced by a list with a ShapeSpecificData for each shape that contains
    // the active text block
    ShapeSpecificData m_shapeSpecificData;

    QTextBlock m_block;
    KoParagraphStyle *m_paragraphStyle;

    QPointF m_mousePosition;

    // should move to ShapeSpecificData
    QRectF m_counter,
           m_firstLine,
           m_followingLines,
           m_border;

    bool m_singleLine,
         m_isList,
         m_textBlockValid,
         m_needsRepaint,
         m_smoothMovement;

    Ruler m_firstIndentRuler,
          m_followingIndentRuler,
          m_rightMarginRuler,
          m_topMarginRuler,
          m_bottomMarginRuler;

    Ruler *m_activeRuler,
          *m_hoverRuler;
};

#endif

