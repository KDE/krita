/* This file is part of the KDE project
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#ifndef SECTIONFORMATDIALOG_H
#define SECTIONFORMATDIALOG_H

#include <KoDialog.h>

class KoTextEditor;
class KoSection;
class KoSectionModel;

#include <ui_SectionFormatDialog.h>
class SectionFormatDialog : public KoDialog
{
    Q_OBJECT

public:
    explicit SectionFormatDialog(QWidget *parent, KoTextEditor *editor);

private Q_SLOTS:
    void sectionSelected(const QModelIndex &idx);
    void sectionNameChanged();
    void updateTreeState();

private:
    class ProxyModel;
    class SectionNameValidator;

    Ui::SectionFormatDialog m_widget;
    KoTextEditor *m_editor;
    QModelIndex m_curIdx;
    KoSectionModel *m_sectionModel;

    KoSection *sectionFromModel(const QModelIndex &idx);
};

#endif //SECTIONFORMATDIALOG_H
