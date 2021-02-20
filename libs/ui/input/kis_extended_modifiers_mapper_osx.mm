/*
 *  SPDX-FileCopyrightText: 2020 Ivan SantaMaría <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_extended_modifiers_mapper_osx.h"

#import <QDebug>
#import <QFlag>
#import <qmutex.h>
#import <AppKit/AppKit.h>

#include "krita_container_utils.h"

class ExtendedModifiers
{
public:
    enum Modifier {
        None = 0x0,
        Space = 0x1,
        Shift = 0x2,
        Control = 0x4,
        Option = 0x8,
        Command = 0x10,
        Key_R = 0x20,
        Key_V = 0x40
    };
    Q_DECLARE_FLAGS(Modifiers, Modifier)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ExtendedModifiers::Modifiers)

static id localMonitor = 0;
static id globalMonitor = 0;
static bool fromGlobalEvent = false;

static ExtendedModifiers::Modifiers extendedMods;
static QMutex mutex;

void processEvent(NSEvent *event)
{
    unsigned int eventType = [event type];

    if (eventType == NSEventTypeKeyDown || eventType == NSEventTypeKeyUp) {
        unsigned short keyCode = [event keyCode];

        // virtual kbd codes
        if(keyCode == 49) {
            extendedMods.setFlag(ExtendedModifiers::Space, (eventType == NSEventTypeKeyDown));
        } else if (keyCode == 15) {
            extendedMods.setFlag(ExtendedModifiers::Key_R, (eventType == NSEventTypeKeyDown));
        } else if (keyCode == 9) {
            extendedMods.setFlag(ExtendedModifiers::Key_V, (eventType == NSEventTypeKeyDown));
        }
    }

    if (eventType == NSEventTypeFlagsChanged) {
        NSEventModifierFlags modifiers = [event modifierFlags];
        extendedMods.setFlag(ExtendedModifiers::Shift, modifiers & NSEventModifierFlagShift);
        extendedMods.setFlag(ExtendedModifiers::Command, modifiers & NSEventModifierFlagCommand);
        extendedMods.setFlag(ExtendedModifiers::Control, modifiers & NSEventModifierFlagControl);
        extendedMods.setFlag(ExtendedModifiers::Option, modifiers & NSEventModifierFlagOption);
    }
}

void activateGlobalMonitor(bool activate)
{
    if (activate) {
        NSEventMask eventMask = (NSEventMaskFlagsChanged | NSEventMaskKeyDown | NSEventMaskKeyUp);
        // global will capture events when focus is on another windows
        globalMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:eventMask handler:^(NSEvent *event) {
            fromGlobalEvent = true;
            processEvent(event);
            return;
        }];
    } else {
        [NSEvent removeMonitor:globalMonitor];
    }
}

void activateLocalMonitor(bool activate, KisShortcutMatcher &matcher)
{
    if (activate) {
        if (localMonitor) {
            return;
        }
        NSEventMask eventMask = (NSEventMaskFlagsChanged | NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskLeftMouseUp | NSEventMaskLeftMouseDown);
        localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^(NSEvent *event)
        {
            bool recoveryFromGlobal = false;

            mutex.lock();
            processEvent(event);

            if (fromGlobalEvent && extendedMods == ExtendedModifiers::None) {
                recoveryFromGlobal = true;
            }

            mutex.unlock();

            // HACK workaround: we call recoveryModifiers once after
            // all extended modifiers are released to avoid locking
            // modifiers when coming from other applications focus.
            if (fromGlobalEvent && recoveryFromGlobal) {
                matcher.recoveryModifiersWithoutFocus(queryPressedKeysMac());
                fromGlobalEvent = false;
            }
            return event;
        }];

    } else {
        extendedMods = ExtendedModifiers::None;
        if (localMonitor) {
            [NSEvent removeMonitor:localMonitor];
            localMonitor = 0;
        }
    }
}

QVector<Qt::Key> queryPressedKeysMac()
{
    QVector<Qt::Key> result;

    mutex.lock();
    if (extendedMods.testFlag(ExtendedModifiers::Space)) {
        result << Qt::Key_Space;
    }
    if (extendedMods.testFlag(ExtendedModifiers::Key_R)) {
        result << Qt::Key_R;
    }
    if (extendedMods.testFlag(ExtendedModifiers::Key_V)) {
        result << Qt::Key_V;
    }

    Qt::KeyboardModifiers modifiers = QGuiApplication::queryKeyboardModifiers();
    if (modifiers.testFlag(Qt::ShiftModifier) || extendedMods.testFlag(ExtendedModifiers::Shift)) {
        result << Qt::Key_Shift;
    }
    if (modifiers.testFlag(Qt::ControlModifier) || extendedMods.testFlag(ExtendedModifiers::Command)) {
        result << Qt::Key_Control;
    }
    if (modifiers.testFlag(Qt::MetaModifier) || extendedMods.testFlag(ExtendedModifiers::Control)) {
        result << Qt::Key_Meta;
    }
    if (modifiers.testFlag(Qt::AltModifier) || extendedMods.testFlag(ExtendedModifiers::Option)) {
        result << Qt::Key_Alt;
    }
    mutex.unlock();

    KritaUtils::makeContainerUnique(result);

    return result;
}
