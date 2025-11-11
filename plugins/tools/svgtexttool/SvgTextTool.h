/* This file is part of the KDE project
 *
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SVG_TEXT_TOOL
#define SVG_TEXT_TOOL


#include <KConfigGroup>
#include <KoToolBase.h>
#include <QPointer>

#include <KoSvgTextShapeOutlineHelper.h>

#include <kis_signal_auto_connection.h>
#include <KisSignalMapper.h>

#include "SvgTextCursor.h"
#include "SvgTextToolOptionsManager.h"
#include "SvgTextOnPathDecorationHelper.h"
#include "glyphpalette/GlyphPaletteDialog.h"

#include <memory>

class KoSelection;
class SvgTextEditor;
class KoSvgTextShape;
class KoInteractionStrategy;
class KUndo2Command;
class QActionGroup;

class SvgTextTool : public KoToolBase
{
    Q_OBJECT

    friend class SvgCreateTextStrategy;
    friend class SvgChangeTextPathInfoStrategy;

public:
    explicit SvgTextTool(KoCanvasBase *canvas);
    ~SvgTextTool() override;
    /// reimplemented from KoToolBase
    QRectF decorationsRect() const override;
    /// reimplemented from KoToolBase
    void paint(QPainter &gc, const KoViewConverter &converter) override;
    /// reimplemented from KoToolBase
    void mousePressEvent(KoPointerEvent *event) override;
    /// reimplemented from superclass
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseTripleClickEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseMoveEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseReleaseEvent(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    /// reimplemented from KoToolBase
    void activate(const QSet<KoShape *> &shapes) override;
    /// reimplemented from KoToolBase
    void deactivate() override;

    KisPopupWidgetInterface* popupWidget() override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void inputMethodEvent(QInputMethodEvent *event) override;

    /// reimplemented from superclass
    void copy() const override;
    /// reimplemented from superclass
    void deleteSelection() override;
    /// reimplemented from superclass
    bool paste() override;
    /// reimplemented from superclass
    bool hasSelection() override;

    bool selectAll() override;

    void deselect() override;
    /// reimplemented from superclass
    KoToolSelection * selection() override;
    
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;

protected:
    /// reimplemented from KoToolBase
    virtual QWidget *createOptionWidget() override;

    KoSelection *koSelection() const;
    KoSvgTextShape *selectedShape() const;

private:
    qreal grabSensitivityInPt() const;

    KoSvgText::WritingMode writingMode() const;

    void addMappedAction(KisSignalMapper* mapper, const QString &actionName, const int value, QActionGroup *group = nullptr);

private Q_SLOTS:

    void showEditor();
    void slotTextEditorClosed();
    void textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs);

    /**
     * @brief showGlyphPalette
     * Shows the glyph palette dialog.
     */
    void showGlyphPalette();
    /**
     * @brief updateGlyphPalette
     * update the glyph palette dialog from the current selection.
     */
    void updateGlyphPalette();

    void updateTextPathHelper();
    /**
     * @brief insertRichText
     * Insert a rich text shape, used by the glyph palette..
     * @param richText -- rich text shape.
     * @param replaceLastGlyph -- whether to replace the last glyph or to fully insert.
     */
    void insertRichText(KoSvgTextShape *richText, bool replaceLastGlyph = false);

    /**
     * @brief generateDefs
     * This generates a defs section with the appropriate
     * css and css strings assigned.
     */
    QString generateDefs(const KoSvgTextProperties &properties = KoSvgTextProperties());

    /**
     * @brief propertiesForNewText
     * get the text properties that should be used for new text.
     */
    KoSvgTextProperties propertiesForNewText() const;

    /**
     * @brief selectionChanged
     * called when the canvas selection is changed.
     */
    void slotShapeSelectionChanged();

    /**
     * @brief updateCursor
     * update the canvas decorations in a particular update rect for the text cursor.
     * @param updateRect the rect to update in.
     */
    void slotUpdateCursorDecoration(QRectF updateRect);

    /**
     * @brief slotConvertType
     * @param index
     */
    void slotConvertType(int index);

    /**
     * @brief slotUpdateVisualCursor
     * update the visual cursor mode on the text cursor.
     */
    void slotUpdateVisualCursor();

    /**
     * @brief slotUpdateTextPasteBehaviour
     * update the default text paste behaviour.
     */
    void slotUpdateTextPasteBehaviour();

    /**
     * @brief slotTextTypeUpdated
     * Update the text type in the tool options.
     */
    void slotTextTypeUpdated();

    /**
     * @brief slotMoveTextSelection
     * Move the start of the selection in typesetting mode by image 1 pix.
     * @param index -- Qt key for a direction.
     */
    void slotMoveTextSelection(int index);

    /**
     * @brief slotUpdateTypeSettingMode
     * Enable typesetting mode from the tool options.
     */
    void slotUpdateTypeSettingMode();

private:
    enum class DragMode {
        None = 0,
        Create,
        Select,
        InlineSizeHandle,
        Move,
        TextPathHandle,
        InShapeOffset,
    };
    enum class HighlightItem {
        None = 0,
        InlineSizeStartHandle,
        InlineSizeEndHandle,
        MoveBorder,
        TypeSettingHandle,
    };

    QScopedPointer<SvgTextToolOptionsManager>m_optionManager;
    QPointer<SvgTextEditor> m_editor;
    QPointer<GlyphPaletteDialog> m_glyphPalette;
    QPointF m_lastMousePos;
    DragMode m_dragging {DragMode::None};
    std::unique_ptr<KoInteractionStrategy> m_interactionStrategy;
    HighlightItem m_highlightItem {HighlightItem::None};
    bool m_strategyAddingCommand {false};

    QScopedPointer<KisSignalMapper> m_textTypeSignalsMapper;
    QScopedPointer<KisSignalMapper> m_typeSettingMovementMapper;


    SvgTextCursor m_textCursor;
    SvgTextOnPathDecorationHelper m_textOnPathHelper;
    QScopedPointer<KoSvgTextShapeOutlineHelper> m_textOutlineHelper;
    KisSignalAutoConnectionsStore m_canvasConnections;

    QPainterPath m_hoveredShapeHighlightRect;

    QCursor m_base_cursor;
    QCursor m_text_inline_horizontal;
    QCursor m_text_inline_vertical;
    QCursor m_text_on_path;
    QCursor m_text_in_shape;
    QCursor m_ibeam_vertical;
    QCursor m_ibeam_horizontal;
    QCursor m_ibeam_horizontal_done;

};

#endif
