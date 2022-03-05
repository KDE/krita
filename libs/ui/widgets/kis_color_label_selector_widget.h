/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLOR_LABEL_SELECTOR_WIDGET_H
#define __KIS_COLOR_LABEL_SELECTOR_WIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "kritaui_export.h"


class KRITAUI_EXPORT KisColorLabelSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    enum SelectionIndicationType {
        /**
         * @brief The buttons show a border and their interior is filled when
         *        selected and empty when not selected
         */
        FillIn,
        /**
         * @brief The buttons have their interior filled and show a border if
         *        they are selected
         */
        Outline
    };

    KisColorLabelSelectorWidget(QWidget *parent = nullptr);
    ~KisColorLabelSelectorWidget() override;

    /**
     * @brief Get if the button at the given position is currently checked
     */
    bool isButtonChecked(int index) const;
    /**
     * @brief Get the index of the currently checked button. Returns -1 if
     *        there is no checked button and -2 if the buttons are not
     *        mutually exclusive
     */
    int currentIndex() const;
    /**
     * @brief Get the list of checked button indices
     */
    QList<int> selection() const;

    /**
     * @brief Get if the button selection is mutually exclusive
     */
    bool exclusive() const;
    /**
     * @brief Set if the button selection is mutually exclusive. This allows
     *        switching between single or multiple selection
     */
    void setExclusive(bool exclusive);

    /**
     * @brief Get if the buttons should wrap in multiple lines if there is no
     *        enough space horizontally
     */
    bool buttonWrapEnabled() const;
    /**
     * @brief Set if the buttons should wrap in multiple lines if there is no
     *        enough space horizontally
     */
    void setButtonWrapEnabled(bool enabled);

    /**
     * @brief Get if the user can drag to check/uncheck multiple buttons
     */
    bool mouseDragEnabled() const;
    /**
     * @brief Set if the user can drag to check/uncheck multiple buttons
     */
    void setMouseDragEnabled(bool enabled);

    /**
     * @brief Get how the button is rendered
     */
    SelectionIndicationType selectionIndicationType() const;
    /**
     * @brief Set how the button should be rendered
     */
    void setSelectionIndicationType(SelectionIndicationType type);

    /**
     * @brief Get the size of the buttons
     */
    int buttonSize() const;
    /**
     * @brief Set the size of the buttons
     */
    void setButtonSize(int size);

public Q_SLOTS:
    /**
     * @brief (Un)Check the button at the given position
     */
    void setButtonChecked(int index, bool state);
    /**
     * @brief Set the currently selected button. It has no effect if the buttons
     *        are mutually exclusive
     */
    void setCurrentIndex(int index);
    /**
     * @brief Set the list of checked button indices.
     */
    void setSelection(const QList<int> &indices);

Q_SIGNALS:
    void buttonToggled(int index, bool state);
    void currentIndexChanged(int index);
    void selectionChanged();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT KisColorLabelSelectorWidgetMenuWrapper : public QWidget
{
    Q_OBJECT

public:
    KisColorLabelSelectorWidgetMenuWrapper(QWidget *parent = nullptr);
    ~KisColorLabelSelectorWidgetMenuWrapper() override;

    KisColorLabelSelectorWidget* colorLabelSelector() const;
    
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* e) override;

    int calculateMenuOffset() const;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLOR_LABEL_SELECTOR_WIDGET_H */
