/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DLG_LIST_PICKER_H
#define KIS_DLG_LIST_PICKER_H

#include <QScopedPointer>
#include <QDialog>
#include <QListWidget>

namespace Ui {
class KisDlgListPicker;
}

//Custom list picker item for proper sorting of tools
class KisDlgListPickerItem : public QListWidgetItem
{
public:
    KisDlgListPickerItem(const QString &text);
    bool operator<(const QListWidgetItem &other) const override;
};

class KisDlgListPicker : public QDialog
{
    Q_OBJECT

public:
    explicit KisDlgListPicker(QString windowTitle, QString availableString, QString currentString,
                              QList<QString> available, QList<QVariant> availableData, QList<QIcon> availableIcons,
                              QList<QString> chosen, QList<QVariant> currentData, QList<QIcon> currentIcons,
                              QWidget *parent = 0, bool useaDataSorting = false);
    ~KisDlgListPicker() override;

    /// Enables/Disables vertical move buttons
    void enable_vertical(bool enable);
    /// Enables/Disables sorting of items
    void enable_sorting(bool enable);

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
