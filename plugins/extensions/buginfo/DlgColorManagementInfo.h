/*
 * SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_COLORMANAGEMENTINFO_H
#define DLG_COLORMANAGEMENTINFO_H

#include <KoDialog.h>
#include "dlg_buginfo.h"


class QSettings;
class KisOutputColorInfoInterface;

class DlgColorManagementInfo: public DlgBugInfo
{
    Q_OBJECT
public:
    DlgColorManagementInfo(QWidget * parent = 0);
    ~DlgColorManagementInfo() override;

    QString defaultNewFileName() override;
    QString originalFileName() override;

public:
    QString replacementWarningText() override;
    QString captionText() override;
    QString infoText(QSettings& kritarc) override;

private:
    QScopedPointer<KisOutputColorInfoInterface> m_outputColorInfoInterface;
};

#endif // DLG_COLORMANAGEMENTINFO_H
