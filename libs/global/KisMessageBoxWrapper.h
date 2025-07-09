/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISMESSAGEBOXWRAPPER_H
#define KISMESSAGEBOXWRAPPER_H

class QMessageBox;
class QString;

#include "kritaglobal_export.h"

/**
 * Wrap a QMessageBox
 */
namespace KisMessageBoxWrapper {


/**
 * @brief doNotAskAgainMessageBoxWrapper takes a messagebox and an identifier and
 * adds a Do Not Ask Again checkbox to the messagebox; then runs the checkbox and
 * returns the return balue
 * @param messageBox the wrapped messagebox
 * @param identifier a unique identifier that is used to store the the check
 * in the config file
 * @return the result of running the messagebox. If the messagebox is not shown,
 * the return value is QMessageBox::Yes
 */
KRITAGLOBAL_EXPORT int doNotAskAgainMessageBoxWrapper(QMessageBox *messageBox, const QString &identifier);


}

#endif // KISMESSAGEBOXWRAPPER_H
