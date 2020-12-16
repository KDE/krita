/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Liebl
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#import <AppKit/AppKit.h>

extern "C" {
    bool isMouseCoalescingEnabled();
    void setMouseCoalescingEnabled(bool enabled);
}

bool isMouseCoalescingEnabled() {
    return NSEvent.isMouseCoalescingEnabled;
}

void setMouseCoalescingEnabled(bool enabled) {
    NSEvent.mouseCoalescingEnabled = enabled;
}
