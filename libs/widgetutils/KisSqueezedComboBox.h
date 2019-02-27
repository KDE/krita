/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description :
 *
 * Copyright 2005 by Tom Albers <tomalbers@kde.nl>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KISSQUEEZEDCOMBOBOX_H
#define KISSQUEEZEDCOMBOBOX_H

class QTimer;
class QResizeEvent;
class QWidget;

// Qt includes.

#include <QComboBox>
#include <QWidget>
#include <QIcon>

#include "kritawidgetutils_export.h"

/** @class KisSqueezedComboBox
 *
 * This widget is a QComboBox, but then a little bit
 * different. It only shows the right part of the items
 * depending on de size of the widget. When it is not
 * possible to show the complete item, it will be shortened
 * and "..." will be prepended.
 *
 * @image html KisSqueezedComboBox.png "This is how it looks"
 * @author Tom Albers
 */
class KRITAWIDGETUTILS_EXPORT KisSqueezedComboBox : public QComboBox
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param parent parent widget
     * @param name name to give to the widget
     */
    KisSqueezedComboBox(QWidget *parent = 0, const char *name = 0);
    /**
     * destructor
     */
    ~KisSqueezedComboBox() override;

    /**
     *
     * Returns true if the combobox contains the original (not-squeezed)
     * version of text.
     * @param text the original (not-squeezed) text to check for
     */
    bool contains(const QString & text) const;

    /**
     * Returns index of a original text, -1 if the text isn't found
     * @param text the original (not-squeezed) text to search for
     */
    qint32 findOriginalText(const QString & text) const;


    /**
     * Return the list of original text items
     */
    QStringList originalTexts() const;

    /**
     * Reset the combo box and initialize it with the list of (original) text items
     */
    void resetOriginalTexts(const QStringList &texts);

    /**
     * This inserts a item to the list. See QComboBox::insertItem()
     * for details. Please do not use QComboBox::insertItem() to this
     * widget, as that will fail.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param index the position in the widget.
     * @param userData the user data.
     */
    void insertSqueezedItem(const QString& newItem, int index, QVariant userData = QVariant());
    void insertSqueezedItem(const QIcon &icon, const QString& newItem, int index, QVariant userData = QVariant());

    /**
     * Append an item.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param userData the user data.
     */
    void addSqueezedItem(const QString& newItem, QVariant userData = QVariant());

    /**
     * Append an item.
     * @param icon    the item icon
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param userData the user data
     */
    void addSqueezedItem(const QIcon &icon, const QString& newItem, QVariant userData = QVariant());

    /**
     * Set the current item to the one matching the given text.
     *
     * @param itemText the original (long version) of the item text
     */
    void setCurrent(const QString& itemText);

    /**
     * This method returns the full text (not squeezed) of the currently
     * highlighted item.
     * @return full text of the highlighted item
     */
    QString itemHighlighted();

    /**
     * remove the squeezed item at index
     */
    void removeSqueezedItem(int index);

    /**
     * Sets the sizeHint() of this widget.
     */
    QSize sizeHint() const override;

    static QString squeezeText(const QString& original, const QWidget *widget);

private Q_SLOTS:
    void slotTimeOut();

private:
    void resizeEvent(QResizeEvent *) override;

    // Prevent these from being used.
    void setCurrentText(const QString& itemText);
    void insertItem(const QString &text);
    void insertItem(qint32 index, const QString &text);
    void addItem(const QString &text);

    QMap<int, QString> m_originalItems;
    QTimer *m_timer;

};

#endif // SQUEEZEDCOMBOBOX_H
