/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOPTIONCOLLECTIONWIDGET_H
#define KISOPTIONCOLLECTIONWIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <kritawidgetutils_export.h>

/**
 * @brief Class providing a list of widgets with some addons such as separators,
 *        orientation or individual widget visibility
 */
class KRITAWIDGETUTILS_EXPORT KisOptionCollectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisOptionCollectionWidget(QWidget *parent = nullptr);
    ~KisOptionCollectionWidget() override;

    /**
     * @brief Get the index of the widget that has the given id
     */
    int widgetIndexFromId(const QString &id) const;
    /**
     * @brief Get if the list contains a widget with the given id
     */
    bool containsWidget(const QString &id) const;

    /**
     * @brief Get the widget that is at the given position
     */
    QWidget* widget(int index) const;
    /**
     * @brief Get the widget that is at the given position casted to some other class
     */
    template <typename T>
    T widgetAs(int index) const
    {
        return qobject_cast<T>(widget(index));
    }

    /**
     * @brief Get the widget with the given id
     */
    QWidget* widget(const QString &id) const;
    /**
     * @brief Get the widget with the given id casted to some other class
     */
    template <typename T>
    T widgetAs(const QString &id) const
    {
        return qobject_cast<T>(widget(id));
    }

    /**
     * @brief Get the widget that is at the given path. The path must be a
     *        forward slash separated list of ids. If the list contains some
     *        other @ref KisOptionCollectionWidget or
     *        @ref KisOptionCollectionWidgetWithHeader, and they do as well,
     *        then they form a hierarchy tree, so the path is searched
     *        recursively through all those child widgets
     */
    QWidget* findWidget(const QString &path) const;
    /**
     * @brief Get the widget that is at the given path casted to some other class
     * @see findWidget
     */
    template <typename T>
    T findWidgetAs(const QString &path) const
    {
        return qobject_cast<T>(findWidget(path));
    }

    /**
     * @brief Insert the given widget with the given id at the given position.
     *        The list widget takes ownership of the inserted widget
     */
    void insertWidget(int index, const QString &id, QWidget *widget);
    /**
     * @brief Insert the given widget with the given id at the end of the list.
     *        The list widget takes ownership of the inserted widget
     */
    void appendWidget(const QString &id, QWidget *widget);

    /**
     * @brief Remove the widget that is at the given position from the list.
     *        This also destroys the widget
     */
    void removeWidget(int index);
    /**
     * @brief Remove the widget that has the given id from the list.
     *        This also destroys the widget
     */
    void removeWidget(const QString &id);

    /**
     * @brief Remove the widget that is at the given position from the list.
     *        The widget is returned instead of being destroyed
     */
    QWidget* takeWidget(int index);
    /**
     * @brief Remove the widget that has the given id from the list.
     *        The widget is returned instead of being destroyed
     */
    QWidget* takeWidget(const QString &id);

    /**
     * @brief Set the visibility of the widget that is at the given position.
     *        Use this function instead of the widget's one directly to get better
     *        visual results
     */
    void setWidgetVisible(int index, bool visible);
    /**
     * @brief Set the visibility of the widget that has the given id.
     *        Use this function instead of the widget's one directly to get better
     *        visual results
     */
    void setWidgetVisible(const QString &id, bool visible);

    /**
     * @brief Set the margins of the widgets. This allows to indent the widgets
     *        with respect to the separators. The separators themselves are not
     *        changed
     */
    void setWidgetsMargin(int margin);
    /**
     * @brief Set the visibility of the separators
     */
    void setSeparatorsVisible(bool visible);
    /**
     * @brief Set the orientation of the list of widgets
     * @param recursive If set to true and the list contains some
     *        @ref KisOptionCollectionWidget or @ref KisOptionCollectionWidgetWithHeader,
     *        then the orientation of those child widgets is also set, with the
     *        same recursive value
     */
    void setOrientation(Qt::Orientation orientation, bool recursive = false);

    /**
     * @brief Get the number of widgets in the list
     */
    int size() const;
    /**
     * @brief Get the number of visible widgets in the list
     */
    int numberOfVisibleWidgets() const;

private:
    class Private;
    QScopedPointer<Private> m_d;
};

/**
 * @brief Wrapper class around a @ref KisOptionCollectionWidget that also
 *        provide a header with a title label and an optional primary widget
 */
class KRITAWIDGETUTILS_EXPORT KisOptionCollectionWidgetWithHeader : public QWidget
{
    Q_OBJECT

public:
    KisOptionCollectionWidgetWithHeader(const QString &title, QWidget *parent = nullptr);
    ~KisOptionCollectionWidgetWithHeader() override;

