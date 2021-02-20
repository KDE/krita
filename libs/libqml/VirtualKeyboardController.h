/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef VIRTUALKEYBOARDCONTROLLER_H
#define VIRTUALKEYBOARDCONTROLLER_H

#include <QObject>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT VirtualKeyboardController : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void requestShowKeyboard();
    Q_INVOKABLE void requestHideKeyboard();

    static VirtualKeyboardController* instance();

Q_SIGNALS:
    void showKeyboard();
    void hideKeyboard();

private:
    explicit VirtualKeyboardController(QObject* parent = 0);
    virtual ~VirtualKeyboardController();

    static VirtualKeyboardController* sm_instance;
};

#endif // VIRTUALKEYBOARDCONTROLLER_H
