/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOUCHPADSCROLLSHORTCUT_H
#define KISTOUCHPADSCROLLSHORTCUT_H

#include "kis_abstract_shortcut.h"

class QWheelEvent;
class KisTouchpadScrollShortcut : public KisAbstractShortcut
{
public:
    KisTouchpadScrollShortcut(KisAbstractInputAction* action, int index);
    ~KisTouchpadScrollShortcut() override;

    int priority() const override;
    bool match(QWheelEvent* event);
};

#endif // KISTOUCHPADSCROLLSHORTCUT_H
