/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDlgListPicker.h"
#include "ui_KisDlgListPicker.h"
#include <kstandardguiitem.h>

struct KisDlgListPicker::Private
{
    
};

KisDlgListPickerItem::KisDlgListPickerItem(const QString &text)
    : QListWidgetItem(text)
{
}

bool KisDlgListPickerItem::operator<(const QListWidgetItem &other) const
{
    auto data = this->data(Qt::UserRole);
    auto other_data = other.data(Qt::UserRole);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (data.type() != QVariant::String || other_data.type() != QVariant::String)
#else
    if (data.metaType().id() != QMetaType::QString || other_data.metaType().id() != QMetaType::QString)
#endif
        return false;

    return data.toString() < other_data.toString();
}

KisDlgListPicker::KisDlgListPicker(QString windowTitle, QString availableString, QString currentString,
                                   QList<QString> available, QList<QVariant> availableData, QList<QIcon> availableIcons,
                                   QList<QString> chosen, QList<QVariant> chosenData, QList<QIcon> chosenIcons,
                                   QWidget *parent, bool useDataSorting
                                   ) :
    QDialog(parent),
    ui(new Ui::KisDlgListPicker),
    m_d(new Private)
{
    ui->setupUi(this);

    this->setWindowTitle(windowTitle);
    ui->label->setText(availableString);
    ui->label_2->setText(currentString);

    KGuiItem::assign(ui->btnBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(ui->btnBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());

    int i = 0;
    Q_FOREACH (QString name, available) {
        QListWidgetItem *item;

        if (useDataSorting) {
            item = new KisDlgListPickerItem(name);
        } else {
            item = new QListWidgetItem(name);
        }

        // Set icons only if icons are provided
        if (availableIcons.length() == available.length()) {
            item->setIcon(availableIcons[i]);
        }

        item->setData(Qt::UserRole, availableData[i]);
        ui->lstAvailable->addItem(item);
        i++;
    }

    i = 0;
    Q_FOREACH (QString name, chosen) {
        QListWidgetItem *item;

        if (useDataSorting) {
            item = new KisDlgListPickerItem(name);
        } else {
            item = new QListWidgetItem(name);
        }

        // Set icons only if icons are provided
        if (chosenIcons.length() == chosen.length()) {
            item->setIcon(chosenIcons[i]);
        }

        item->setData(Qt::UserRole, chosenData[i]);
        ui->lstCurrent->addItem(item);
        i++;
    }

    ui->lstAvailable->sortItems(); // sort alphabetically by name

    connect(ui->btnAdd, SIGNAL(clicked()), SLOT(slotMoveRight()));
    connect(ui->btnRemove, SIGNAL(clicked()), SLOT(slotMoveLeft()));

    connect(ui->btnDown, SIGNAL(clicked()), SLOT(slotMoveDown()));
    connect(ui->btnUp, SIGNAL(clicked()), SLOT(slotMoveUp()));
}

KisDlgListPicker::~KisDlgListPicker()
{
    delete ui;
}

void KisDlgListPicker::enable_vertical(bool enable)
{
    ui->btnUp->setEnabled(enable);
    ui->btnUp->setVisible(enable);
    ui->btnDown->setEnabled(enable);
    ui->btnDown->setVisible(enable);
}

void KisDlgListPicker::enable_sorting(bool enable)
{
    ui->lstAvailable->setSortingEnabled(enable);
    ui->lstCurrent->setSortingEnabled(enable);
}

QList<QVariant> KisDlgListPicker::getChosenData()
{
    QList<QVariant> chosenData;
    for (int i = 0; i < ui->lstCurrent->count(); i++) {
        chosenData.append(ui->lstCurrent->item(i)->data(Qt::UserRole));
    }
    return chosenData;
}

void KisDlgListPicker::slotMoveRight()
{
    QListWidgetItem *item = ui->lstAvailable->currentItem();
    if (!item) return;

    const int prevPosition = ui->lstAvailable->row(item) - 1;
    const int newPosition = ui->lstCurrent->currentRow() + 1;

    ui->lstAvailable->takeItem(ui->lstAvailable->row(item));
    ui->lstAvailable->setCurrentRow(qMax(0, prevPosition));
    ui->lstCurrent->insertItem(newPosition, item);
    ui->lstCurrent->setCurrentItem(item);
}

void KisDlgListPicker::slotMoveLeft()
{
    QListWidgetItem *item = ui->lstCurrent->currentItem();
    if (!item) return;

    const int prevPosition = ui->lstCurrent->row(item) - 1;
    const int newPosition = ui->lstAvailable->currentRow() + 1;

    ui->lstCurrent->takeItem(ui->lstCurrent->row(item));
    ui->lstCurrent->setCurrentRow(qMax(0, prevPosition));
    ui->lstAvailable->insertItem(newPosition, item);
    ui->lstAvailable->setCurrentItem(item);
}

void KisDlgListPicker::slotMoveUp()
{
    QListWidgetItem *item = ui->lstCurrent->currentItem();
    if (!item) return;

    int position = ui->lstCurrent->row(item);

    if (position <= 0) return;

    ui->lstCurrent->takeItem(ui->lstCurrent->row(item));
    ui->lstCurrent->insertItem(position - 1, item);
    ui->lstCurrent->setCurrentItem(item);
}

void KisDlgListPicker::slotMoveDown()
{
    QListWidgetItem *item = ui->lstCurrent->currentItem();
    if (!item) return;

    int position = ui->lstCurrent->row(item);

    if (position >= ui->lstCurrent->count() - 1) return;

    ui->lstCurrent->takeItem(ui->lstCurrent->row(item));
    ui->lstCurrent->insertItem(position + 1, item);
    ui->lstCurrent->setCurrentItem(item);
}

