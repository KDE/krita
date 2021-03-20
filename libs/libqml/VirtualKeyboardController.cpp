/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "VirtualKeyboardController.h"

#include <QCoreApplication>

VirtualKeyboardController* VirtualKeyboardController::sm_instance = 0;

void VirtualKeyboardController::requestShowKeyboard()
{
    emit showKeyboard();
}

void VirtualKeyboardController::requestHideKeyboard()
{
    emit hideKeyboard();
}

VirtualKeyboardController* VirtualKeyboardController::instance()
{
    if (!sm_instance)
        sm_instance = new VirtualKeyboardController(QCoreApplication::instance());

    return sm_instance;
}

VirtualKeyboardController::VirtualKeyboardController(QObject* parent)
 : QObject(parent)
{

}

VirtualKeyboardController::~VirtualKeyboardController()
{

}
