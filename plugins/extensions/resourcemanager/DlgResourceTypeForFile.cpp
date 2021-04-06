/*
 *  SPDX-FileCopyrightText: 2021 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "DlgResourceTypeForFile.h"


#include <QRadioButton>
#include <QButtonGroup>
#include <QDebug>

#include <kis_assert.h>

#include <KisResourceTypeModel.h>
#include <KisResourceTypes.h>

DlgResourceTypeForFile::DlgResourceTypeForFile(QWidget *parent, QMap<QString, QStringList> resourceTypesForMimetype)
    : KoDialog(parent)
{
    setCaption(i18n("Set the resource type for files"));

    QVBoxLayout* layout = new QVBoxLayout(parent);

    KisResourceTypeModel model;

    QStringList keys = resourceTypesForMimetype.keys();

    for (int i = 0; i < keys.size(); i++) {
        QLabel* label;
        label = new QLabel(parent);
        if (keys[i] == "image/png") {
            label->setText(i18nc("Question in a dialog so the user can choose which resource type the PNG files belong to: brush tips or patterns",
                                 "What resource do you want to import these PNG files as?"));
        } else if (keys[i] == "image/svg+xml") {
            label->setText(i18nc("Question in a dialog so the user can choose which resource type the SVG files belong to: brush tips or patterns or symbols",
                                 "What resource do you want to import these SVG files as?"));
        } else {
            label->setText(i18nc("Question in a dialog so the user can choose which resource type the files of type %1 belongs to; % will be rather untranslatable noun (a mimetype)",
                                 "What resource do you want to import these %1 files as?", keys[i]));
        }
        layout->addWidget(label);

        QButtonGroup* buttonGroup = new QButtonGroup(parent);
        m_buttonGroupForMimetype.insert(keys[i], buttonGroup);

        QStringList resourceTypes = resourceTypesForMimetype[keys[i]];
        for (int j = 0; j < resourceTypes.size(); j++)
        {
            QString resourceName = ResourceName::resourceTypeToName(resourceTypes[j]);
            QRadioButton* button = new QRadioButton(resourceName, parent);
            button->setProperty("resourceType", resourceName);
            buttonGroup->addButton(button);
            layout->addWidget(button);
        }
    }


    QWidget* widget = new QWidget(parent);
    widget->setLayout(layout);
    this->setMainWidget(widget);

    
}

QString DlgResourceTypeForFile::getResourceTypeForMimetype(QString mimetype)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_buttonGroupForMimetype.contains(mimetype), "");

    QButtonGroup* group = m_buttonGroupForMimetype[mimetype];

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(group, "");

    QList<QAbstractButton*> buttons = group->buttons();
    for (int i = 0; i < buttons.size(); i++) {
        if (buttons[i]->isChecked()) {
            QString resourceType = buttons[i]->property(m_propertyName.toStdString().c_str()).toString();
            return resourceType;
        }
    }
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, "");
    return "";
}


