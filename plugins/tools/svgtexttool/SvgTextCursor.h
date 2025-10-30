/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SVGTEXTCURSOR_H
#define SVGTEXTCURSOR_H

#include <KoSvgTextShape.h>
#include <KoSvgTextProperties.h>
#include <KoSvgTextPropertiesInterface.h>
#include <KoToolSelection.h>
#include <QPainter>
#include <KoShape.h>
#include "kritatoolsvgtext_export.h"

class KoCanvasBase;
class SvgTextInsertCommand;
class SvgTextRemoveCommand;
class KUndo2Command;
class QKeyEvent;
class QInputMethodEvent;
class QAction;

/**
 * @brief The SvgTextCursor class
 *
 * This class handles cursor movement and text editing operations.
 *
 * It acts as the KoToolSelection for SvgTextTool, allowing it to
 * integrate with the basic KoToolBase functionality for copy, cut
 * paste and clear.
 *
 * A selection is defined as the anchor being different from the cursor
 * position, with the move operation accepting whether you want to shift
 * the cursor position.
 *
 * It is also a shape listener to allow the textcursor to update itself
 * whenever the corresponding text shape changes.
 */

class KRITATOOLSVGTEXT_EXPORT SvgTextCursor : public KoToolSelection, public KoSvgTextShape::TextCursorChangeListener
{
    Q_OBJECT
public:
    explicit SvgTextCursor(KoCanvasBase *canvas);

    enum MoveMode {
        MoveNone,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        MoveNextChar,
        MovePreviousChar,
        MoveNextLine,
        MovePreviousLine,
        MoveWordLeft,
        MoveWordRight,
        MoveWordEnd,
        MoveWordStart,
        MoveLineStart,
        MoveLineEnd,
        ParagraphStart,
        ParagraphEnd,
    };

    enum TypeSettingModeHandle {
        NoHandle,
        StartPos,
        EndPos,
        Ascender,
        BaselineShift,
        Descender,

        BaselineAlphabetic,
        BaselineIdeographic,
        BaselineMiddle,
        BaselineHanging,
        BaselineMathematic
    };

    ~SvgTextCursor();

    /**
     * @brief Get the current text shape
     * @return KoSvgTextShape *
     */
    KoSvgTextShape *shape() const;

    /**
     * @brief setShape
     * @param textShape KoSvgTextShape to set, is allowed to be a nullptr, the cursor just won't do anything.
     */
    void setShape(KoSvgTextShape *textShape);

    /**
     * @brief setCaretSetting
     * Set the caret settings for the cursor. Qt has some standard functionality associated, which we pass via this.
     * @param cursorWidth - Cursor width from the style.
     * @param cursorFlash - the total time it takes for a cursor to hide reapear.
     * @param cursorFlashLimit - maximum amount of time a cursor is allowed to flash.
     */
    void setCaretSetting(int cursorWidth = 1, int cursorFlash = 1000, int cursorFlashLimit = 5000);

    /**
     * @brief setVisualMode
     * set whether the navigation mode is visual or logical.
     * This right now primarily affects Bidirectional text.
     * @param mode whether to turn off visual mode.
     */
    void setVisualMode(const bool visualMode = true);

    /**
     * @brief setPasteRichText
     * @param pasteRichText -- whether to paste rich text when possible.
     */
    void setPasteRichTextByDefault(const bool pasteRichText = true);

    void setTypeSettingModeActive(bool activate);

    /// Get the current position.
    int getPos();

    /// Get the current selection anchor. This is the same as position, unless there's a selection.
    int getAnchor();

    /// Set the pos and the anchor.
    void setPos(int pos, int anchor);

    /// Set the pos from a point. This currently does a search inside the text shape.
    void setPosToPoint(QPointF point, bool moveAnchor = true);

    /// Get typeSettingMode handle for text;
    TypeSettingModeHandle typeSettingHandleAtPos(const QRectF regionOfInterest);
    void setTypeSettingHandleHovered(TypeSettingModeHandle hovered = TypeSettingModeHandle::NoHandle);
    void setDrawTypeSettingHandle(bool draw);

    /// Move the cursor, and, if you don't want a selection, move the anchor.
    void moveCursor(MoveMode mode, bool moveAnchor = true);

    /// Insert text at getPos()
    void insertText(QString text);

    /// Insert rich text at getPos();
    void insertRichText(KoSvgTextShape *insert);

    /**
     * @brief removeText
     * remove text relative to the current position.
     * This will move the cursor according to the move modes and then
     * remove the text between the two positions.
     * @param first how the cursor should move to get to the start position.
     * @param second how the cursor should move to get to the end position.
     */
    void removeText(MoveMode first, MoveMode second);

