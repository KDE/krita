/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTPROPERTYCONFIGDIALOG_H
#define TEXTPROPERTYCONFIGDIALOG_H

#include <QObject>
#include <KoDialog.h>
#include <KisQQuickWidget.h>

#include "TextPropertyConfigModel.h"

class TextPropertyConfigDialog : public KoDialog
{
    Q_OBJECT
public:
    TextPropertyConfigDialog(QWidget *parent = nullptr);
    ~TextPropertyConfigDialog();

    void setTextPropertyConfigModel(TextPropertyConfigModel *model);
    TextPropertyConfigModel *model();
private:
    KisQQuickWidget *m_quickWidget {0};
    TextPropertyConfigModel *m_model;
};

#endif // TEXTPROPERTYCONFIGDIALOG_H
