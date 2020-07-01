/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef TAGACTIONS_H
#define TAGACTIONS_H

#include <QMenu>
#include <QWidgetAction>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <KoResource.h>

#include <KisTag.h>
#include <KisTagModel.h>


class ExistingTagAction : public QAction
{
    Q_OBJECT
public:
    explicit ExistingTagAction(KoResourceSP resource, KisTagSP tag, QObject* parent = 0);
    ~ExistingTagAction() override;

Q_SIGNALS:
    void triggered(KoResourceSP resource, KisTagSP tag);

protected Q_SLOTS:
    void onTriggered();

private:
    KoResourceSP m_resource;
    KisTagSP m_tag;
};

/**
 *  A line edit QWidgetAction.
 *  Default behavior: Closes its parent upon triggering.
 */
class TagEditAction : public QWidgetAction
{
    Q_OBJECT
public:
    explicit TagEditAction(KoResourceSP resource, KisTagSP tag, QObject* parent);
    ~TagEditAction() override;
    void setIcon(const QIcon &icon);
    void closeParentOnTrigger(bool closeParent);
    bool closeParentOnTrigger();
    void setPlaceholderText(const QString& clickMessage);
    void setText(const QString& text);
    void setVisible(bool showAction);
    void setTag(KisTagSP tag);

Q_SIGNALS:

    void triggered(const KisTagSP tag);
    void triggered(const QString &tag);
    void triggered(KoResourceSP resource, const QString &tag);

protected Q_SLOTS:

    void onTriggered();

private:
    bool m_closeParentOnTrigger;
    QLabel *m_label;
    QLineEdit *m_editBox;
    QPushButton *m_AddButton;
    KisTagSP m_tag;
    KoResourceSP m_resource;
};

class NewTagAction : public TagEditAction
{
    Q_OBJECT
public:
    explicit NewTagAction(KoResourceSP resource, QMenu* parent);
    ~NewTagAction() override;
};


class CompareWithOtherTagFunctor
{
    KisTagSP m_referenceTag;

public:
    CompareWithOtherTagFunctor(KisTagSP referenceTag);

    bool operator()(KisTagSP otherTag);

    void setReferenceTag(KisTagSP referenceTag);

    KisTagSP referenceTag();

};


#endif // TAGACTIONS_H
