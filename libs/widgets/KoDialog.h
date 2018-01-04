/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      and Holger Freyther <freyther@kde.org>
 *            2005-2009 Olivier Goffart <ogoffart @ kde.org>
 *            2006      Tobias Koenig <tokoe@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KODIALOG_H
#define KODIALOG_H

class QPushButton;
class QMenu;
class KoDialogPrivate;

#include <kritawidgets_export.h>
#include <kconfiggroup.h>
#include <kguiitem.h>

#include <QDialog>

/**
 * @short A dialog base class with standard buttons and predefined layouts.
 *
 * Provides basic functionality needed by nearly all dialogs.
 *
 * It offers the standard action buttons you'd expect to find in a
 * dialog as well as the capability to define at most three configurable
 * buttons. You can define a main widget that contains your specific
 * dialog layout
 *
 * The class takes care of the geometry management. You only need to define
 * a minimum size for the widget you want to use as the main widget.
 *
 * By default, the dialog is non-modal.
 *
 * <b>Standard buttons (action buttons):</b>\n
 *
 * You select which buttons should be displayed, but you do not choose the
 * order in which they are displayed. This ensures a standard interface in
 * KDE. The button order can be changed, but this ability is only available
 * for a central KDE control tool. The following buttons are available:
 * OK, Cancel/Close, Apply/Try, Default, Help and three user definable
 * buttons: User1, User2 and User3. You must specify the text of the UserN
 * buttons. Each button emit a signal, so you can choose to connect that signal.
 *
 * The default action of the Help button will open the help system if you have
 * provided a path to the help text.
 * The default action of Ok and Cancel will run QDialog::accept() and QDialog::reject(),
 * which you can override by reimplementing slotButtonClicked(). The default
 * action of the Close button will close the dialog.
 *
 * Note that the KoDialog will animate a button press
 * when the user presses Escape. The button that is enabled is either Cancel,
 * Close or the button that is defined by setEscapeButton().
 * Your custom dialog code should reimplement the keyPressEvent and
 * animate the cancel button so that the dialog behaves like regular
 * dialogs.
 *
 * <b>Layout:</b>\n
 *
 * The dialog consists of a help area on top (becomes visible if you define
 * a help path and use enableLinkedHelp()), the main area which is
 * the built-in dialog face or your own widget in the middle and by default
 * a button box at the bottom. The button box can also be placed at the
 * right edge (to the right of the main widget). Use
 * setButtonsOrientation() to control this behavior. A separator
 * can be placed above the button box (or to the left when the button box
 * is at the right edge).
 *
 * <b>Standard compliance:</b>\n
 *
 * The marginHint() and spacingHint() sizes shall be used
 * whenever you lay out the interior of a dialog. One special note. If
 * you make your own action buttons (OK, Cancel etc), the space
 * between the buttons shall be spacingHint(), whereas the space
 * above, below, to the right and to the left shall be marginHint().
 * If you add a separator line above the buttons, there shall be a
 * marginHint() between the buttons and the separator and a
 * marginHint() above the separator as well.
 *
 * <b>Example:</b>\n
 *
 * \code
 *   KoDialog *dialog = new KoDialog( this );
 *   dialog->setCaption( "My title" );
 *   dialog->setButtons( KoDialog::Ok | KoDialog::Cancel | KoDialog::Apply );
 *
 *   FooWidget *widget = new FooWidget( dialog );
 *   dialog->setMainWidget( widget );
 *   connect( dialog, SIGNAL( applyClicked() ), widget, SLOT( save() ) );
 *   connect( dialog, SIGNAL( okClicked() ), widget, SLOT( save() ) );
 *   connect( widget, SIGNAL( changed( bool ) ), dialog, SLOT( enableButtonApply( bool ) ) );
 *
 *   dialog->enableButtonApply( false );
 *   dialog->show();
 * \endcode
 *
 * \image html kdialog.png "KDE Dialog example"
 *
 * This class can be used in many ways. Note that most KDE ui widgets
 * and many of KDE core applications use the KoDialog so for more
 * inspiration you should study the code for these.
 *
 *
 * @see KPageDialog
 * @author Thomas Tanghus <tanghus@earthling.net>
 * @author Espen Sand <espensa@online.no>
 * @author Mirko Boehm <mirko@kde.org>
 * @author Olivier Goffart <ogoffart at kde.org>
 * @author Tobias Koenig <tokoe@kde.org>
 */
