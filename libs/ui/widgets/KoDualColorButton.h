/* This file is part of the KDE libraries

   SPDX-FileCopyrightText: 1999 Daniel M. Duley <mosfet@kde.org>
   SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KODUALCOLORBUTTON_H
#define KODUALCOLORBUTTON_H

#include <kritaui_export.h>
#include <KoColorDisplayRendererInterface.h>
#include <QWidget>

class KoColor;
class KoColorSpace;
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
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KRITAUI_EXPORT KoDualColorButton : public QWidget
{
    Q_OBJECT
    Q_ENUMS( Selection )
    Q_PROPERTY( KoColor foregroundColor READ foregroundColor WRITE setForegroundColor )
    Q_PROPERTY( KoColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
    Q_PROPERTY( bool popDialog READ popDialog WRITE setPopDialog )


  public:
    enum Selection {
      Foreground,
      Background
    };

    /**
     * Constructs a new KoDualColorButton with the supplied foreground and
     * background colors.
     *
     * @param parent The parent widget of the KoDualColorButton.
     * @param dialogParent The parent widget of the color selection dialog.
     */
    KoDualColorButton(const KoColor &foregroundColor, const KoColor &backgroundColor,
                      QWidget *parent = 0, QWidget* dialogParent = 0 );

    KoDualColorButton(const KoColor &foregroundColor, const KoColor &backgroundColor,
                      const KoColorDisplayRendererInterface *displayRenderer,
                      QWidget *parent = 0, QWidget* dialogParent = 0 );

    /**
     * Destroys the KoDualColorButton.
     */
    ~KoDualColorButton() override;

    /**
     * Returns the current foreground color.
     */
    KoColor foregroundColor() const;

    /**
     * Returns the current background color.
     */
    KoColor backgroundColor() const;

    /**
     * Returns if a dialog with a color chooser will be popped up when clicking
     * If false then you could/should connect to the pleasePopDialog signal
     * and pop your own dialog. Just set the current color afterwards.
     */
    bool popDialog() const;

    /**
     * Returns the minimum size needed to display the widget and all its
     * controls.
     */
    QSize sizeHint() const override;

  public Q_SLOTS:
    /**
     * Sets the foreground color.
     */
    void setForegroundColor( const KoColor &color );

    /**
     * Sets the background color.
     */
    void setBackgroundColor( const KoColor &color );

    void slotSetForeGroundColorFromDialog (const KoColor color);

    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());

    /**
     * @brief setColorSpace
     * set ColorSpace so we can lock the selector. Right now this'll be changed per view-change.
     * @param cs
     */
    void setColorSpace(const KoColorSpace *cs);

    /**
     * @brief getColorFromDisplayRenderer
     * convenience function to get the right qcolor from the display renderer, including checking
     * whether the display renderer actually exists.
     * @param c the kocolor to convert.
     * @return the qcolor to use for display.
     */
    QColor getColorFromDisplayRenderer(KoColor c);

    /**
     * Sets if a dialog with a color chooser should be popped up when clicking
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

  protected:
    /**
     * Sets the supplied rectangles to the proper size and position for the
     * current widget size. You can reimplement this to change the layout
     * of the widget. Restrictions are that the swap control will always
     * be at the top right, the reset control will always be at the bottom
     * left, and you must leave at least a 14x14 space in those corners.
     */
    virtual void metrics( QRect &foregroundRect, QRect &backgroundRect );

    void paintEvent( QPaintEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    void changeEvent(QEvent *event) override;

  private:
    class Private;
    Private *const d;
};

#endif

