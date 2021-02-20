/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISNEWWINDOWLAYOUTDIALOG_H
#define KISNEWWINDOWLAYOUTDIALOG_H

#include <QDialog>
#include "ui_wdgnewwindowlayout.h"

class KisNewWindowLayoutDialog : public QDialog, Ui::DlgNewWindowLayout
{
public:
    KisNewWindowLayoutDialog(QWidget *parent = 0);

    void setName(const QString &name);
    QString name() const;
    bool showImageInAllWindows() const;
    bool primaryWorkspaceFollowsFocus() const;
};

#endif
