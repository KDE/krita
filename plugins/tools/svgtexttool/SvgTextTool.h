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
#include <QPushButton>
#include <QFontComboBox>
#include <QPointer>
#include <QDoubleSpinBox>
#include <QTimer>

#include <kis_signal_auto_connection.h>

#include "SvgTextEditor.h"
#include "SvgTextCursor.h"

#include <memory>

class KoSelection;
class SvgTextEditor;
class KoSvgTextShape;
class SvgTextCursor;
class KoInteractionStrategy;

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
    void mouseMoveEvent(KoPointerEvent *event) override;
    /// reimplemented from KoToolBase
    void mouseReleaseEvent(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    /// reimplemented from KoToolBase
    void activate(const QSet<KoShape *> &shapes) override;
    /// reimplemented from KoToolBase
    void deactivate() override;

    KisPopupWidgetInterface* popupWidget() override;

    void updateCursor(QRectF updateRect);

protected:
    /// reimplemented from KoToolBase
    virtual QWidget *createOptionWidget() override;

    KoSelection *koSelection() const;
    KoSvgTextShape *selectedShape() const;

private:
    qreal grabSensitivityInPt() const;

    QFont defaultFont() const;
    Qt::Alignment horizontalAlign() const;

private Q_SLOTS:

    void showEditor();
    void showEditorSvgSource();
    void slotTextEditorClosed();
    void textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs, bool richTextUpdated);

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

    void selectionChanged();


private:
    enum class DragMode {
        None = 0,
        Create,
        InlineSizeHandle,
        Move,
    };
    enum class HighlightItem {
        None = 0,
        InlineSizeHandle,
        MoveBorder,
    };

    QPointer<SvgTextEditor> m_editor;
    QPointF m_lastMousePos;
    DragMode m_dragging {DragMode::None};
    std::unique_ptr<KoInteractionStrategy> m_interactionStrategy;
    HighlightItem m_highlightItem {HighlightItem::None};

    QButtonGroup *m_defAlignment {nullptr};
    KConfigGroup m_configGroup;
    SvgTextCursor m_textCursor;
    KisSignalAutoConnectionsStore m_canvasConnections;


    QPainterPath m_hoveredShapeHighlightRect;
    boost::optional<KoColor> m_originalColor { boost::none };


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
