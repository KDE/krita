/* This file is part of the KDE project
 *
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SVG_TEXT_TOOL
#define SVG_TEXT_TOOL

#include "ui_WdgSvgTextOptionWidget.h"

#include <KConfigGroup>
#include <KoToolBase.h>
#include <QFontComboBox>
#include <QPointer>
#include <QDoubleSpinBox>

#include <kis_signal_auto_connection.h>

#include "SvgTextCursor.h"
#include "glyphpalette/GlyphPaletteDialog.h"

#include <memory>

class KoSelection;
class SvgTextEditor;
class KoSvgTextShape;
class SvgTextCursor;
class KoInteractionStrategy;
class KUndo2Command;

class SvgTextTool : public KoToolBase
{
    Q_OBJECT

    friend class SvgCreateTextStrategy;

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

    QFont defaultFont() const;
    Qt::Alignment horizontalAlign() const;
    int writingMode() const;
    bool isRtl() const;

private Q_SLOTS:

    void showEditor();
    void showEditorSvgSource();
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
     * css and css strings assigned. This allows the artist
     * to select settings that new texts will be created with.
     * @return a string containing the defs.
     */
    QString generateDefs(const QString &extraProperties = QString());

    /**
     * @brief storeDefaults
     * store default font and point size when they change.
     */
    void storeDefaults();

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

    void slotConvertToPreformatted();
    void slotConvertToInlineSize();
    void slotConvertToSVGCharTransforms();


private:
    enum class DragMode {
        None = 0,
        Create,
        Select,
        InlineSizeHandle,
        Move,
    };
    enum class HighlightItem {
        None = 0,
        InlineSizeStartHandle,
        InlineSizeEndHandle,
        MoveBorder,
    };

    QPointer<SvgTextEditor> m_editor;
    QPointer<GlyphPaletteDialog> m_glyphPalette;
    QPointF m_lastMousePos;
    DragMode m_dragging {DragMode::None};
    std::unique_ptr<KoInteractionStrategy> m_interactionStrategy;
    HighlightItem m_highlightItem {HighlightItem::None};
    bool m_strategyAddingCommand {false};

    QButtonGroup *m_defAlignment {nullptr};
    QButtonGroup *m_defWritingMode {nullptr};
    QButtonGroup *m_defDirection {nullptr};
    KConfigGroup m_configGroup;
    SvgTextCursor m_textCursor;
    KisSignalAutoConnectionsStore m_canvasConnections;


    QPainterPath m_hoveredShapeHighlightRect;

    Ui_WdgSvgTextOptionWidget optionUi;

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
