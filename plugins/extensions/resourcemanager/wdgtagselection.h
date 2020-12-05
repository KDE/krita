/*
 *  Copyright (c) 2020 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef WDG_TAG_SELECTION_H
#define WDG_TAG_SELECTION_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QToolButton>
#include <QHBoxLayout>

#include <KisTag.h>
#include <KoResource.h>
#include <KisResourceModelProvider.h>
#include <KisTagModelProvider.h>

#include <KisTagSelectionWidget.h>

class KisWdgTagSelectionControllerOneResource : public QObject
{
    Q_OBJECT

public:
    KisWdgTagSelectionControllerOneResource(WdgTagSelection* widget, bool editable);
    ~KisWdgTagSelectionControllerOneResource() override;


    void setResource(KoResourceSP resource);


private Q_SLOTS:
    void slotRemoveTag(KoID tag);
    void slotAddTag(KoID tag);

private:
    void updateView();

private:
    WdgTagSelection* m_tagSelectionWidget {0};
    bool m_editable {true};
    KoResourceSP m_resource;

    QSharedPointer<KisTagModel> m_tagModel;
    QSharedPointer<KisTagResourceModel> m_tagResourceModel;
};


class KisWdgTagSelectionControllerBundleTags : public QObject
{
    Q_OBJECT

public:
    KisWdgTagSelectionControllerBundleTags(WdgTagSelection* widget, bool editable);
    ~KisWdgTagSelectionControllerBundleTags() override;

    QList<int> getSelectedTagIds() const;

    void updateView();
    void setResourceType(const QString& resourceType);


private Q_SLOTS:
    void slotRemoveTag(KoID tag);
    void slotAddTag(KoID tag);

private:

    struct TagResourceType
    {
        KisTagSP tag;
    };

private:
    WdgTagSelection* m_tagSelectionWidget {0};
    bool m_editable {true};
    QString m_resourceType {""};

    QList<KoID> m_selectedTags;

    QMap<QString, QList<KoID>> m_selectedTagsByResourceType;

    QSharedPointer<KisTagModel> m_tagModel;
    QSharedPointer<KisTagResourceModel> m_tagResourceModel;
};


#endif // WDG_TAG_SELECTION_H