class KRITAWIDGETS_EXPORT KoDialog : public QDialog //krazy:exclude=qclasses
{
    Q_OBJECT
    Q_ENUMS(ButtonCode)
    Q_DECLARE_PRIVATE(KoDialog)

public:

    enum ButtonCode {
        None    = 0x00000000,
        Help    = 0x00000001, ///< Show Help button. (this button will run the help set with setHelp)
        Default = 0x00000002, ///< Show Default button.
        Ok      = 0x00000004, ///< Show Ok button. (this button accept()s the dialog; result set to QDialog::Accepted)
        Apply   = 0x00000008, ///< Show Apply button.
        Try     = 0x00000010, ///< Show Try button.
        Cancel  = 0x00000020, ///< Show Cancel-button. (this button reject()s the dialog; result set to QDialog::Rejected)
        Close   = 0x00000040, ///< Show Close-button. (this button closes the dialog)
        No      = 0x00000080, ///< Show No button. (this button closes the dialog and sets the result to KoDialog::No)
        Yes     = 0x00000100, ///< Show Yes button. (this button closes the dialog and sets the result to KoDialog::Yes)
        Reset   = 0x00000200, ///< Show Reset button
        Details = 0x00000400, ///< Show Details button. (this button will show the detail widget set with setDetailsWidget)
        User1   = 0x00001000, ///< Show User defined button 1.
        User2   = 0x00002000, ///< Show User defined button 2.
        User3   = 0x00004000, ///< Show User defined button 3.
        NoDefault = 0x00008000 ///< Used when specifying a default button; indicates that no button should be marked by default.
    };
    // TODO KDE5: remove NoDefault and use the value None instead
    Q_DECLARE_FLAGS(ButtonCodes, ButtonCode)

    enum ButtonPopupMode {
        InstantPopup = 0,
        DelayedPopup = 1
    };
    Q_DECLARE_FLAGS(ButtonPopupModes, ButtonPopupMode)

public:
    /**
     * Creates a dialog.
     *
     * @param parent The parent of the dialog.
     * @param flags  The widget flags passed to the QDialog constructor
     */
    explicit KoDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    /**
     * Destroys the dialog.
     */
    ~KoDialog() override;

    /**
     * Creates (or recreates) the button box and all the buttons in it.
     *
     * Note that some combinations are not possible. That means, you can't
     * have the following pairs of buttons in a dialog:
     * - Default and Details
     * - Cancel and Close
     * - Ok and Try
     *
     * This will reset all default KGuiItem of all button.
     *
     * @param buttonMask Specifies what buttons will be made.
     *
     * @deprecated Since 5.0 use QDialogButtonBox
     */
    void setButtons(ButtonCodes buttonMask);

    /**
     * Sets the orientation of the button box.
     *
     * It can be @p Vertical or @p Horizontal. If @p Horizontal
     * (default), the button box is positioned at the bottom of the
     * dialog. If @p Vertical it will be placed at the right edge of the
     * dialog.
     *
     * @param orientation The button box orientation.
     */
    void setButtonsOrientation(Qt::Orientation orientation);

    /**
     * Sets the button that will be activated when the Escape key
     * is pressed.
     *
     * By default, the Escape key is mapped to either the Cancel or the Close button
     * if one of these buttons are defined. The user expects that Escape will
     * cancel an operation so use this function with caution.
     *
     * @param id The button code.
     */
    void setEscapeButton(ButtonCode id);

    /**
     * Sets the button that will be activated when the Enter key
     * is pressed.
     *
     * By default, this is the Ok button if it is present
     *
     * @param id The button code.
     */
    void setDefaultButton(ButtonCode id);

    /**
     * Returns the button code of the default button,
     * or NoDefault if there is no default button.
     */
    ButtonCode defaultButton() const;

    /**
     * Hide or display the a separator line drawn between the action
     * buttons an the main widget.
     */
    void showButtonSeparator(bool state);

    /**
     * Hide or display a general action button.
     *
     *  Only buttons that have
     * been created in the constructor can be displayed. This method will
     * not create a new button.
     *
     * @param id Button identifier.
     * @param state true display the button(s).
     */
    void showButton(ButtonCode id, bool state);

    /**
     * Sets the text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonText(ButtonCode id, const QString &text);

    /**
     * Returns the text of any button.
     */
    QString buttonText(ButtonCode id) const;

    /**
     * Sets the icon of any button.
     *
     * @param id The button identifier.
     * @param icon Button icon.
     */
    void setButtonIcon(ButtonCode id, const QIcon &icon);

    /**
     * Returns the icon of any button.
     */
    QIcon buttonIcon(ButtonCode id) const;