    QSize minimumSizeHint() const override;
    /**
     * @brief Get the primary widget
     */
    QWidget* primaryWidget() const;
    /**
     * @brief Get the primary widget casted to some other class
     */
    template <typename T>
    T primaryWidgetAs() const
    {
        return qobject_cast<T>(primaryWidget());
    }
    /**
     * @brief Set the primary widget. The list widget takes ownership of it
     */
    void setPrimaryWidget(QWidget *widget);
    /**
     * @brief Remove the primary widget. This also destroys it
     */
    void removePrimaryWidget();
    /**
     * @brief Remove the primary widget. The widget is returned instead of
     *        being destroyed
     */
    QWidget* takePrimaryWidget();
    /**
     * @brief Set the visibility of the primary widget. Use this function
     *        instead of the widget one directly to get better visual results
     */
    void setPrimaryWidgetVisible(bool visible);

    /**
     * @brief Get the index of the widget that has the given id
     */
    int widgetIndexFromId(const QString &id) const;
    /**
     * @brief Get if the list contains a widget with the given id
     */
    bool containsWidget(const QString &id) const;

    /**
     * @brief Get the widget that is at the given position
     */
    QWidget* widget(int index) const;
    /**
     * @brief Get the widget that is at the given position casted to some other class
     */
    template <typename T>
    T widgetAs(int index) const
    {
        return qobject_cast<T>(widget(index));
    }

    /**
     * @brief Get the widget with the given id
     */
    QWidget* widget(const QString &id) const;
    /**
     * @brief Get the widget with the given id casted to some other class
     */
    template <typename T>
    T widgetAs(const QString &id) const
    {
        return qobject_cast<T>(widget(id));
    }

    /**
     * @brief Get the widget that is at the given path. The path must be a
     *        forward slash separated list of ids. If the list contains some
     *        other @ref KisOptionCollectionWidget or
     *        @ref KisOptionCollectionWidgetWithHeader, and they do as well,
     *        then they form a hierarchy tree, so the path is searched
     *        recursively through all those child widgets
     */
    QWidget* findWidget(const QString &path) const;
    /**
     * @brief Get the widget that is at the given path casted to some other class
     * @see findWidget
     */
    template <typename T>
    T findWidgetAs(const QString &path) const
    {
        return qobject_cast<T>(findWidget(path));
    }

    /**
     * @brief Insert the given widget with the given id at the given position.
     *        The list widget takes ownership of the inserted widget
     */
    void insertWidget(int index, const QString &id, QWidget *widget);
    /**
     * @brief Insert the given widget with the given id at the end of the list.
     *        The list widget takes ownership of the inserted widget
     */
    void appendWidget(const QString &id, QWidget *widget);

    /**
     * @brief Remove the widget that is at the given position from the list.
     *        This also destroys the widget
     */
    void removeWidget(int index);
    /**
     * @brief Remove the widget that has the given id from the list.
     *        This also destroys the widget
     */
    void removeWidget(const QString &id);

    /**
     * @brief Remove the widget that is at the given position from the list.
     *        The widget is returned instead of being destroyed
     */
    QWidget* takeWidget(int index);
    /**
     * @brief Remove the widget that has the given id from the list.
     *        The widget is returned instead of being destroyed
     */
    QWidget* takeWidget(const QString &id);

    /**
     * @brief Set the visibility of the widget that is at the given position
     */
    void setWidgetVisible(int index, bool visible);
    /**
     * @brief Set the visibility of the widget that has the given id
     */
    void setWidgetVisible(const QString &id, bool visible);

    /**
     * @brief Set the margins of the widgets. This allows to indent the widgets
     *        with respect to the separators. The separators themselves are not
     *        changed
     */
    void setWidgetsMargin(int margin);
    /**
     * @brief Set the visibility of the separators
     */
    void setSeparatorsVisible(bool visible);
    /**
     * @brief Set the orientation of the list of widgets
     * @param recursive If set to true and the list contains some
     *        @ref KisOptionCollectionWidget or @ref KisOptionCollectionWidgetWithHeader,
     *        then the orientation of those child widgets is also set, with the
     *        same recursive value
     */
    void setOrientation(Qt::Orientation orientation, bool recursive = false);

    /**
     * @brief Get the number of widgets in the list
     */
    int size() const;
    /**
     * @brief Get the number of visible widgets in the list
     */
    int numberOfVisibleWidgets() const;

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    class Private;
    QScopedPointer<Private> m_d;
    friend class KisOptionCollectionWidget;
};

#endif
