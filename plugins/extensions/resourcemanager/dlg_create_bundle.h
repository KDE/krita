/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <KoDialog.h>

#include <KoResourceBundle.h>

namespace Ui
{
class WdgDlgCreateBundle;
}

class DlgCreateBundle : public KoDialog
{
    Q_OBJECT

public:
    explicit DlgCreateBundle(KoResourceBundleSP bundle = nullptr, QWidget *parent = 0);
    ~DlgCreateBundle() override;

    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;
    QString saveLocation() const;
    QString previewImage() const;

private Q_SLOTS:

    void accept() override;
    void reject() override;

    void selectSaveLocation();
    void addSelected();
    void removeSelected();
    void resourceTypeSelected(int idx);
    void getPreviewImage();
    void saveToConfiguration(bool full);
    void slotEmbedTags();
    QVector<KisTagSP> getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags) const;


private:

    void putResourcesInTheBundle() const;

    QWidget *m_page;
    Ui::WdgDlgCreateBundle *m_ui;

    QList<int> m_selectedResourcesIds;
    QList<int> m_selectedTagIds;

    QString m_previewImage;

    KoResourceBundleSP m_bundle;
};

#endif // KOBUNDLECREATIONWIDGET_H
