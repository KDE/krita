/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef SPELLCHECKCONFIGDIALOG_H
#define SPELLCHECKCONFIGDIALOG_H

#include "ui_SpellCheckConfig.h"

#include <KDialog>

class SpellCheck;

class SpellCheckConfig : public QWidget
{
    Q_OBJECT
public:
    SpellCheckConfig(SpellCheck *spellcheck, QWidget *parent);
    virtual ~SpellCheckConfig();

public slots:
    void applyConfig();

private slots:

private:
    Ui::SpellCheckConfig widget;
    SpellCheck *m_spellcheck;
};

class SpellCheckConfigDialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a SpellCheck configuration dialog
     * @param autocorrect the spellcheck plugin
     * @param parent the parent widget
     */
    explicit SpellCheckConfigDialog(SpellCheck *spellcheck, QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~SpellCheckConfigDialog();

private:
    SpellCheckConfig *ui;
};

#endif

