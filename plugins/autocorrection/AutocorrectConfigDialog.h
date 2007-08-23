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

#ifndef AUTOCORRECTCONFIGDIALOG_H
#define AUTOCORRECTCONFIGDIALOG_H

#include "ui_AutocorrectConfig.h"

#include <KDialog>

class Autocorrect;

class AutocorrectConfig : public QWidget
{
    Q_OBJECT
public:
    AutocorrectConfig(Autocorrect *autocorrect, QWidget *parent);
    virtual ~AutocorrectConfig();

public slots:
    void applyConfig();

private slots:
    /* tab 3 */
    void enableAdvAutocorrection(int state);
    void enableAutocorrectFormat(int state);
    void addAutocorrectEntry();
    void removeAutocorrectEntry();
    void setFindReplaceText(int row, int column);
    void enableAddRemoveButton();
    void changeCharFormat();

    /* tab 4 */
    void abbreviationChanged(const QString &text);
    void twoUpperLetterChanged(const QString &text);
    void addAbbreviationEntry();
    void removeAbbreviationEntry();
    void addTwoUpperLetterEntry();
    void removeTwoUpperLetterEntry();

private:
    Ui::AutocorrectConfig widget;
    Autocorrect *m_autocorrect;
    QSet<QString> m_upperCaseExceptions;
    QSet<QString> m_twoUpperLetterExceptions;
    QHash<QString, QString> m_autocorrectEntries;
};

class AutocorrectConfigDialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs an Autocorrect configuration dialog
     * @param autocorrect the autocorrection plugin
     * @param parent the parent widget
     */
    explicit AutocorrectConfigDialog(Autocorrect *autocorret, QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~AutocorrectConfigDialog();

private:
    AutocorrectConfig *ui;
};

#endif

