/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KisMessageBoxWrapper.h>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

namespace KisMessageBoxWrapper {

int doNotAskAgainMessageBoxWrapper(QMessageBox *messageBox, const QString &identifier)
{
    KConfigGroup cfg(KSharedConfig::openConfig(), "DoNotAskAgain");
    bool showMessage = cfg.readEntry(identifier, true);
    if (showMessage) {
        QCheckBox *cb = new QCheckBox(i18n("Don't ask this again"));
        messageBox->setCheckBox(cb);
        const int res = messageBox->exec();
        cfg.writeEntry(identifier, cb->checkState() == Qt::CheckState::Unchecked);
        return res;
    }
    else {
        return QMessageBox::Yes;
    }
}


}
