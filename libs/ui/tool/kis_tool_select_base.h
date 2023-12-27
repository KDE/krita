/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTOOLSELECTBASE_H
#define KISTOOLSELECTBASE_H

#include "KoPointerEvent.h"
#include "kis_tool.h"
#include "kis_canvas2.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_config_widget_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "kis_selection_modifier_mapper.h"
#include "strokes/move_stroke_strategy.h"
#include "kis_image.h"
#include "kis_cursor.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signal_auto_connection.h"
#include "kis_selection_tool_helper.h"
#include "kis_assert.h"
#include <input/kis_extended_modifiers_mapper.h>

/**
 * This is a basic template to create selection tools from basic path based drawing tools.
 * The template overrides the ability to execute alternate actions correctly.
 * The default behavior for the modifier keys is as follows:
 *
 * Shift: add to selection
 * Alt: subtract from selection
 * Shift+Alt: intersect current selection
 * Ctrl+Alt: symmetric difference
 * Ctrl: replace selection
 *
 * The mapping itself is done in KisSelectionModifierMapper.
 *
 * Certain tools also use modifier keys to alter their behavior, e.g. forcing square proportions with the rectangle tool.
 * The template enables the following rules for forwarding keys:

 * 1) If the user is not selecting, then changing the modifier combination
 *    changes the selection method.
 * 
 * 2) If the user is selecting then the modifier keys are forwarded to the
 *    specific tool, so that it can do with them whatever it wants. The selection
 *    method is not changed in this stage and it will be the same as just before
 *    the user started selecting.
 * 
 * 3) Once the user finishes selecting, the selection method is updated to reflect
 *    the current modifier combination
 * 
 * 4) If the user is moving the selection, then changing the modifiers 
 */

template <class BaseClass>
class KisToolSelectBase : public BaseClass
{

public:

    KisToolSelectBase(KoCanvasBase* canvas, const QString toolName)
        : BaseClass(canvas)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    KisToolSelectBase(KoCanvasBase* canvas, const QCursor cursor, const QString toolName)
        : BaseClass(canvas, cursor)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    KisToolSelectBase(KoCanvasBase* canvas, QCursor cursor, QString toolName, KoToolBase *delegateTool)
        : BaseClass(canvas, cursor, delegateTool)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    enum SampleLayersMode
    {
        SampleAllLayers,
        SampleCurrentLayer,
        SampleColorLabeledLayers,
    };

    void updateActionShortcutToolTips() {
        KisSelectionOptions *widget = m_widgetHelper.optionWidget();
        if (widget) {
            widget->updateActionButtonToolTip(
                SELECTION_REPLACE,
                this->action("selection_tool_mode_replace")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_ADD,
                this->action("selection_tool_mode_add")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_SUBTRACT,
                this->action("selection_tool_mode_subtract")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_INTERSECT,
                this->action("selection_tool_mode_intersect")->shortcut());
        }
    }

