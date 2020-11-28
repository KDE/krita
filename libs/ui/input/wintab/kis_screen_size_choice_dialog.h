/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SCREEN_SIZE_CHOICE_DIALOG_H
#define KIS_SCREEN_SIZE_CHOICE_DIALOG_H

#include <QDialog>

class QButtonGroup;

namespace Ui {
class KisScreenSizeChoiceDialog;
}

class KisScreenSizeChoiceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KisScreenSizeChoiceDialog(QWidget *parent,
                                       const QRect &wintabRect,
                                       const QRect &qtRect);

    ~KisScreenSizeChoiceDialog();


    QRect screenRect() const;
    bool canUseDefaultSettings() const;

private:
    void loadSettings(const QRect &defaultRect);
    void saveSettings();

private Q_SLOTS:
    void slotFinished();

private:
    Ui::KisScreenSizeChoiceDialog *ui;

    QRect m_wintabRect;
    QRect m_qtRect;

    QButtonGroup *m_dataSource;
};

#endif // KIS_SCREEN_SIZE_CHOICE_DIALOG_H
