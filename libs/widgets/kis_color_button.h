/*
 *  SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *  This file is forked from the KF5 KColorButton
    SPDX-FileCopyrightText: 1997 Martin Jones (mjones@kde.org)

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KisColorButton_H
#define KisColorButton_H
#include <kritawidgets_export.h>

#include <KoColor.h>
#include <QPushButton>

class KisColorButtonPrivate;
/**
* @short A pushbutton to display or allow user selection of a color.
*
* This widget can be used to display or allow user selection of a color.
*
* @see QColorDialog
*
* \image html KisColorButton.png "KDE Color Button"
*/
class KRITAWIDGETS_EXPORT KisColorButton : public QPushButton
{
    Q_OBJECT

    /**
     * QtCreator treats KoColor as a QColor in incorrect way, so just disable using them in QtCreator
     * https://bugs.kde.org/show_bug.cgi?id=368483
     */
    Q_PROPERTY(KoColor color READ color WRITE setColor NOTIFY changed USER true DESIGNABLE false)
    Q_PROPERTY(KoColor defaultColor READ defaultColor WRITE setDefaultColor DESIGNABLE false)
    Q_PROPERTY(bool alphaChannelEnabled READ isAlphaChannelEnabled WRITE setAlphaChannelEnabled)

public:
    /**
     * Creates a color button.
     */
    explicit KisColorButton(QWidget *parent = 0);

    /**
     * Creates a color button with an initial color @p c.
     */
    explicit KisColorButton(const KoColor &c, QWidget *parent = 0);

    /**
     * Creates a color button with an initial color @p c and default color @p defaultColor.
     */
    KisColorButton(const KoColor &c, const KoColor &defaultColor, QWidget *parent = 0);

    ~KisColorButton() override;

    /**
     * Returns the currently chosen color.
     */
    KoColor color() const;

    /**
     * Sets the current color to @p c.
     */
    void setColor(const KoColor &c);

    /**
     * When set to true, allow the user to change the alpha component
     * of the color. The default value is false.
     */
    void setAlphaChannelEnabled(bool alpha);

    /**
     * Returns true if the user is allowed to change the alpha component.
     */
    bool isAlphaChannelEnabled() const;

    /**
     * Allow having a palette.
     */
    void setPaletteViewEnabled( bool enable);

    /**
     * @brief paletteViewEnabled
     * @return whether the palette is enabled.
     */
    bool paletteViewEnabled() const;
    /**
     * Returns the default color or an invalid color
     * if no default color is set.
     */
    KoColor defaultColor() const;

    /**
     * Sets the default color to @p c.
     */
    void setDefaultColor(const KoColor &c);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    /**
     * Emitted when the color of the widget
     * is changed, either with setColor() or via user selection.
     */
    void changed(const KoColor &newColor);

protected:
    void paintEvent(QPaintEvent *pe) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    class KisColorButtonPrivate;
    KisColorButtonPrivate *const d;

    Q_PRIVATE_SLOT(d, void _k_chooseColor())
};

#endif
