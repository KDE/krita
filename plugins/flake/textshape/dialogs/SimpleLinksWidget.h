/* This file is part of the KDE project
 * Copyright (C) 2001 David Faure <faure@kde.org>
 * Copyright (C) 2005-2007, 2009, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010-2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2013 Aman Madaan <madaan.amanmadaan@gmail.com>
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
#ifndef SIMPLELINKSWIDGET_H
#define SIMPLELINKSWIDGET_H

#include  <ui_SimpleLinksWidget.h>
#include "FormattingButton.h"
#include <QWidget>
#include <KoTextEditor.h>

class ReferencesTool;

class SimpleLinksWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleLinksWidget(ReferencesTool *tool, QWidget *parent = 0);
    virtual ~SimpleLinksWidget();

Q_SIGNALS:
    void doneWithFocus();

public Q_SLOTS:
    void preparePopUpMenu();

private Q_SLOTS:
    void manageBookmarks();

private:
    Ui::SimpleLinksWidget widget;
    ReferencesTool *m_referenceTool;
};

#endif
