#ifndef KIS_SELECTION_ACTION_TEMPLATE_H
#define KIS_SELECTION_ACTION_TEMPLATE_H

#include "KoPointerEvent.h"
#include "kis_tool.h"
#include "kis_canvas2.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_config_widget_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include <bitset>

#define QMOD_BINARY() QString(std::bitset<sizeof(int) * 8>(event->modifiers()).to_string().c_str())


static SelectionAction selectionModifierMap(Qt::KeyboardModifiers m) {
    SelectionAction newAction = SELECTION_DEFAULT;
    if (m & Qt::ControlModifier) {
        newAction = SELECTION_REPLACE;
    } else if ((m & Qt::ShiftModifier) && (m & Qt::AltModifier)) {
        newAction = SELECTION_INTERSECT;
    } else if (m & Qt::ShiftModifier) {
        newAction = SELECTION_ADD;
    } else if (m & Qt::AltModifier) {
        newAction = SELECTION_SUBTRACT;
    }
    return newAction;
}

template <class BaseClass>
class SelectionActionHandler : public BaseClass
{

public:

    SelectionActionHandler(KoCanvasBase* canvas, QString toolName)
        : BaseClass(canvas)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
        , m_widgetHelper(toolName)
    {
    }


    QWidget* createOptionWidget()
    {
        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        Q_ASSERT(canvas);

        m_widgetHelper.createOptionWidget(canvas, this->toolId());
        m_widgetHelper.optionWidget()->disableAntiAliasSelectionOption();
        return m_widgetHelper.optionWidget();
    }

    void keyPressEvent(QKeyEvent *event)
    {
        if (!m_widgetHelper.processKeyPressEvent(event)) {
            BaseClass::keyPressEvent(event);
        }
    }

    SelectionActionHandler(KoCanvasBase * Canvas)
        : BaseClass(Canvas)
    {
    }

    SelectionMode selectionMode() const
    {
        return m_widgetHelper.selectionMode();
    }

    bool antiAliasSelection() const
    {
        return m_widgetHelper.optionWidget()->antiAliasSelection();
    }

    SelectionAction selectionAction() const
    {
        if (alternateSelectionAction() == SELECTION_DEFAULT) {
            return m_widgetHelper.selectionAction();
        }
        return alternateSelectionAction();
    }


public:
    void setAlternateSelectionAction(SelectionAction action)
    {
        m_selectionActionAlternate = action;
    }

    SelectionAction alternateSelectionAction() const
    {
        return m_selectionActionAlternate;
    }

    void activateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::activatePrimaryAction();
    }

    void deactivateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::deactivatePrimaryAction();
    }

    void beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        beginPrimaryAction(event);
    }

    void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        continuePrimaryAction(event);
    }

    void endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        dbgKrita << "endAlternateAction " << "; selectionAction=" << selectionAction() << "; modifiers=" << QMOD_BINARY();;
        endPrimaryAction(event);
    }

    void beginPrimaryAction(KoPointerEvent *event)
    {
        keysAtStart = event->modifiers();

        setAlternateSelectionAction(selectionModifierMap(keysAtStart));
        if (alternateSelectionAction() != SELECTION_DEFAULT) {
            dbgKrita << "beginPrimaryAction" << alternateSelectionAction()
                     << "; modifiers =" << QMOD_BINARY();
        } else {
            dbgKrita << "Starting selection action" << SELECTION_DEFAULT << "(default)";
        }

        //Start in greedy mode & tell base class not to listen to modifiers
        //Ctrl key (SELECTION_REPLACE) forwards all further keyboard input to the underlying tool
        if (alternateSelectionAction() != SELECTION_REPLACE) {
            BaseClass::listenToModifiers(false);
        }

        BaseClass::beginPrimaryAction(event);
    }

    void continuePrimaryAction(KoPointerEvent *event)
    {
        //Tell the base tool it can start capturing modifiers if it pleases
        if ((keysAtStart != event->modifiers()) && !BaseClass::listeningToModifiers()) {
            BaseClass::listenToModifiers(true);
        }

        //Either we are performing the action in greedy mode or the base tool won't take modifiers
        if ((keysAtStart == Qt::NoModifier) || !BaseClass::listeningToModifiers()) {
            setAlternateSelectionAction(selectionModifierMap(event->modifiers()));
        }

        BaseClass::continuePrimaryAction(event);
    }

    void endPrimaryAction(KoPointerEvent *event)
    {
        dbgKrita << "Ending selection action " << alternateSelectionAction();
        keysAtStart = Qt::NoModifier; //reset this with each action
        BaseClass::endPrimaryAction(event);
    }

    using BaseClass::canvas;

protected:
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionAction;

private:
    Qt::KeyboardModifiers keysAtStart;
    SelectionAction m_selectionActionAlternate;
};

#endif // KIS_SELECTION_ACTION_TEMPLATE_H
