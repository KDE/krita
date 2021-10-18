/*
 *  SPDX-FileCopyrightText: 2020 Anna Medonosová <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SPenSettings.h"
#include <KisSPenSettings.h>

#include <kpluginfactory.h>
#include <kis_action.h>
#include <KisViewManager.h>
#include <kis_action_manager.h>
#include <KisPart.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kis_preference_set_registry.h>
#include <kis_canvas2.h>
#include <input/kis_input_manager.h>
#include <kis_canvas_controller.h>
#include <kis_popup_palette.h>


K_PLUGIN_FACTORY_WITH_JSON(SPenSettingsFactory, "kritaspensettings.json", registerPlugin<SPenSettings>();)


SPenSettings::SPenSettings(QObject* parent, const QVariantList&)
    : KisActionPlugin(parent)
{
    // hack: create action for popup palette
    m_actionShowPopupPalette.reset(viewManager()->actionManager()->createAction("spen_show_popup_palette"));
    connect(m_actionShowPopupPalette.data(), SIGNAL(triggered()), this, SLOT(slotTriggerPopupPalette()));

    KisPreferenceSetRegistry* preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    KisSPenSettingsFactory* settingsFactory = new KisSPenSettingsFactory();
    preferenceSetRegistry->add("KisSPenSettingsFactory", settingsFactory);

    //emit settingsChanged() if the settings are changed in krita preferences
    connect(&(settingsFactory->repeater), SIGNAL(settingsUpdated()), this, SLOT(slotLoadSettings()), Qt::UniqueConnection);

    slotLoadSettings();

    KisAction* actionSPenClick = viewManager()->actionManager()->createAction("spen_click");
    connect(actionSPenClick, &KisAction::triggered, this, [this](){ slotActivateAction(Action::Click); });

    KisAction* actionSPenDoubleClick = viewManager()->actionManager()->createAction("spen_double_click");
    connect(actionSPenDoubleClick, &KisAction::triggered, this, [this]() { slotActivateAction(Action::DoubleClick); });

    KisAction* actionSPenSwipeUp = viewManager()->actionManager()->createAction("spen_swipe_up");
    connect(actionSPenSwipeUp, &KisAction::triggered, this, [this]() { slotActivateAction(Action::SwipeUp); });

    KisAction* actionSPenSwipeDown = viewManager()->actionManager()->createAction("spen_swipe_down");
    connect(actionSPenSwipeDown, &KisAction::triggered, this, [this]() { slotActivateAction(Action::SwipeDown); });

    KisAction* actionSPenSwipeLeft = viewManager()->actionManager()->createAction("spen_swipe_left");
    connect(actionSPenSwipeLeft, &KisAction::triggered, this, [this]() { slotActivateAction(Action::SwipeLeft); });

    KisAction* actionSPenSwipeRight = viewManager()->actionManager()->createAction("spen_swipe_right");
    connect(actionSPenSwipeRight, &KisAction::triggered, this, [this]() { slotActivateAction(Action::SwipeRight); });

    KisAction* actionSPenCircleCW = viewManager()->actionManager()->createAction("spen_circle_cw");
    connect(actionSPenCircleCW, &KisAction::triggered, this, [this]() { slotActivateAction(Action::CircleCW); });

    KisAction* actionSPenCircleCCW = viewManager()->actionManager()->createAction("spen_circle_ccw");
    connect(actionSPenCircleCCW, &KisAction::triggered, this, [this]() { slotActivateAction(Action::CircleCCW); });
}

SPenSettings::~SPenSettings()
{
}

void SPenSettings::slotActivateAction(SPenSettings::Action gestureType)
{
    QString actionName = m_actionMap.value(gestureType);
//    qDebug() << "Gesture " << gestureType << " action " << actionName;

    if (!actionName.isEmpty()) {
        KActionCollection* actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
        QAction* action = actionCollection->action(actionName);
        if (action) {
//            qDebug() << "triggering action " << actionName;
            action->trigger();
        }
    }
}

void SPenSettings::slotLoadSettings()
{
    m_actionMap.clear();

    KConfigGroup cfg = KSharedConfig::openConfig()->group("SPenSettings");

    m_actionMap.insert(Action::Click, cfg.readEntry("actionButtonClick", QString()));
    m_actionMap.insert(Action::DoubleClick,cfg.readEntry("actionButtonDoubleClick", QString()));
    m_actionMap.insert(Action::SwipeUp,cfg.readEntry("actionGestureSwipeUp", QString()));
    m_actionMap.insert(Action::SwipeDown, cfg.readEntry("actionGestureSwipeDown", QString()));
    m_actionMap.insert(Action::SwipeLeft, cfg.readEntry("actionGestureSwipeLeft", QString()));
    m_actionMap.insert(Action::SwipeRight, cfg.readEntry("actionGestureSwipeRight", QString()));
    m_actionMap.insert(Action::CircleCW, cfg.readEntry("actionGestureCircleCW", QString()));
    m_actionMap.insert(Action::CircleCCW, cfg.readEntry("actionGestureCircleCCW", QString()));
}

void SPenSettings::slotTriggerPopupPalette()
{
    if (KisPart::instance()->currentInputManager()->canvas()) {
        // determine the current location of cursor on the screen, for popup palette placement
        QPoint cursorPosition = KisPart::instance()->currentInputManager()->canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
        KisPopupPalette *popupPalette = KisPart::instance()->currentInputManager()->canvas()->popupPalette();
        if (popupPalette) {
            if (popupPalette->isVisible()) {
                popupPalette->dismiss();
            } else {
                popupPalette->popup(cursorPosition);
            }
        }
    }
}

#include "SPenSettings.moc"

