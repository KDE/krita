/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2018 Mehmet Salih Çalışkan <msalihcaliskan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */


#ifndef SVGRICHTEXTCTRL_H
#define SVGRICHTEXTCTRL_H

#include <QTextEdit>

class SvgRichTextCtrl : public QTextEdit
{
public:
    SvgRichTextCtrl(QWidget* parent = nullptr);
protected:
    void insertFromMimeData(const QMimeData* source) override;
};

#endif // SVGRICHTEXTCTRL_H
