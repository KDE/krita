/* This file is part of the KDE libraries

   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>
                 2006 Tobias Koenig <tokoe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDUALCOLORBUTTON_H
#define KDUALCOLORBUTTON_H

#include <kdelibs_export.h>

#include <QtGui/QWidget>

/**
 * @short A widget for selecting two related colors.
 *
 * KDualColorButton allows the user to select two cascaded colors (usually a
 * foreground and background color). Other features include drag and drop
 * from other KDE color widgets, a reset to black and white control, and a
 * swap colors control.
 *
 * When the user clicks on the foreground or background rectangle the
 * rectangle is first sunken and the selectionChanged() signal is emitted.
 * Further clicks will present a color dialog and emit either the foregroundColorChanged()
 * or backgroundColorChanged() if a new color is selected.
 *
 * Note: With drag and drop when dropping a color the current selected color
 * will be set, while when dragging a color it will use whatever color
 * rectangle the mouse was pressed inside.
 *
 * \image html kdualcolorbutton.png "KDE Dual Color Button"
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KDEUI_EXPORT KDualColorButton : public QWidget
{
    Q_OBJECT
    Q_ENUMS( Selection )
    Q_PROPERTY( QColor foregroundColor READ foregroundColor WRITE setForegroundColor )
    Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
    Q_PROPERTY( QColor currentColor READ currentColor WRITE setCurrentColor )
    Q_PROPERTY( Selection selection READ selection WRITE setSelection STORED false DESIGNABLE false )


  public:
    enum Selection {
      Foreground,
      Background
    };

    /**
     * Constructs a new KDualColorButton using the default black and white
     * colors.
     *
     * @param parent The parent widget of the KDualColorButton.
     * @param dialogParent The parent widget of the color selection dialog.
     */
    explicit KDualColorButton( QWidget *parent = 0, QWidget* dialogParent = 0 );

    /**
     * Constructs a new KDualColorButton with the supplied foreground and
     * background colors.
     *
     * @param parent The parent widget of the KDualColorButton.
     * @param dialogParent The parent widget of the color selection dialog.
     */
    KDualColorButton( const QColor &foregroundColor, const QColor &backgroundColor,
                      QWidget *parent = 0, QWidget* dialogParent = 0 );

    /**
     * Destroys the KDualColorButton.
     */
    ~KDualColorButton();

    /**
     * Returns the current foreground color.
     */
    QColor foregroundColor() const;

    /**
     * Returns the current background color.
     */
    QColor backgroundColor() const;

    /**
     * Returns the current color depending on the
     * selection.
     *
     * This is equal to
     *
     * \code
     *  if ( selection() == Foreground )
     *    return foregroundColor();
     *  else
     *    return backgroundColor();
     * \endcode
     */
    QColor currentColor() const;

    /**
     * Returns whether the foreground or background item
     * is selected.
     */
    Selection selection() const;

    /**
     * Returns the minimum size needed to display the widget and all its
     * controls.
     */
    virtual QSize sizeHint() const;

  public Q_SLOTS:
    /**
     * Sets the foreground color.
     */
    void setForegroundColor( const QColor &color );

    /**
     * Sets the background color.
     */
    void setBackgroundColor( const QColor &color );

    /**
     * Sets the color of the selected item.
     */
    void setCurrentColor( const QColor &color );

    /**
     * Sets the current selected color item.
     */
    void setSelection( Selection selection );

  Q_SIGNALS:
    /**
     * Emitted when the foreground color is changed.
     */
    void foregroundColorChanged( const QColor &color );

    /**
     * Emitted when the background color is changed.
     */
    void backgroundColorChanged( const QColor &color );

    /**
     * Emitted when the user changes the current color selection.
     */
    void selectionChanged( KDualColorButton::Selection selection );

  protected:
    /**
     * Sets the supplied rectangles to the proper size and position for the
     * current widget size. You can reimplement this to change the layout
     * of the widget. Restrictions are that the swap control will always
     * be at the top right, the reset control will always be at the bottom
     * left, and you must leave at least a 14x14 space in those corners.
     */
    virtual void metrics( QRect &foregroundRect, QRect &backgroundRect );

    virtual void paintEvent( QPaintEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dropEvent( QDropEvent *event );

  private:
    class Private;
    Private *const d;
};

#endif