    void removeLastCodePoint();

    /**
     * @brief currentTextProperties
     * @return a qpair, where the first is the properties without inheritance, and the second, with inheritance.
     */
    QPair<KoSvgTextProperties, KoSvgTextProperties> currentTextProperties() const;

    QList<KoSvgTextProperties> propertiesForRange() const;

    void mergePropertiesIntoSelection(const KoSvgTextProperties props, const QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>());

    /**
     * @brief removeSelection
     * if there's a selection, creates a text-removal command.
     * @param parent
     * @return the text-removal command, if possible, if there's no selection or shape, it'll return 0;
     */
    void removeSelection();

    /**
     * @brief copy
     * copies plain text into the clipboard between anchor and pos.
     */
    void copy() const;
    /**
     * @brief paste
     * pastes plain text in the clipboard at pos.
     * Uses pasteRichTextByDefault to determine whether
     * to try and paste rich text.
     * @return true when successfull.
     */
    bool paste();


    void deselectText();

    void paintDecorations(QPainter &gc, QColor selectionColor, int decorationThickness = 1, qreal handleRadius = 5.0);

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    void inputMethodEvent(QInputMethodEvent *event);

    // Reimplemented.
    bool hasSelection() override;

    /// ShapeChangeListener reimplementation. This will update the cursor position
    /// when the shape was updated.
    void notifyShapeChanged(KoShape::ChangeType type, KoShape *shape) override;

    /// TextCursorChangeListener reimplementation, this allows undo commands
    /// to update the cursor without having the cursor owned by the command.
    void notifyCursorPosChanged(int pos, int anchor) override;

    void notifyMarkupChanged() override;

    /// Handle the cursor-related key events.
    void keyPressEvent(QKeyEvent *event);

    /// the cursor is currently adding a command
    bool isAddingCommand() const;

    /// Turns on blinking cursor.
    void focusIn();

    /// Stops blinking cursor.
    void focusOut();

    /// Register an action.
    bool registerPropertyAction(QAction *action, const QString &name);

    KoSvgTextPropertiesInterface *textPropertyInterface();

Q_SIGNALS:

    void updateCursorDecoration(QRectF updateRect);
    void selectionChanged();

    void sigOpenGlyphPalette();
private Q_SLOTS:
    void blinkCursor();
    void stopBlinkCursor();

    void updateInputMethodItemTransform();
    void canvasResourceChanged(int key, const QVariant &value);
    void toggleProperty(KoSvgTextProperties::PropertyId property);
    void propertyAction();

    /**
     * @brief pasteRichText
     * @return try to paste rich text at pos.
     */
    bool pasteRichText();

    /**
     * @brief pastePlainText
     * Explicitely paste plaintext at pos.
     * @return true when successfull.
     */
    bool pastePlainText();

private:

    /**
     * @brief removeSelection
     * if there's a selection, creates a text-removal command.
     * @param parent
     * @return the text-removal command, if possible, if there's no selection or shape, it'll return 0;
     */
    SvgTextRemoveCommand *removeSelectionImpl(bool allowCleanUp, KUndo2Command *parent = 0);


    /// update the cursor shape. First update will block ensuring the canvas is visible so setShape won't cause this.
    void updateCursor(bool firstUpdate = false);
    void updateSelection();
    void updateIMEDecoration();
    void updateTypeSettingDecoration();
    void addCommandToUndoAdapter(KUndo2Command *cmd);

    int moveModeResult(MoveMode &mode, int &pos, bool visual = false) const;
    bool acceptableInput(const QKeyEvent *event) const;

    void commitIMEPreEdit();

    void updateCanvasResources();

    struct Private;
    const QScopedPointer<Private> d;
};

/// Interface to interact with the text property manager.
class KRITATOOLSVGTEXT_EXPORT SvgTextCursorPropertyInterface : public KoSvgTextPropertiesInterface
{
public:
    SvgTextCursorPropertyInterface(SvgTextCursor *parent);
    ~SvgTextCursorPropertyInterface();
    virtual QList<KoSvgTextProperties> getSelectedProperties() override;
    virtual KoSvgTextProperties getInheritedProperties() override;
    virtual void setPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) override;
    virtual bool spanSelection() override;
    void emitSelectionChange();
private:
    struct Private;
    const QScopedPointer<Private> d;
};

Q_DECLARE_METATYPE(SvgTextCursor::MoveMode)


#endif // SVGTEXTCURSOR_H
