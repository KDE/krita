/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef PARAGRAPHSETTINGSDIALOG_H
#define PARAGRAPHSETTINGSDIALOG_H

#include <KoUnit.h>

#include <KDialog>
#include <QTextCursor>

class TextTool;
class ParagraphGeneral;

/// A dialog to show the settings for a paragraph
class ParagraphSettingsDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ParagraphSettingsDialog(TextTool *tool, QTextCursor *cursor, QWidget* parent = 0);
    ~ParagraphSettingsDialog();

    void setUnit(const KoUnit &unit);

signals:
    /// emitted when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &name);
    /// emitted when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

protected slots:
    void slotApply();
    void slotOk();

private:
    void initTabs();

    ParagraphGeneral *m_paragraphGeneral;
    TextTool *m_tool;
    QTextCursor *m_cursor;
    bool m_uniqueFormat;
};

#endif
