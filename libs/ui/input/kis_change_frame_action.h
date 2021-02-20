/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CHANGE_FRAME_ACTION_H
#define __KIS_CHANGE_FRAME_ACTION_H

#include <QScopedPointer>

#include "kis_abstract_input_action.h"


class KisChangeFrameAction : public KisAbstractInputAction
{
public:
    enum Shortcuts {
        NextFrameShortcut,
        PreviousFrameShortcut
    };


    KisChangeFrameAction();
    ~KisChangeFrameAction() override;

    void begin(int shortcut, QEvent *event) override;
    bool isAvailable() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CHANGE_FRAME_ACTION_H */
