/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KORESOURCESELECTOR_H
#define KORESOURCESELECTOR_H

#include "kowidgets_export.h"
#include <QtGui/QComboBox>

class QMouseEvent;
class KoAbstractResourceServerAdapter;
class KoResource;

/**
 * A custom combobox widget for selecting resource items like gradients or patterns.
 */
class KOWIDGETS_EXPORT KoResourceSelector : public QComboBox
{
    Q_OBJECT
public:
    enum DisplayMode {
        ImageMode,  ///< Displays image of resources (default)
        TextMode   ///< Displays name of resources
    };

    /**
     * Constructs a new resource selector.
     * @param parent the parent widget
     */
    explicit KoResourceSelector(QWidget *parent = 0);

    /**
     * Constructs a new resource selector showing the resources of the given resource adapter.
     * @param resourceAdapter the resource adapter providing the resources to display
     * @param parent the parent widget
     */
    explicit KoResourceSelector( KoAbstractResourceServerAdapter * resourceAdapter, QWidget * parent = 0 );

    /// Destroys the resource selector
    virtual ~KoResourceSelector();

    /// Sets the resource adaptor to get resources from
    void setResourceAdapter(KoAbstractResourceServerAdapter *resourceAdapter);

    /// Sets the display mode
    void setDisplayMode(DisplayMode mode);

    /// Sets number of columns to display in the popup view
    void setColumnCount( int columnCount );

    /// Sets the height of the popup view rows
    void setRowHeight( int rowHeight );

signals:
    /// Emitted when a resource was selected
    void resourceSelected( KoResource * resource );

    /// Is emitted when the user has clicked on the current resource
    void resourceApplied( KoResource * resource );

protected:
    /// reimplemented
    virtual void paintEvent( QPaintEvent * );
    /// reimplemented
    virtual void mousePressEvent( QMouseEvent * );
    /// reimplemented
    virtual void mouseMoveEvent( QMouseEvent * event );

private slots:
    void indexChanged( int index );
    void resourceAdded(KoResource*);
    void resourceRemoved(KoResource*);
private:
    class Private;
    Private * const d;
};

#endif // KORESOURCESELECTOR_H
