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

class KoColor;
/**
 * @short A widget for selecting two related colors.
 *
 * KoDualColorButton allows the user to select two cascaded colors (usually a
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
 * \image html kdualcolorbutton.png "KOffice Dual Color Button"
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KDEUI_EXPORT KoDualColorButton : public QWidget
{
    Q_OBJECT
    Q_ENUMS( Selection )
    Q_PROPERTY( KoColor foregroundColor READ foregroundColor WRITE setForegroundColor )
    Q_PROPERTY( KoColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
    Q_PROPERTY( KoColor currentColor READ currentColor WRITE setCurrentColor )
    Q_PROPERTY( bool popDialog READ popDialog WRITE setPopDialog )
    Q_PROPERTY( Selection selection READ selection WRITE setSelection STORED false DESIGNABLE false )


  public:
    enum Selection {
      Foreground,
      Background
    };

    /**
     * Constructs a new KoDualColorButton using the default black and white
     * colors.
     *
     * @param parent The parent widget of the KoDualColorButton.
     * @param dialogParent The parent widget of the color selection dialog.
     */
    explicit KoDualColorButton( QWidget *parent = 0, QWidget* dialogParent = 0 );

    /**
     * Constructs a new KoDualColorButton with the supplied foreground and
     * background colors.
     *
     * @param parent The parent widget of the KoDualColorButton.
     * @param dialogParent The parent widget of the color selection dialog.
     */
    KoDualColorButton( const KoColor &foregroundColor, const KoColor &backgroundColor,
                      QWidget *parent = 0, QWidget* dialogParent = 0 );

    /**
     * Destroys the KoDualColorButton.
     */
    ~KoDualColorButton();

    /**
     * Returns the current foreground color.
     */
    KoColor foregroundColor() const;

    /**
     * Returns the current background color.
     */
    KoColor backgroundColor() const;

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
    KoColor currentColor() const;

    /**
     * Returns whether the foreground or background item
     * is selected.
     */
    Selection selection() const;

    /**
     * Returns if a dialog with a KoUniColorChooser will be popped up when clicking
     * If false then you could/should connect to the pleasePopDialog signal
     * and pop your own dialog. Just set the current color afterwards.
     */
    bool popDialog() const;

    /**
     * Returns the minimum size needed to display the widget and all its
     * controls.
     */
    virtual QSize sizeHint() const;

  public Q_SLOTS:
    /**
     * Sets the foreground color.
     */
    void setForegroundColor( const KoColor &color );

    /**
     * Sets the background color.
     */
    void setBackgroundColor( const KoColor &color );

    /**
     * Sets the color of the selected item.
     */
    void setCurrentColor( const KoColor &color );

    /**
     * Sets the current selected color item.
     */
    void setSelection( Selection selection );

    /**
     * Sets if a dialog with a KoUniColorChooser should be popped up when clicking
     * If you set this to false then you could connect to the pleasePopDialog signal
     * and pop your own dialog. Just set the current color afterwards.
     */
    void setPopDialog( bool popDialog );

  Q_SIGNALS:
    /**
     * Emitted when the foreground color is changed.
     */
    void foregroundColorChanged( const KoColor &color );

    /**
     * Emitted when the background color is changed.
     */
    void backgroundColorChanged( const KoColor &color );

    /**
     * Emitted when the user changes the current color selection.
     */
    void selectionChanged( KoDualColorButton::Selection selection );

    /**
     * Emitted when the user clicks one of the two color patches.
     * You should/could pop you own color chooser dialog in response.
     * Also see the popDialog attribute.
     */
    void pleasePopDialog( const KoColor &color );

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

