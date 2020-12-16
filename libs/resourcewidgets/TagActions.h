/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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


// ########### Actions ###########
///
/// \brief The SimpleExistingTagAction class defines an action that holds a resource and a tag
///
/// This is mostly used in ContextMenu for the context menu actions displaying tags to tag or untag
/// specific resource with a specific tag.
///
class SimpleExistingTagAction : public QAction
{
    Q_OBJECT
public:
    explicit SimpleExistingTagAction(KoResourceSP resource, KisTagSP tag, QObject* parent = 0);
    ~SimpleExistingTagAction() override;

Q_SIGNALS:
    void triggered(KisTagSP tag, KoResourceSP resource);

protected Q_SLOTS:
    void onTriggered();

private:
    ///
    /// \brief m_resource resource associated with the action
    ///
    KoResourceSP m_resource;
    ///
    /// \brief m_tag tag associated with the action
    ///
    KisTagSP m_tag;
};


// ########### Line Edit Actions ###########
// ---------------------------
/**
 *  A line edit QWidgetAction.
 *  Default behavior: Closes its parent upon triggering.
 *  This is a base for all tag/resources actions that needs a user text input
 *  (for example create new tag or rename tag)
 */

///
/// \brief The LineEditAction class defines an action with a user text input
///
/// This is a base for all tag/resources actions that needs a user text input
/// (for example create new tag or rename tag).
/// By default it closes its parent upon triggering and clears the content,
///  but it can be disabled.
///
class LineEditAction : public QWidgetAction
{
    Q_OBJECT
protected:
    LineEditAction(QObject* parent);

public:
    ~LineEditAction() override;
    void setIcon(const QIcon &icon);
    void setCloseParentOnTrigger(bool closeParent);
    bool closeParentOnTrigger();

    void setPlaceholderText(const QString& clickMessage);
    void setText(const QString& text);
    void setVisible(bool showAction);

protected Q_SLOTS:

    ///
    /// \brief slotActionTriggered defines all behaviour expressed when the widget is triggered
    ///
    /// It contains logic for closing the widget and calls onTriggered() to make sure
    ///   behaviours defined in classes inheriting LineEditAction are called.
    ///
    void slotActionTriggered();

    ///
    /// \brief onTriggered defines additional behaviour for the action
    ///
    /// This function is called in slotActionTriggered() *before* the widget is closed
    ///   and cleared.
    ///
    virtual void onTriggered() {}

    ///
    /// \brief userText getter for the text inside the edit box
    /// \return text inside edit box (user input)
    ///
    QString userText();

private:
    bool m_closeParentOnTrigger;
    QLabel *m_label;
    QLineEdit *m_editBox;
    QPushButton *m_AddButton;
};

///
/// \brief The UserInputTagAction class defines an action with user text input that sends a signal with a simple QString
///
/// It inherits all behaviour from LineEditAction.
/// When triggered, it sends a signal with the content of the edit box.
///
/// Usages:
/// - Create a new tag
/// - Rename a current tag (the responsibility to know which tag is current
///    depends on the external widget like KisTagChooserWidget)
///
class UserInputTagAction : public LineEditAction
{
    Q_OBJECT

public:
    explicit UserInputTagAction(QObject* parent);
    ~UserInputTagAction() override;

Q_SIGNALS:
    void triggered(const QString &newTagName);

protected Q_SLOTS:
    void onTriggered() override;
};

///
/// \brief The NewTagResourceAction class defines an action that sends a signal with QString and a saved resource
///
/// It inherits all behaviours from LineEditAction.
/// When triggered, it sends a signal with the content of the edit box as QString, and the saved resource.
///
/// Usages:
/// - Tag a resource
/// - Untag a resource
///
class NewTagResourceAction : public LineEditAction
{
    Q_OBJECT

public:
    explicit NewTagResourceAction(KoResourceSP resource, QObject* parent);
    ~NewTagResourceAction() override;

    void setResource(KoResourceSP resource);

Q_SIGNALS:
    void triggered(const QString &newTagName, KoResourceSP resource);

protected Q_SLOTS:

    void onTriggered() override;

private:
    KoResourceSP m_resource;
};

// ########### Tag Comparer ###########

///
/// \brief The CompareWithOtherTagFunctor class defines a comparer for tags
///
/// It contains a saved tag and can be used to determine if another tag is equal
///   to the saved tag ("referece tag") or not. It can be used in stl list features
///   like erase() etc.
///
class CompareWithOtherTagFunctor
{
    /// Tag to compare all other tags to
    KisTagSP m_referenceTag;

public:
    ///
    /// \brief CompareWithOtherTagFunctor defines a default constructor
    /// \param referenceTag a tag to compare all other tags to
    ///
    CompareWithOtherTagFunctor(KisTagSP referenceTag);

    ///
    /// \brief operator () contains comparison logic
    /// \param otherTag a tag to compare with the reference tag
    /// \return
    ///
    bool operator()(KisTagSP otherTag);

    ///
    /// \brief setReferenceTag sets a reference tag in the comparer
    /// \param referenceTag a tag that is used to compare other tags to
    ///
    void setReferenceTag(KisTagSP referenceTag);

    ///
    /// \brief referenceTag is a getter for the reference tag
    /// \return a tag that is used to compare other tags to
    ///
    KisTagSP referenceTag();

};


#endif // TAGACTIONS_H