    void activate(const QSet<KoShape *> &shapes) override
    {
        BaseClass::activate(shapes);

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_replace"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotReplaceModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_add"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotAddModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_subtract"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotSubtractModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_intersect"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotIntersectModeRequested()));

        updateActionShortcutToolTips();

        if (m_widgetHelper.optionWidget()) {
            if (isPixelOnly()) {
                m_widgetHelper.optionWidget()->setModeSectionVisible(false);
                m_widgetHelper.optionWidget()->setAdjustmentsSectionVisible(
                    true);
            }
            m_widgetHelper.optionWidget()->setReferenceSectionVisible(
                usesColorLabels());
        }
    }

    void deactivate() override
    {
        BaseClass::deactivate();
        m_modeConnections.clear();
    }

    QWidget *createOptionWidget() override
    {
        m_widgetHelper.createOptionWidget(this->toolId());
        m_widgetHelper.setConfigGroupForExactTool(this->toolId());

        this->connect(this, SIGNAL(isActiveChanged(bool)), &m_widgetHelper, SLOT(slotToolActivatedChanged(bool)));
        this->connect(&m_widgetHelper,
                      SIGNAL(selectionActionChanged(SelectionAction)),
                      this,
                      SLOT(resetCursorStyle()));

        updateActionShortcutToolTips();
        if (m_widgetHelper.optionWidget()) {
            m_widgetHelper.optionWidget()->setContentsMargins(0, 10, 0, 10);
            if (isPixelOnly()) {
                m_widgetHelper.optionWidget()->setModeSectionVisible(false);
                m_widgetHelper.optionWidget()->setAdjustmentsSectionVisible(
                    true);
            }
            m_widgetHelper.optionWidget()->setReferenceSectionVisible(
                usesColorLabels());
        }

        return m_widgetHelper.optionWidget();
    }

    SelectionMode selectionMode() const
    {
        return m_widgetHelper.selectionMode();
    }

    SelectionAction selectionAction() const
    {
        if (alternateSelectionAction() == SELECTION_DEFAULT) {
            return m_widgetHelper.selectionAction();
        }
        return alternateSelectionAction();
    }

    bool antiAliasSelection() const
    {
        return m_widgetHelper.antiAliasSelection();
    }

    int growSelection() const
    {
        return m_widgetHelper.growSelection();
    }

    bool stopGrowingAtDarkestPixel() const
    {
        return m_widgetHelper.stopGrowingAtDarkestPixel();
    }

    int featherSelection() const
    {
        return m_widgetHelper.featherSelection();
    }

    QList<int> colorLabelsSelected() const
    {
        return m_widgetHelper.selectedColorLabels();
    }

    SampleLayersMode sampleLayersMode() const
    {
        KisSelectionOptions::ReferenceLayers referenceLayers =
            m_widgetHelper.referenceLayers();
        if (referenceLayers == KisSelectionOptions::AllLayers) {
            return SampleAllLayers;
        } else if (referenceLayers == KisSelectionOptions::CurrentLayer) {
            return SampleCurrentLayer;
        } else if (referenceLayers == KisSelectionOptions::ColorLabeledLayers) {
            return SampleColorLabeledLayers;
        }
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(true, SampleAllLayers);
        return SampleAllLayers;
    }

    SelectionAction alternateSelectionAction() const
    {
        return m_selectionActionAlternate;
    }

    KisSelectionOptions* selectionOptionWidget()
    {
        return m_widgetHelper.optionWidget();
    }

    virtual void setAlternateSelectionAction(SelectionAction action)
    {
        m_selectionActionAlternate = action;
    }

    void activateAlternateAction(KisTool::AlternateAction action) override
    {
        Q_UNUSED(action);
        BaseClass::activatePrimaryAction();
    }

    void deactivateAlternateAction(KisTool::AlternateAction action) override
    {
        Q_UNUSED(action);
        BaseClass::deactivatePrimaryAction();
    }

    void beginAlternateAction(KoPointerEvent *event,
                              KisTool::AlternateAction action) override
    {
        Q_UNUSED(action);
        beginPrimaryAction(event);
    }

    void continueAlternateAction(KoPointerEvent *event,
                                 KisTool::AlternateAction action) override
    {
        Q_UNUSED(action);
        continuePrimaryAction(event);
    }

    void endAlternateAction(KoPointerEvent *event,
                            KisTool::AlternateAction action) override
    {
        Q_UNUSED(action);
        endPrimaryAction(event);
    }

    KisNodeSP locateSelectionMaskUnderCursor(const QPointF &pos, Qt::KeyboardModifiers modifiers) {
        if (modifiers != Qt::NoModifier) return 0;

        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas, 0);

        KisSelectionSP selection = canvas->viewManager()->selection();
        if (selection &&
            selection->outlineCacheValid()) {

            const qreal handleRadius = qreal(this->handleRadius()) / canvas->coordinatesConverter()->effectiveZoom();
            QPainterPath samplePath;
            samplePath.addEllipse(pos, handleRadius, handleRadius);

            const QPainterPath selectionPath = selection->outlineCache();

            if (selectionPath.intersects(samplePath) && !selectionPath.contains(samplePath)) {
                KisNodeSP parent = selection->parentNode();
                if (parent && parent->isEditable()) {
                    return parent;
                }
            }
        }

        return 0;
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        const Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(event);
        // Assume all the modifiers were unpressed...
        m_currentModifiers = Qt::NoModifier;
        // ...and add those which are right now
        if (key == Qt::Key_Control || event->modifiers().testFlag(Qt::ControlModifier)) {
            m_currentModifiers.setFlag(Qt::ControlModifier);
        }
        if (key == Qt::Key_Shift || event->modifiers().testFlag(Qt::ShiftModifier)) {
            m_currentModifiers.setFlag(Qt::ShiftModifier);
        }
        if (key == Qt::Key_Alt || event->modifiers().testFlag(Qt::AltModifier)) {
            m_currentModifiers.setFlag(Qt::AltModifier);
        }
        
        // Avoid changing the selection mode and cursor if the user is interacting
        if (isSelecting()) {
            BaseClass::keyPressEvent(event);
            return;
        }
        if (isMovingSelection()) {
            return;
        }

        setAlternateSelectionAction(KisSelectionModifierMapper::map(m_currentModifiers));
        this->resetCursorStyle();
    }

    void keyReleaseEvent(QKeyEvent *event) override
    {
        const Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(event);
        // Assume all the modifiers were pressed...
        m_currentModifiers = Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier;
        // ...and remove those which aren't right now
        if (key == Qt::Key_Control || !event->modifiers().testFlag(Qt::ControlModifier)) {
            m_currentModifiers.setFlag(Qt::ControlModifier, false);
        }
        if (key == Qt::Key_Shift || !event->modifiers().testFlag(Qt::ShiftModifier)) {
            m_currentModifiers.setFlag(Qt::ShiftModifier, false);
        }
        if (key == Qt::Key_Alt || !event->modifiers().testFlag(Qt::AltModifier)) {
            m_currentModifiers.setFlag(Qt::AltModifier, false);
        }

        // Avoid changing the selection mode and cursor if the user is interacting
        if (isSelecting()) {
            BaseClass::keyReleaseEvent(event);
            return;
        }
        if (isMovingSelection()) {
            return;
        }

        setAlternateSelectionAction(KisSelectionModifierMapper::map(m_currentModifiers));
        if (m_currentModifiers == Qt::NoModifier) {
            KisNodeSP selectionMask = locateSelectionMaskUnderCursor(m_currentPos, m_currentModifiers);
            if (selectionMask) {
                this->useCursor(KisCursor::moveSelectionCursor());
            } else {
                this->resetCursorStyle();
            }
        } else {
            this->resetCursorStyle();
        }
    }

    void mouseMoveEvent(KoPointerEvent *event) override
    {
        m_currentPos = this->convertToPixelCoord(event->point);

        if (isSelecting()) {
            BaseClass::mouseMoveEvent(event);
            return;
        }
        if (isMovingSelection()) {
            return;
        }

        KisNodeSP selectionMask = locateSelectionMaskUnderCursor(m_currentPos, event->modifiers());
        if (selectionMask) {
            this->useCursor(KisCursor::moveSelectionCursor());
        } else {
            setAlternateSelectionAction(KisSelectionModifierMapper::map(m_currentModifiers));
            this->resetCursorStyle();
        }
    }

    void beginPrimaryAction(KoPointerEvent *event) override
    {
        if (isSelecting()) {
            BaseClass::beginPrimaryAction(event);
            return;
        }
        if (isMovingSelection()) {
            return;
        }

        const QPointF pos = this->convertToPixelCoord(event->point);
        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);

        KisNodeSP selectionMask = locateSelectionMaskUnderCursor(pos, event->modifiers());
        if (selectionMask) {
            if (this->beginMoveSelectionInteraction()) {
                KisStrokeStrategy *strategy = new MoveStrokeStrategy({selectionMask}, this->image().data(), this->image().data());
                m_moveStrokeId = this->image()->startStroke(strategy);
                m_dragStartPos = pos;
                m_didMove = true;
                return;
            }
        }

        m_didMove = false;
        BaseClass::beginPrimaryAction(event);
    }

    void continuePrimaryAction(KoPointerEvent *event) override
    {
        if (isMovingSelection()) {
            const QPointF pos = this->convertToPixelCoord(event->point);
            const QPoint offset((pos - m_dragStartPos).toPoint());

            this->image()->addJob(m_moveStrokeId, new MoveStrokeStrategy::Data(offset));
            return;
        }

        BaseClass::continuePrimaryAction(event);
    }

    void endPrimaryAction(KoPointerEvent *event) override
    {
        if (isMovingSelection()) {
            this->image()->endStroke(m_moveStrokeId);
            m_moveStrokeId.clear();
            this->endMoveSelectionInteraction();
            return;
        }

        BaseClass::endPrimaryAction(event);
    }

    bool selectionDidMove() const
    {
        return m_didMove;
    }

    QMenu *popupActionsMenu() override
    {
        if (isSelecting()) {
            return BaseClass::popupActionsMenu();
        }

        KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(kisCanvas, 0);

        return KisSelectionToolHelper::getSelectionContextMenu(kisCanvas);
    }

    KisPopupWidgetInterface* popupWidget() override
    {
        if (isSelecting()) {
            return BaseClass::popupWidget();
        }
        return nullptr;
    }

    bool beginMoveSelectionInteraction() {
        if (m_currentInteraction != Interaction_None) {
            return false;
        }
        m_currentInteraction = Interaction_MoveSelection;
        return true;
    }

    bool endMoveSelectionInteraction() {
        if (!isMovingSelection()) {
            return false;
        }
        m_currentInteraction = Interaction_None;
        updateCursorDelayed();
        return true;
    }

    bool beginSelectInteraction() {
        if (m_currentInteraction != Interaction_None) {
            return false;
        }
        m_currentInteraction = Interaction_Select;
        return true;
    }

    bool endSelectInteraction() {
        if (!isSelecting()) {
            return false;
        }
        m_currentInteraction = Interaction_None;
        updateCursorDelayed();
        return true;
    }

    bool isMovingSelection() const {
        return m_currentInteraction == Interaction_MoveSelection;
    }

    bool isSelecting() const {
        return m_currentInteraction == Interaction_Select;
    }

    void updateCursorDelayed() {
        setAlternateSelectionAction(KisSelectionModifierMapper::map(m_currentModifiers));
        QTimer::singleShot(100,
            this,
            [this]()
            {
                KisNodeSP selectionMask = locateSelectionMaskUnderCursor(m_currentPos, m_currentModifiers);
                if (selectionMask) {
                    this->useCursor(KisCursor::moveSelectionCursor());
                } else {
                    this->resetCursorStyle();
                }
            }
        );
    }

protected:
    using BaseClass::canvas;
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionActionAlternate;

    virtual bool isPixelOnly() const {
        return false;
    }

    virtual bool usesColorLabels() const {
        return false;
    }

private:
    enum Interaction
    {
        Interaction_None,
        Interaction_Select,
        Interaction_MoveSelection
    };

    Interaction m_currentInteraction{Interaction_None};

    Qt::KeyboardModifiers m_currentModifiers;

    QPointF m_dragStartPos;
    QPointF m_currentPos;
    KisStrokeId m_moveStrokeId;
    bool m_didMove = false;

    KisSignalAutoConnectionsStore m_modeConnections;
};

struct FakeBaseTool : KisTool
{
    FakeBaseTool(KoCanvasBase* canvas)
        : KisTool(canvas, QCursor())
    {
    }

    FakeBaseTool(KoCanvasBase* canvas, const QString &toolName)
        : KisTool(canvas, QCursor())
    {
        Q_UNUSED(toolName);
    }

    FakeBaseTool(KoCanvasBase* canvas, const QCursor &cursor)
        : KisTool(canvas, cursor)
    {
    }
};


typedef KisToolSelectBase<FakeBaseTool> KisToolSelect;


#endif // KISTOOLSELECTBASE_H
