/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko cacko.azh @gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
