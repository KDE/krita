/*
 * SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DLGCONFIGURECUMULATIVEUNDO_H
#define DLGCONFIGURECUMULATIVEUNDO_H

#include <KoDialog.h>
#include <QObject>
#include <QWidget>

struct KisCumulativeUndoData;

class KisDlgConfigureCumulativeUndo : public KoDialog
{
    Q_OBJECT

public:
    KisDlgConfigureCumulativeUndo(const KisCumulativeUndoData &data, int undoLimit, QWidget *parent = 0);
    ~KisDlgConfigureCumulativeUndo();

    KisCumulativeUndoData cumulativeUndoData() const;

private Q_SLOTS:
    void slotDefaultClicked();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // DLGCONFIGURECUMULATIVEUNDO_H
