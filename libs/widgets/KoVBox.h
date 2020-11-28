/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KoVBOX_H
#define KoVBOX_H

#include <kritawidgets_export.h>

#include <QFrame>

class QChildEvent;

/**
 * A container widget which arranges its children vertically.
 * 
 * When using a KoVBox you don't need to create a layout nor
 * to add the child widgets to it.
 *
 * Both margin and spacing are initialized to 0. Use QHBoxLayout
 * if you need standard layout margins.
 *
 * @see KVBox
 */
class KRITAWIDGETS_EXPORT KoVBox : public QFrame
{
    Q_OBJECT

public:
    /**
     * Creates a new hbox.
     */
    explicit KoVBox(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~KoVBox() override;

    /**
     * Sets the @p margin of the hbox.
     */
    void setMargin(int margin);

    /**
     * Sets the spacing between the child widgets to @p space.
     *
     * To get the default layout spacing, set @p space to -1.
     */
    void setSpacing(int space);

    /**
     * Sets the stretch factor of @p widget to @p stretch.
     */
    void setStretchFactor(QWidget *widget, int stretch);

    /**
     * Calculate the recommended size for this hbox.
     */
    QSize sizeHint() const override;

    /**
     * Calculate the recommended minimum size for this hbox.
     */
    QSize minimumSizeHint() const override;

protected:

    void childEvent(QChildEvent *ev) override;

private:
    class Private;
    friend class Private;
    Private *const d;

    Q_DISABLE_COPY(KoVBox)
};

#endif
