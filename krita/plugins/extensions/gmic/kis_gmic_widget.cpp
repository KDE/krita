/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include <kis_gmic_widget.h>
#include <QGridLayout>
#include <QPushButton>
#include <qdialogbuttonbox.h>
#include <QDebug>

#include <QMetaType>

#include <kis_gmic_filter_settings.h>
#include "kis_gmic_settings_widget.h"
#include <kis_gmic_input_output_widget.h>

KisGmicWidget::KisGmicWidget(KisGmicFilterModel * filters): QWidget(),m_filterModel(filters)
{
    createMainLayout();


}

KisGmicWidget::~KisGmicWidget()
{

}

void KisGmicWidget::createMainLayout()
{
    m_filterConfigLayout = new QGridLayout;

    int column = 0;
    int row = 0;
    m_inputOutputOptions = new KisGmicInputOutputWidget();
    m_filterConfigLayout->addWidget(m_inputOutputOptions, row, column);
    column++;

    m_filterTree = new QTreeView();
    m_filterTree->setModel(m_filterModel);

    QItemSelectionModel *selectionModel= m_filterTree->selectionModel();
    connect(selectionModel, SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));

    m_filterConfigLayout->addWidget(m_filterTree, row, column);
    column++;

    m_filterOptions = new QWidget();
    m_filterConfigLayout->addWidget(m_filterOptions,row, column);
    m_filterOptionsRow = row;
    m_filterOptionsColumn = column;

    QDialogButtonBox * controlButtonBox = new QDialogButtonBox;
    QPushButton * maximize = new QPushButton("Maximize");
    controlButtonBox->addButton(maximize, QDialogButtonBox::AcceptRole);
    controlButtonBox->addButton(QDialogButtonBox::Ok);

    controlButtonBox->addButton(QDialogButtonBox::Apply);
    QAbstractButton *applyButton = controlButtonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(applyFilterSlot()));


    controlButtonBox->addButton(QDialogButtonBox::Cancel);
    controlButtonBox->addButton(QDialogButtonBox::Reset);
    m_filterConfigLayout->addWidget(controlButtonBox,row + 1, column, 1, 2);
    column++;

    setLayout(m_filterConfigLayout);
}


void KisGmicWidget::selectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/)
 {
     //get the text of the selected item
    const QModelIndex index = m_filterTree->selectionModel()->currentIndex();
    QString selectedText = index.data(Qt::DisplayRole).toString();


    QVariant var = index.data(WidgetRole);

    if (!var.isValid())
    {
        qDebug() << "Invalid QVariant, invalid command? : ';' ";
    }

    Command * gmicCommand = var.value<Command *>();

    if (gmicCommand)
    {
        m_filterConfigLayout->removeWidget(m_filterOptions);
        delete m_filterOptions;

        m_filterOptions = new KisGmicSettingsWidget(gmicCommand);
        m_filterConfigLayout->addWidget(m_filterOptions,m_filterOptionsRow,m_filterOptionsColumn);
        m_filterConfigLayout->update();
    } else {
        qDebug() << "Command is null";
    }

     //find out the hierarchy level of the selected item
     int hierarchyLevel=1;
     QModelIndex seekRoot = index;
     while(seekRoot.parent() != QModelIndex())
     {
         seekRoot = seekRoot.parent();
         hierarchyLevel++;
     }
     QString showString = QString("%1, Level %2").arg(selectedText)
                          .arg(hierarchyLevel);
     setWindowTitle(showString);
 }

void KisGmicWidget::applyFilterSlot()
{
    const QModelIndex index = m_filterTree->selectionModel()->currentIndex();
    QVariant settings = index.data(FilterSettingsRole);
    if (settings.isValid())
    {
        KisGmicFilterSetting * filterSettings = settings.value<KisGmicFilterSetting * >();
        filterSettings->setInputLayerMode(m_inputOutputOptions->inputMode());
        filterSettings->setOutputMode(m_inputOutputOptions->outputMode());


        qDebug() << "Valid settings!";
        qDebug() << "GMIC command : " << filterSettings->gmicCommand();

        emit sigApplyCommand(filterSettings);
    }
    else
    {
        qDebug() << "InValid settings!";
    }

}

void KisGmicWidget::cancelFilterSlot()
{

}

void KisGmicWidget::maximizeSlot()
{

}

void KisGmicWidget::okFilterSlot()
{

}

void KisGmicWidget::resetFilterSlot()
{

}
