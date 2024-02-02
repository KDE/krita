/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DLG_LIST_PICKER_H
#define KIS_DLG_LIST_PICKER_H

#include <QScopedPointer>
#include <QDialog>

#include "kis_types.h"


namespace Ui {
class KisDlgListPicker;
}

class KisDlgListPicker : public QDialog
{
    Q_OBJECT

public:
    explicit KisDlgListPicker(QString windowTitle, QString availableString, QString currentString,
                              QList<QString> available, QList<QVariant> availableData,
                              QList<QString> chosen, QList<QVariant> currentData,
                              QWidget *parent = 0);
    ~KisDlgListPicker() override;

public Q_SLOTS:
    QList<QVariant> getChosenData();

private Q_SLOTS:
    void slotMoveRight();
    void slotMoveLeft();
    void slotMoveUp();
    void slotMoveDown();

private:
    Ui::KisDlgListPicker *ui;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_DLG_LIST_PICKER_H