    /**
     * Sets the tooltip text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonToolTip(ButtonCode id, const QString &text);

    /**
     * Returns the tooltip of any button.
     */
    QString buttonToolTip(ButtonCode id) const;

    /**
     * Sets the "What's this?" text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonWhatsThis(ButtonCode id, const QString &text);

    /**
     * Returns the "What's this?" text of any button.
     */
    QString buttonWhatsThis(ButtonCode id) const;

    /**
     * Sets the KGuiItem directly for the button instead of using 3 methods to
     * set the text, tooltip and whatsthis strings. This also allows to set an
     * icon for the button which is otherwise not possible for the extra
     * buttons beside Ok, Cancel and Apply.
     *
     * @param id The button identifier.
     * @param item The KGuiItem for the button.
     */
    void setButtonGuiItem(ButtonCode id, const KGuiItem &item);

    /**
     * Sets the focus to the button of the passed @p id.
     */
    void setButtonFocus(ButtonCode id);

    /**
     * Convenience method. Sets the initial dialog size.
     *
     * This method should only be called right before show() or exec().
     * The initial size will be ignored if smaller than
     * the dialog's minimum size.
     *
     * @param size Startup size.
     */
    void setInitialSize(const QSize &size);

    /**
     * Convenience method. Add a size to the default minimum size of a
     * dialog.
     *
     * This method should only be called right before show() or exec().
     *
     * @param size  Size added to minimum size.
     */
    void incrementInitialSize(const QSize &size);

    /**
     * Returns the help link text.
     *
     *  If no text has been defined,
     * "Get help..." (internationalized) is returned.
     *
     * @return The help link text.
     *
     * @see enableLinkedHelp()
     * @see setHelpLinkText()
     * @see setHelp()
     */
    QString helpLinkText() const;

    /**
     * Returns whether any button is enabled.
     */
    bool isButtonEnabled(ButtonCode id) const;

    /**
     * Returns the button that corresponds to the @p id.
     *
     * Normally you should not use this function.
     * @em Never delete the object returned by this function.
     * See also enableButton(), showButton(), setButtonGuiItem().
     *
     * @param id Identifier of the button.
     * @return The button or 0 if the button does not exist.
     */
    QPushButton *button(ButtonCode id) const;

    /**
     * Returns the number of pixels that should be used between a
     * dialog edge and the outermost widget(s) according to the KDE standard.
     *
     * @deprecated Use the style's pixelMetric() function to query individual margins.
     * Different platforms may use different values for the four margins.
     */
    static int marginHint();

    /**
     * Returns the number of pixels that should be used between
     * widgets inside a dialog according to the KDE standard.
     *
     * @deprecated Use the style's layoutSpacing() function to query individual spacings.
     * Different platforms may use different values depending on widget types and pairs.
     */
    static int spacingHint();

    /**
     * Returns the number of pixels that should be used to visually
     * separate groups of related options in a dialog according to
     * the KDE standard.
     * @since 4.2
     */
    static int groupSpacingHint();

    /**
     * @enum StandardCaptionFlag
     * Used to specify how to construct a window caption
     *
     * @value AppName Indicates that the method shall include
     * the application name when making the caption string.
     * @value Modified Causes a 'modified' sign will be included in the
     * returned string. This is useful when indicating that a file is
     * modified, i.e., it contains data that has not been saved.
     * @value HIGCompliant The base minimum flags required to align a
     * caption with the KDE Human Interface Guidelines
     */
    enum CaptionFlag {
        NoCaptionFlags = 0,
        AppNameCaption = 1,
        ModifiedCaption = 2,
        HIGCompliantCaption = AppNameCaption
    };
    Q_DECLARE_FLAGS(CaptionFlags, CaptionFlag)

    /**
     * Builds a caption that contains the application name along with the
     * userCaption using a standard layout.
     *
     * To make a compliant caption for your window, simply do:
     * @p setWindowTitle(KoDialog::makeStandardCaption(yourCaption));
     *
     * To ensure that the caption is appropriate to the desktop in which the
     * application is running, pass in a pointer to the window the caption will
     * be applied to.
     *
     * If using a KoDialog or KMainWindow subclass, call setCaption instead and
     * an appropriate standard caption will be created for you
     *
     * @param userCaption The caption string you want to display in the
     * window caption area. Do not include the application name!
     * @param window a pointer to the window this application will apply to
     * @param flags
     * @return the created caption
     */
    static QString makeStandardCaption(const QString &userCaption,
                                       QWidget *window = 0,
                                       CaptionFlags flags = HIGCompliantCaption);

