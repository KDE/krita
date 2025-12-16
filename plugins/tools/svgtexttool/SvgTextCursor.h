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

    /// Handles used by type setting mode.
    enum TypeSettingModeHandle {
        NoHandle,
        StartPos,
        EndPos,

        /// The text properties handles.
        BaselineShift,
        Ascender,
        Descender,
        LineHeightTop,
        LineHeightBottom,

        /// Baselines.
        BaselineAlphabetic,
        BaselineIdeographic,
        BaselineMiddle,
        BaselineHanging,
        BaselineMathematical,
        BaselineCentral
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
    void setCaretSetting(int cursorWidth = 1, int cursorFlash = 1000, int cursorFlashLimit = 5000, bool drawCursorInAdditionToSelection = false);

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

    /// Set type setting mode active.
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

    /// Set a given typesetting handle as hovered, so it will be drawn as such.
    void setTypeSettingHandleHovered(TypeSettingModeHandle hovered = TypeSettingModeHandle::NoHandle);

    /// Whether to draw the type setting handles.
    /// Turned off when the typesetting strategy is active to give artists more control.
    void setDrawTypeSettingHandle(bool draw);

    /// Update the type setting decorations.
    void updateTypeSettingDecorFromShape();

    /// Return appropriate typeSetting cursor;
    QCursor cursorTypeForTypeSetting() const;

    /**
     * @brief handleName
     * @return translated name of a given handle.
     */
    QString handleName(TypeSettingModeHandle handle) const;

    /**
     * @brief setDominantBaselineFromHandle
     * Set the dominant baseline from a given handle.
     * @return true if dominant baseline was set, false if the handle doesn't
     * correspond to a baseline.
     */
    bool setDominantBaselineFromHandle(const TypeSettingModeHandle handle);

    /**
     * @brief posForHandleAndRect
     * Returns the closest cursor position for a given region and typesetting handle.
     * Used by the type setting mode to find the relevant metrics to scale.
     * @return cursor pos closest.
     */
    int posForTypeSettingHandleAndRect(const TypeSettingModeHandle handle, const QRectF regionOfInterest);

    /// Move the cursor, and, if you don't want a selection, move the anchor.
    void moveCursor(MoveMode mode, bool moveAnchor = true);

    /// Insert text at getPos()
    void insertText(QString text);

    /// Insert rich text at getPos();
    void insertRichText(KoSvgTextShape *insert, bool inheritPropertiesIfPossible = false);

    /**
     * @brief removeText
     * remove text relative to the current position.
     * This will move the cursor according to the move modes and then
     * remove the text between the two positions.
     * @param first how the cursor should move to get to the start position.
     * @param second how the cursor should move to get to the end position.
     */
    void removeText(MoveMode first, MoveMode second);

    /**
     * @brief removeLastCodePoint
     * Special function to remove the last code point. Triggered by backspace.
     * This is distinct from remove text, as some clusters have multiple code
     * points, but it is generally expected backspace deletes the codepoints
     * while delete with selection deletes the whole cluster.
     */
    void removeLastCodePoint();

    /**
     * @brief currentTextProperties
     * @return a qpair, where the first is the properties without inheritance, and the second, with inheritance.
     */
    QPair<KoSvgTextProperties, KoSvgTextProperties> currentTextProperties() const;

    /**
     * @brief propertiesForRange
     * @return properties for the current range defined by the cursor pos and anchor.
     */
    QList<KoSvgTextProperties> propertiesForRange() const;
    /**
     * @brief propertiesForShape
     * @return properties for the current shape.
     */
    QList<KoSvgTextProperties> propertiesForShape() const;

    /**
     * @brief mergePropertiesIntoSelection
     * Within Krita's SVG/CSS text system, it is possible to apply incomplete
     * properties to a whole range. In that case, only the existing properties
     * are applied. Properties can also be removed this way.
     *
     * @param props -- properties to apply.
     * @param removeProperties -- properties to be removed.
     * @param paragraphOnly -- whether to apply to the paragraph.
     * @param selectWord -- whether to select the word if there's no selection.
     */
    void mergePropertiesIntoSelection(const KoSvgTextProperties props, const QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>(), bool paragraphOnly = false, bool selectWord = false);

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

    /// Deselect all text. This effectively makes anchor the same as pos.
    void deselectText();

    /// Paint all decorations and blinkingcursors.
    void paintDecorations(QPainter &gc, QColor selectionColor, int decorationThickness = 1, qreal handleRadius = 5.0);

    /// Process an input method query and return the requested result.
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// Process an input method event. This is used by IME like virtual keyboards.
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

    void updateModifiers(const Qt::KeyboardModifiers modifiers);

    /// the cursor is currently adding a command
    bool isAddingCommand() const;

    /// Turns on blinking cursor.
    void focusIn();

    /// Stops blinking cursor.
    void focusOut();

    /// Register an action.
    bool registerPropertyAction(QAction *action, const QString &name);

    /// The text properties interface. This is how the text properties docker
    /// communicates with the text tool.
    KoSvgTextPropertiesInterface *textPropertyInterface();

Q_SIGNALS:

    /// Sents an update to the parent tool to update it's decorations.
    void updateCursorDecoration(QRectF updateRect);
    /// Sents an update selection was changed.
    void selectionChanged();

    /// Called by actions, tells the parent tool to open the glyph palette.
    void sigOpenGlyphPalette();
private Q_SLOTS:
    /// Called by timer, toggles the text cursor visible or invisible.
    void blinkCursor();
    /// Called by timer, stops the text blinking animation.
    void stopBlinkCursor();

    /*
     *  Called when the canvas view navigation changes,
     *  so we can ensure the input method widgets get aligned.
     */
    void updateInputMethodItemTransform();
    /// Called when the canvas resources (foreground/background) change.
    void canvasResourceChanged(int key, const QVariant &value);
    /// Called by the actions to execute a property change based on their data.
    void propertyAction();
    /// Called by the clear formatting action.
    void clearFormattingAction();

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

    /**
     * @brief removeTransformsFromRange
     * Called by actions to remove svg character transforms from range.
     */
    void removeTransformsFromRange();

    /// Update the canvas resources with fore and background color.
    void updateCanvasResources();

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

    /// Adds a command to the canvas of the parent tool.
    void addCommandToUndoAdapter(KUndo2Command *cmd);

    /// Processes a move action, returns the input.
    int moveModeResult(const MoveMode mode, int &pos, bool visual = false) const;

    /// Tests whether the current keyboard input can be printed as text, or is
    /// probably a shortcut. This is so that various keyboard events,
    /// like print don't get inserted as text.
    bool acceptableInput(const QKeyEvent *event) const;

    /// This applies any running IME interactions, used when the shape is
    /// deselected halfways through an IME interaction.
    void commitIMEPreEdit();

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
    virtual QList<KoSvgTextProperties> getCharacterProperties() override;
    virtual KoSvgTextProperties getInheritedProperties() override;
    virtual void setPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) override;
    virtual void setCharacterPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) override;
    virtual bool spanSelection() override;
    virtual bool characterPropertiesEnabled() override;
    void emitSelectionChange();
    void emitCharacterSelectionChange();
private:
    struct Private;
    const QScopedPointer<Private> d;
};

Q_DECLARE_METATYPE(SvgTextCursor::MoveMode)


#endif // SVGTEXTCURSOR_H
