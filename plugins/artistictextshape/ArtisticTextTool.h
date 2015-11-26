/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#ifndef ARTISTICTEXTTOOL_H
#define ARTISTICTEXTTOOL_H

#include "ArtisticTextShape.h"
#include "ArtisticTextToolSelection.h"

#include <KoToolBase.h>
#include <QTimer>

class QAction;
class QActionGroup;
class KoInteractionStrategy;

/// This is the tool for the artistic text shape.
class ArtisticTextTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit ArtisticTextTool(KoCanvasBase *canvas);
    ~ArtisticTextTool();

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented
    virtual void repaintDecorations();
    /// reimplemented
    virtual void mousePressEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseMoveEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void shortcutOverrideEvent(QKeyEvent *event);
    /// reimplemented
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes);
    /// reimplemented
    virtual void deactivate();
    /// reimplemented
    virtual QList<QPointer<QWidget> > createOptionWidgets();
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual KoToolSelection *selection();

    /// reimplemented from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;

    /// Sets cursor for specified text shape it is the current text shape
    void setTextCursor(ArtisticTextShape *textShape, int textCursor);

    /// Returns the current text cursor position
    int textCursor() const;

    /**
     * Determines cursor position from specified mouse position.
     * @param mousePosition mouse position in document coordinates
     * @return cursor position, -1 means invalid cursor
     */
    int cursorFromMousePosition(const QPointF &mousePosition);

protected:
    void enableTextCursor(bool enable);
    void removeFromTextCursor(int from, unsigned int count);
    void addToTextCursor(const QString &str);

private Q_SLOTS:
    void detachPath();
    void convertText();
    void blinkCursor();
    void textChanged();
    void shapeSelectionChanged();
    void setStartOffset(int offset);
    void toggleFontBold(bool enabled);
    void toggleFontItalic(bool enabled);
    void anchorChanged(QAction *);
    void setFontFamiliy(const QFont &font);
    void setFontSize(int size);
    void setSuperScript();
    void setSubScript();
    void selectAll();
    void deselectAll();

Q_SIGNALS:
    void shapeSelected();

private:
    void updateActions();
    void setTextCursorInternal(int textCursor);
    void createTextCursorShape();
    void updateTextCursorArea() const;
    void setCurrentShape(ArtisticTextShape *currentShape);

    enum FontProperty {
        BoldProperty,
        ItalicProperty,
        FamiliyProperty,
        SizeProperty
    };

    /// Changes the specified font property for the current text selection
    void changeFontProperty(FontProperty property, const QVariant &value);

    /// Toggle sub and super script
    void toggleSubSuperScript(ArtisticTextRange::BaselineShift mode);

    /// returns the transformation matrix for the text cursor
    QTransform cursorTransform() const;

    /// Returns the offset handle shape for the current text shape
    QPainterPath offsetHandleShape();

    ArtisticTextToolSelection m_selection; ///< the tools selection
    ArtisticTextShape *m_currentShape;  ///< the current text shape we are working on
    ArtisticTextShape *m_hoverText;     ///< the text shape the mouse cursor is hovering over
    KoPathShape *m_hoverPath;           ///< the path shape the mouse cursor is hovering over
    QPainterPath m_textCursorShape;     ///< our visual text cursor representation
    bool m_hoverHandle;

    QAction *m_detachPath;
    QAction *m_convertText;
    QAction *m_fontBold;
    QAction *m_fontItalic;
    QAction *m_superScript;
    QAction *m_subScript;
    QActionGroup *m_anchorGroup;

    int m_textCursor;
    QTimer m_blinkingCursor;
    bool m_showCursor;
    QList<QPointF> m_linefeedPositions; ///< offset positions for temporary line feeds
    KoInteractionStrategy *m_currentStrategy;
};

#endif // ARTISTICTEXTTOOL_H
