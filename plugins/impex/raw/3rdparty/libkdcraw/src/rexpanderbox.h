/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-03-14
 * @brief  A widget to host settings as expander box
 *
 * @author Copyright (C) 2008-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2008-2013 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 * @author Copyright (C) 2010 by Manuel Viet
 *         <a href="mailto:contact at 13zenrv dot fr">contact at 13zenrv dot fr</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef REXPANDERBOX_H
#define REXPANDERBOX_H

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QScrollArea>

// KDE includes

#include <kconfiggroup.h>

// Local includes


#include "rwidgetutils.h"

namespace KDcrawIface
{

class  RClickLabel : public QLabel
{
    Q_OBJECT

public:

    RClickLabel(QWidget* const parent = 0);
    explicit RClickLabel(const QString& text, QWidget* const parent = 0);
    ~RClickLabel() override;

Q_SIGNALS:

    /// Emitted when activated by left mouse click
    void leftClicked();
    /// Emitted when activated, by mouse or key press
    void activated();

protected:

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

// -------------------------------------------------------------------------

class  RSqueezedClickLabel : public RAdjustableLabel
{
    Q_OBJECT

public:

    RSqueezedClickLabel(QWidget* const parent = 0);
    explicit RSqueezedClickLabel(const QString& text, QWidget* const parent = 0);
    ~RSqueezedClickLabel() override;

Q_SIGNALS:

    void leftClicked();
    void activated();

protected:

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

// -------------------------------------------------------------------------

class  RArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    RArrowClickLabel(QWidget* const parent = 0);
    ~RArrowClickLabel() override;

    void setArrowType(Qt::ArrowType arrowType);
    Qt::ArrowType arrowType() const;

    QSize sizeHint () const override;

Q_SIGNALS:

    void leftClicked();

protected:

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

protected:

    Qt::ArrowType m_arrowType;
    int           m_size;
    int           m_margin;
};

// -------------------------------------------------------------------------

class  RLabelExpander : public QWidget
{
    Q_OBJECT

public:

    RLabelExpander(QWidget* const parent = 0);
    ~RLabelExpander() override;

    void setCheckBoxVisible(bool b);
    bool checkBoxIsVisible() const;

    void setChecked(bool b);
    bool isChecked() const;

    void setLineVisible(bool b);
    bool lineIsVisible() const;

    void setText(const QString& txt);
    QString text() const;

    void setIcon(const QIcon &icon);
    QIcon icon() const;

    void setWidget(QWidget* const widget);
    QWidget* widget() const;

    void setExpanded(bool b);
    bool isExpanded() const;

    void setExpandByDefault(bool b);
    bool isExpandByDefault() const;

Q_SIGNALS:

    void signalExpanded(bool);
    void signalToggled(bool);

private Q_SLOTS:

    void slotToggleContainer();

private:

    bool eventFilter(QObject* obj, QEvent* ev) override;

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class  RExpanderBox : public QScrollArea
{
    Q_OBJECT

public:

    RExpanderBox(QWidget* const parent = 0);
    ~RExpanderBox() override;

    /** Add RLabelExpander item at end of box layout with these settings :
        'w'               : the widget hosted by RLabelExpander.
        'pix'             : pixmap used as icon to item title.
        'txt'             : text used as item title.
        'objName'         : item object name used to read/save expanded settings to rc file.
        'expandBydefault' : item state by default (expanded or not).
     */
    void addItem(QWidget* const w, const QIcon &icon, const QString& txt,
                 const QString& objName, bool expandBydefault);
    void addItem(QWidget* const w, const QString& txt,
                 const QString& objName, bool expandBydefault);

    /** Insert RLabelExpander item at box layout index with these settings :
        'w'               : the widget hosted by RLabelExpander.
        'pix'             : pixmap used as icon to item title.
        'txt'             : text used as item title.
        'objName'         : item object name used to read/save expanded settings to rc file.
        'expandBydefault' : item state by default (expanded or not).
     */
    void insertItem(int index, QWidget* const w, const QIcon &icon, const QString& txt,
                    const QString& objName, bool expandBydefault);
    void insertItem(int index, QWidget* const w, const QString& txt,
                    const QString& objName, bool expandBydefault);

    void removeItem(int index);

    void setCheckBoxVisible(int index, bool b);
    bool checkBoxIsVisible(int index) const;

    void setChecked(int index, bool b);
    bool isChecked(int index) const;

    void setItemText(int index, const QString& txt);
    QString itemText (int index) const;

    void setItemIcon(int index, const QIcon &icon);
    QIcon itemIcon(int index) const;

    void setItemToolTip(int index, const QString& tip);
    QString itemToolTip(int index) const;

    void setItemEnabled(int index, bool enabled);
    bool isItemEnabled(int index) const;

    void addStretch();
    void insertStretch(int index);

    void setItemExpanded(int index, bool b);
    bool isItemExpanded(int index) const;

    int  count() const;

    RLabelExpander* widget(int index) const;
    int indexOf(RLabelExpander* const widget) const;

    virtual void readSettings(KConfigGroup& group);
    virtual void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalItemExpanded(int index, bool b);
    void signalItemToggled(int index, bool b);

private Q_SLOTS:

    void slotItemExpanded(bool b);
    void slotItemToggled(bool b);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class  RExpanderBoxExclusive : public RExpanderBox
{
    Q_OBJECT

public:

    RExpanderBoxExclusive(QWidget* const parent = 0);
    ~RExpanderBoxExclusive() override;

    /** Show one expander open at most */
    void setIsToolBox(bool b);
    bool isToolBox() const;

private Q_SLOTS:

    void slotItemExpanded(bool b);

private:

    bool m_toolbox;
};

} // namespace KDcrawIface

#endif // REXPANDERBOX_H
