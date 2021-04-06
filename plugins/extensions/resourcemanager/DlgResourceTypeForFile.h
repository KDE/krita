/*
 *  SPDX-FileCopyrightText: 2021 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef DLG_RESOURCE_TYPE_FOR_FILE_H
#define DLG_RESOURCE_TYPE_FOR_FILE_H

#include <KoDialog.h>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>
#include <QWidget>
#include <QMap>
#include <QString>


#include "ui_wdgdlgbundlemanager.h"

class QButtonGroup;


class DlgResourceTypeForFile : public KoDialog
{

    Q_OBJECT
public:

    DlgResourceTypeForFile(QWidget* parent, QMap<QString, QStringList> resourceTypesForMimetype);

    explicit DlgResourceTypeForFile(QWidget *parent = 0);

    QString getResourceTypeForMimetype(QString mimetype);


private:
    QMap<QString, QButtonGroup*> m_buttonGroupForMimetype;
    const QString m_propertyName {"resourceType"};
};

#endif // DLG_RESOURCE_TYPE_FOR_FILE_H