    /**
     * Resize every layout manager used in @p widget and its nested children.
     *
     * @param widget The widget used.
     * @param margin The new layout margin.
     * @param spacing The new layout spacing.
     *
     * @deprecated Use QLayout functions where necessary. Setting margin and spacing
     * values recursively for all children prevents QLayout from creating platform native
     * layouts.
     */
    static void resizeLayout(QWidget *widget, int margin, int spacing);

    /**
     * Resize every layout associated with @p lay and its children.
     *
     * @param lay layout to be resized
     * @param margin The new layout margin
     * @param spacing The new layout spacing
     *
     * @deprecated Use QLayout functions where necessary. Setting margin and spacing
     * values recursively for all children prevents QLayout from creating platform native
     * layouts.
     */
    static void resizeLayout(QLayout *lay, int margin, int spacing);

    /**
     * Centers @p widget on the desktop, taking multi-head setups into
     * account. If @p screen is -1, @p widget will be centered on its
     * current screen (if it was shown already) or on the primary screen.
     * If @p screen is -3, @p widget will be centered on the screen that
     * currently contains the mouse pointer.
     * @p screen will be ignored if a merged display (like Xinerama) is not
     * in use, or merged display placement is not enabled in kdeglobals.
     */
    static void centerOnScreen(QWidget *widget, int screen = -1);

    /**
     * Places @p widget so that it doesn't cover a certain @p area of the screen.
     * This is typically used by the "find dialog" so that the match it finds can
     * be read.
     * For @p screen, see centerOnScreen
     * @return true on success (widget doesn't cover area anymore, or never did),
     * false on failure (not enough space found)
     */
    static bool avoidArea(QWidget *widget, const QRect &area, int screen = -1);

    /**
     * Sets the main widget of the dialog.
     */
    void setMainWidget(QWidget *widget);

    /**
     * @return The current main widget. Will create a QWidget as the mainWidget
     * if none was set before. This way you can write
     * \code
     *   ui.setupUi(mainWidget());
     * \endcode
     * when using designer.
     */
    QWidget *mainWidget();

    /**
     * Reimplemented from QDialog.
     */
    QSize sizeHint() const override;

    /**
     * Reimplemented from QDialog.
     */
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    /**
     * Make a KDE compliant caption.
     *
     * @param caption Your caption. Do @p not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     *
     * @deprecated Since 5.0 use QWidget::setWindowTitle
     */
    virtual void setCaption(const QString &caption);

    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     * @param modified Specify whether the document is modified. This displays
     * an additional sign in the title bar, usually "**".
     *
     * @deprecated Since 5.0 use QWidget::setWindowTitle and QWidget::setWindowModified.
     */
    virtual void setCaption(const QString &caption, bool modified);

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption(const QString &caption);

    /**
     * Enable or disable (gray out) a general action button.
     *
     * @param id Button identifier.
     * @param state @p true enables the button(s).
     */
    void enableButton(ButtonCode id, bool state);

    /**
     * Enable or disable (gray out) the OK button.
     *
     * @param state @p true enables the button.
     */
    void enableButtonOk(bool state);

    /**
     * Enable or disable (gray out) the Apply button.
     *
     * @param state true enables the button.
     */
    void enableButtonApply(bool state);

    /**
     * Enable or disable (gray out) the Cancel button.
     *
     * @param state true enables the button.
     */
    void enableButtonCancel(bool state);

    /**
     * Display or hide the help link area on the top of the dialog.
     *
     * @param state @p true will display the area.
     *
     * @see helpLinkText()
     * @see setHelpLinkText()
     * @see setHelp()
     */
    void enableLinkedHelp(bool state);

    /**
     * Sets the text that is shown as the linked text.
     *
     * If text is empty,
     * the text "Get help..." (internationalized) is used instead.
     *
     * @param text The link text.
     *
     * @see helpLinkText()
     * @see enableLinkedHelp()
     * @see setHelp()
     */
    void setHelpLinkText(const QString &text);

    /**
     * Sets the help path and topic.
     *
     * @param anchor Defined anchor in your docbook sources
     * @param appname Defines the appname the help belongs to
     *                If empty it's the current one
     *
     * @note The help button works differently for the class
     * KCMultiDialog, so it does not make sense to call this
     * function for Dialogs of that type.  See
     * KCMultiDialog::slotHelp() for more information.
     */
    void setHelp(const QString &anchor, const QString &appname = QString());

    /**
     * Returns the status of the Details button.
     */
    bool isDetailsWidgetVisible() const;

