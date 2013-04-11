/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#ifndef SELECTVIDEOWIDGET_H
#define SELECTVIDEOWIDGET_H

#include <kurl.h>

#include <QWidget>

class KFileWidget;
class QCheckBox;

class SelectVideoWidget : public QWidget
{
public:
    explicit SelectVideoWidget(QWidget *parent = 0);
    ~SelectVideoWidget();

    KUrl selectedUrl() const;
    bool saveEmbedded();

    void accept();
    void cancel();
private:

    KFileWidget *m_fileWidget;
    QCheckBox *m_saveEmbedded;
};

#endif //SELECTVIDEOWIDGET_H
