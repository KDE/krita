/*
 *  Copyright (c) 2020 Agata Cacko cacko.azh@gmail.com
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
#ifndef DLG_EMBED_TAGS_H
#define DLG_EMBED_TAGS_H

#include <KoDialog.h>

#include <KoResourceBundle.h>

namespace Ui
{
class WdgDlgEmbedTags;
}

class DlgEmbedTags : public KoDialog
{
    Q_OBJECT

public:
    explicit DlgEmbedTags(QList<int> selectedTags, QWidget *parent = 0);
    ~DlgEmbedTags() override;

    QList<int> selectedTagIds();

private Q_SLOTS:

    void addSelected();
    void removeSelected();
    void resourceTypeSelected(int idx);

private:

    QWidget *m_page;
    Ui::WdgDlgEmbedTags *m_ui;

    QList<int> m_selectedTagIds;
};

#endif // KOBUNDLECREATIONWIDGET_H
