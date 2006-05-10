//Added by qt3to4:
#include <QResizeEvent>
/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description : 
 * 
 * Copyright 2005 by Tom Albers
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

/** @file squeezedcombobox.h */

#ifndef SQUEEZEDCOMBOBOX_H
#define SQUEEZEDCOMBOBOX_H

class QTimer;

// Qt includes.

#include <QComboBox>
#include <QToolTip>
#include <krita_export.h>

class SqueezedComboBox;

/** @class SqueezedComboBoxTip
 * This class shows a tooltip for a SqueezedComboBox
 * the tooltip will contain the full text and helps
 * the user find the correct entry. It is automatically
 * activated when starting a SqueezedComboBox. This is
 * inherited from QToolTip
 * 
 * @author Tom Albers
 */
// class SqueezedComboBoxTip : public QToolTip
// {
// public:
//     /**
//      * Constructor. An example call (as done in
//      * SqueezedComboBox::SqueezedComboBox):
//      * @code
//      * t = new SqueezedComboBoxTip( this->listBox()->viewport(), this );
//      * @endcode
//      *
//      * @param parent parent widget (viewport)
//      * @param name parent widget
//      */
//     SqueezedComboBoxTip( QWidget *parent, SqueezedComboBox *name );
//
// protected:
//     /**
//      * Reimplemented version from QToolTip which shows the
//      * tooltip when needed.
//      * @param  pos the point where the mouse currently is
//      */
//     void maybeTip( const QPoint& pos );
//
// private:
//     SqueezedComboBox*        m_originalWidget;
// };

/** @class SqueezedComboBox
 *
 * This widget is a QComboBox, but then a little bit
 * different. It only shows the right part of the items
 * depending on de size of the widget. When it is not
 * possible to show the complete item, it will be shortened
 * and "..." will be prepended.
 *
 * @image html squeezedcombobox.png "This is how it looks"
 * @author Tom Albers
 */
class KRITAUI_EXPORT SqueezedComboBox : public QComboBox
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent parent widget
     * @param name name to give to the widget
     */
    SqueezedComboBox(QWidget *parent = 0, const char *name = 0 );

    /**
     * destructor
     */
    virtual ~SqueezedComboBox();

    bool contains(const QString & text) const;

    /**
     * This inserts a item to the list. See QComboBox::insertItem()
     * for detaills. Please do not use QComboBox::insertItem() to this
     * widget, as that will fail.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param index the position in the widget.
     */
    void insertSqueezedItem(const QString& newItem, int index);

    /**
     * Append an item.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     */
    void addSqueezedItem(const QString& newItem);

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
    QString itemHighlighted( );

    /**
     * Sets the sizeHint() of this widget.
     */
    virtual QSize sizeHint() const;

private slots:
    void slotTimeOut();
    void slotUpdateToolTip( int index );

private:
    void resizeEvent ( QResizeEvent * );
    QString squeezeText( const QString& original);

    // Prevent these from being used.
    void setCurrentText(const QString& itemText);
    void insertItem(const QString &text);
    void insertItem(qint32 index, const QString &text);
    void addItem(const QString &text);

    QMap<int,QString>   m_originalItems;
    QTimer*             m_timer;
    //SqueezedComboBoxTip*      m_tooltip; XXX
};

#endif // SQUEEZEDCOMBOBOX_H