    /**
     * Sets the status of the Details button.
     */
    void setDetailsWidgetVisible(bool visible);

    /**
     * Sets the widget that gets shown when "Details" is enabled.
     *
     * The dialog takes over ownership of the widget.
     * Any previously set widget gets deleted.
     */
    void setDetailsWidget(QWidget *detailsWidget);

    /**
     * Destruct the dialog delayed.
     *
     * You can call this function from slots like closeClicked() and hidden().
     * You should not use the dialog any more after calling this function.
     * @deprecated use hide()+deleteLater()
     */
    void delayedDestruct();

Q_SIGNALS:
    /**
     * Emitted when the margin size and/or spacing size
     * have changed.
     *
     * Use marginHint() and spacingHint() in your slot
     * to get the new values.
     *
     * @deprecated This signal is not emitted. Listen to QEvent::StyleChange events instead.
     */
    void layoutHintChanged();

    /**
     * The Help button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void helpClicked();

    /**
     * The Default button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void defaultClicked();

    /**
     * The Reset button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void resetClicked();

    /**
     * The User3 button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void user3Clicked();

    /**
     * The User2 button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void user2Clicked();

    /**
     * The User1 button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void user1Clicked();

    /**
     * The Apply button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void applyClicked();

    /**
     * The Try button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void tryClicked();

    /**
     * The OK button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void okClicked();

    /**
     * The Yes button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void yesClicked();

    /**
     * The No button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void noClicked();

    /**
     * The Cancel button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void cancelClicked();

    /**
     * The Close button was pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     */
    void closeClicked();

    /**
     * A button has been pressed. This signal is only emitted if
     * slotButtonClicked() is not replaced
     * @param button is the code of the pressed button.
     */
    void buttonClicked(KoDialog::ButtonCode button);

    /**
     * The dialog is about to be hidden.
     *
     * A dialog is hidden after a user clicks a button that ends
     * the dialog or when the user switches to another desktop or
     * minimizes the dialog.
     */
    void hidden();

    /**
     * The dialog has finished.
     *
     * A dialog emits finished after a user clicks a button that ends
     * the dialog.
     *
     * This signal is also emitted when you call hide()
     *
     * If you have stored a pointer to the
     * dialog do @em not try to delete the pointer in the slot that is
     * connected to this signal.
     *
     * You should use deleteLater() instead.
     */
    void finished();

    /**
     * The detailsWidget is about to get shown. This is your last chance
     * to call setDetailsWidget if you haven't done so yet.
     */
    void aboutToShowDetails();

protected:
    /**
     * Emits the #hidden signal. You can connect to that signal to
     * detect when a dialog has been closed.
     */
    void hideEvent(QHideEvent *) override;

    /**
     * Detects when a dialog is being closed from the window manager
     * controls. If the Cancel or Close button is present then the button
     * is activated. Otherwise standard QDialog behavior
     * will take place.
     */
    void closeEvent(QCloseEvent *e) override;

    /**
     * @internal
     */
    void keyPressEvent(QKeyEvent *) override;

protected Q_SLOTS:
    /**
     * Activated when the button @p button is clicked
     *
     * Sample that shows how to catch and handle button clicks within
     * an own dialog;
     * @code
     * class MyDialog : public KoDialog {
     *     protected Q_SLOTS:
     *         virtual void slotButtonClicked(int button) {
     *             if (button == KoDialog::Ok)
     *                 accept();
     *             else
     *                 KoDialog::slotButtonClicked(button);
     *         }
     * }
     * @endcode
     *
     * @param button is the type @a KoDialog::ButtonCode
     *
     * @deprecated since 5.0 use QDialogButtonBox and connect to the clicked signal
     */
    virtual void slotButtonClicked(int button);

    /**
     * Updates the margins and spacings.
     *
     * @deprecated KoDialog respects the style's margins and spacings automatically. Calling
     * this function has no effect.
     */
    void updateGeometry();

private:
    KoDialog(KoDialogPrivate &dd, QWidget *parent, Qt::WindowFlags flags = 0);
    KoDialogPrivate *const d_ptr;

private:
    Q_DISABLE_COPY(KoDialog)
    Q_PRIVATE_SLOT(d_ptr, void queuedLayoutUpdate())
    Q_PRIVATE_SLOT(d_ptr, void helpLinkClicked())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoDialog::ButtonCodes)
Q_DECLARE_OPERATORS_FOR_FLAGS(KoDialog::CaptionFlags)

#endif // KODIALOG_H
