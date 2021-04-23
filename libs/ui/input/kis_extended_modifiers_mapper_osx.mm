/*
 *  Copyright (c) 2020 Ivan SantaMaría <ghevan@gmail.com>
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
    if (eventType == NSEventTypeMouseExited) {
        extendedMods = ExtendedModifiers::None;
    }
}

void activateLocalMonitor(bool activate)
{
    if (activate) {
        if (localMonitor) {
            return;
        }
        NSEventMask eventMask = (NSEventMaskFlagsChanged | NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskLeftMouseUp
                                 | NSEventMaskLeftMouseDown | NSEventMaskMouseEntered | NSEventMaskMouseExited );
        localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^(NSEvent *event)
        {
            mutex.lock();
            processEvent(event);
            mutex.unlock();

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
