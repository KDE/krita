/*
 *  Author 2021 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_TAG_SELECTION_WIDGET_H
#define KIS_TAG_SELECTION_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVariant>

#include <KoID.h>

#include <kritawidgets_export.h>

struct CustomTag
{
    QString name;
    QString shortName;
    QVariant data;

public:
    CustomTag(QString _name, QVariant _data)
    {

        name = _name;
        shortName = _name;
        data = _data;
    }

    CustomTag(QString _name, QString _shortName, QVariant _data)
    {

        name = _name;
        shortName = _shortName;
        data = _data;
    }

    CustomTag()
    {
        name = "";
        shortName = "";
        data = QVariant();
    }

};
typedef QSharedPointer<CustomTag> CustomTagSP;
Q_DECLARE_METATYPE(QSharedPointer<CustomTag>)


struct CustomTagsCategory
{
    QString categoryName;
    // TODO: add category name/resource type, but translated
    QList<CustomTagSP> tags;
};
typedef QSharedPointer<CustomTagsCategory> CustomTagsCategorySP;


class KRITAWIDGETS_EXPORT WdgCloseableLabel : public QWidget
{
    Q_OBJECT

public:
    explicit WdgCloseableLabel(KoID tag, bool editable, bool semiSelected = false, QWidget *parent = 0);
    ~WdgCloseableLabel() override;

Q_SIGNALS:
    void sigRemoveTagFromSelection(KoID tag);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel* m_textLabel;
    QLabel* m_closeIconLabel;

    bool m_editble;
    bool m_semiSelected;
    KoID m_tag;

    friend class TestKisTagSelectionWidget;

};


class WdgAddTagButton : public QToolButton
{

    Q_OBJECT

public:
    explicit WdgAddTagButton(QWidget* parent);
    ~WdgAddTagButton() override;

    void setAvailableTagsList(QList<KoID> &notSelected);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_size { 21 };

    friend class TestKisTagSelectionWidget;

};

class KRITAWIDGETS_EXPORT KisTagSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KisTagSelectionWidget(QWidget *parent = 0);
    ~KisTagSelectionWidget() override;

    void setTagList(bool editable, QList<KoID> &selected, QList<KoID> &notSelected);
    void setTagList(bool editable, QList<KoID> &selected, QList<KoID> &notSelected, QList<KoID> &semitSelected);


Q_SIGNALS:
    void sigAddTagToSelection(KoID tag);
    void sigRemoveTagFromSelection(KoID tag);

private Q_SLOTS:

    void slotAddTagToSelection(QAction* action);
    void slotRemoveTagFromSelection(KoID tag);

private:
    QLayout* m_layout;
    QToolButton* m_addTagButton;
    bool m_editable;
    bool m_categorized;

    friend class TestKisTagSelectionWidget;
};

#endif // KIS_TAG_SELECTION_WIDGET_H
