/* This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __CHANGE_CONFIGURE_DIALOG_H__
#define __CHANGE_CONFIGURE_DIALOG_H__

#include <QLabel>
#include <KoChangeTracker.h>

class ColorDisplayLabel: public QLabel
{
public:
    explicit ColorDisplayLabel(QWidget *parent = 0);
    ~ColorDisplayLabel();
    void paintEvent(QPaintEvent *event);
    const QColor &color() const;
    void setColor(const QColor &color);

private:
    QColor labelColor;
};

#include <ui_ChangeConfigureDialog.h>

class ChangeConfigureDialog: public QDialog
{
    Q_OBJECT

    typedef enum {
        eInsert,
        eDelete,
        eFormatChange,
        eChangeTypeNone
    } ChangeType;

public:
    ChangeConfigureDialog(const QColor &insertionColor, const QColor &deletionColor, const QColor &formatChangeColor, const QString &authorName, KoChangeTracker::ChangeSaveFormat changeSaveFormat, QWidget *parent = NULL);
    ~ChangeConfigureDialog();

    const QColor &getInsertionBgColor();
    const QColor &getDeletionBgColor();
    const QColor &getFormatChangeBgColor();
    const QString authorName();
    KoChangeTracker::ChangeSaveFormat saveFormat();

private:
    Ui::ChangeConfigureDialog ui;
    void updatePreviewText();
    void colorSelect(ChangeType type);

private Q_SLOTS:
    void insertionColorSelect();
    void deletionColorSelect();
    void formatChangeColorSelect();
};
#endif
