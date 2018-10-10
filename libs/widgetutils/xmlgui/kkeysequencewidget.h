/* This file is part of the KDE libraries
    Copyright (C) 2001, 2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KKEYSEQUENCEWIDGET_H
#define KKEYSEQUENCEWIDGET_H

#include <kritawidgetutils_export.h>

#include <QList>
#include <QPushButton>

class KKeySequenceWidgetPrivate;
class QAction;
class KActionCollection;

/**
 * @short A widget to input a QKeySequence.
 *
 * This widget lets the user choose a QKeySequence, which is usually used as a
 * shortcut key. The recording is initiated by calling captureKeySequence() or
 * the user clicking into the widget.
 *
 * The widgets provides support for conflict handling. See
 * setCheckForConflictsAgainst() for more information.
 *
 * \image html kkeysequencewidget.png "KDE Key Sequence Widget"
 *
 * @author Mark Donohoe <donohoe@kde.org>
 * @internal
 */
class KRITAWIDGETUTILS_EXPORT KKeySequenceWidget: public QWidget
{
    Q_OBJECT

    Q_FLAGS(ShortcutTypes)

    Q_PROPERTY(
        bool multiKeyShortcutsAllowed
        READ multiKeyShortcutsAllowed
        WRITE setMultiKeyShortcutsAllowed)

    Q_PROPERTY(
        ShortcutTypes checkForConflictsAgainst
        READ checkForConflictsAgainst
        WRITE setCheckForConflictsAgainst)

    Q_PROPERTY(
        bool modifierlessAllowed
        READ isModifierlessAllowed
        WRITE setModifierlessAllowed)

public:
    ///An enum about validation when setting a key sequence.
    ///@see setKeySequence()
    enum Validation {
        ///Validate key sequence
        Validate = 0,
        ///Use key sequence without validation
        NoValidate = 1
    };

    /**
    * Constructor.
    */
    explicit KKeySequenceWidget(QWidget *parent = 0);

    /**
    * Destructs the widget.
    */
    ~KKeySequenceWidget() override;

    /**
     * \name Configuration
     *
     * Configuration options for the widget.
     */
    //@{

    enum ShortcutType {
        None           = 0x00,      //!< No checking for conflicts
        LocalShortcuts = 0x01,      //!< Check with local shortcuts. @see setCheckActionCollections()
        StandardShortcuts = 0x02,   //!< Check against standard shortcuts. @see KStandardShortcut
        GlobalShortcuts = 0x04      //!< Check against global shortcuts. @see KGlobalAccel
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

    /**
     * Configure if the widget should check for conflicts with existing
     * shortcuts.
     *
     * When capturing a key sequence for local shortcuts you should check
     * against GlobalShortcuts and your other local shortcuts. This is the
     * default.
     *
     * You have to provide the local actions to check against with
     * setCheckActionCollections().
     *
     * When capturing a key sequence for a global shortcut you should
     * check against StandardShortcuts, GlobalShortcuts and your local
     * shortcuts.
     *
     * There are two ways to react to a user agreeing to steal a shortcut:
     *
     * 1. Listen to the stealShortcut() signal and steal the shortcuts
     * manually. It's your responsibility to save that change later when
     * you think it is appropriate.
     *
     * 2. Call applyStealShortcut and KKeySequenceWidget will steal the
     * shortcut. This will save the actionCollections the shortcut is part
     * of so make sure it doesn't inadvertly save some unwanted changes
     * too. Read its documentation for some limitation when handling
     * global shortcuts.
    *
     * If you want to do the conflict checking yourself here are some code
     * snippets for global ...
     *
     * \code
     * QStringList conflicting = KGlobalAccel::findActionNameSystemwide(keySequence);
     * if (!conflicting.isEmpty()) {
     *     // Inform and ask the user about the conflict and reassigning
     *     // the keys sequence
     *     if (!KGlobalAccel::promptStealShortcutSystemwide(q, conflicting, keySequence)) {
     *         return true;
     *     }
     *     KGlobalAccel::stealShortcutSystemwide(keySequence);
     * }
     * \endcode
     *
     * ...  and standard shortcuts
     *
     * \code
     * KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
     * if (ssc != KStandardShortcut::AccelNone) {
     *     // We have a conflict
     * }
     * \endcode
     *
     *
     * @since 4.2
     */
    void setCheckForConflictsAgainst(ShortcutTypes types);

    /**
     * The shortcut types we check for conflicts.
     *
     * @see setCheckForConflictsAgainst()
     * @since 4.2
     */
    ShortcutTypes checkForConflictsAgainst() const;

    /**
     * Allow multikey shortcuts?
     */
    void setMultiKeyShortcutsAllowed(bool);
    bool multiKeyShortcutsAllowed() const;

    /**
     * This only applies to user input, not to setShortcut().
     * Set whether to accept "plain" keys without modifiers (like Ctrl, Alt, Meta).
     * Plain keys by our definition include letter and symbol keys and
     * text editing keys (Return, Space, Tab, Backspace, Delete).
     * "Special" keys like F1, Cursor keys, Insert, PageDown will always work.
     */
    void setModifierlessAllowed(bool allow);

    /**
     * @see setModifierlessAllowed()
     */
    bool isModifierlessAllowed();

    /**
     * Set whether a small button to set an empty key sequence should be displayed next to the
     * main input widget. The default is to show the clear button.
     */
    void setClearButtonShown(bool show);

    //@}

    /**
     * Checks whether the key sequence @a seq is available to grab.
     *
     * The sequence is checked under the same rules as if it has been typed by
     * the user. This method is useful if you get key sequences from another
     * input source and want to check if it is save to set them.
     *
     * @since 4.2
     */
    bool isKeySequenceAvailable(const QKeySequence &seq) const;

    /**
     * Return the currently selected key sequence.
     */
    QKeySequence keySequence() const;

    /**
     * Set a list of action collections to check against for conflictuous shortcut.
     *
     * @see setCheckForConflictsAgainst()
     *
     * If a KAction with a conflicting shortcut is found inside this list and
     * its shortcut can be configured (KAction::isShortcutConfigurable()
     * returns true) the user will be prompted whether to steal the shortcut
     * from this action.
     *
     * @since 4.1
     */
    void setCheckActionCollections(const QList<KActionCollection *> &actionCollections);

    /**
     * If the component using this widget supports shortcuts contexts, it has
     * to set its component name so we can check conflicts correctly.
     */
    void setComponentName(const QString &componentName);

Q_SIGNALS:

    /**
     * This signal is emitted when the current key sequence has changed, be it by user
     * input or programmatically.
     */
    void keySequenceChanged(const QKeySequence &seq);

    /**
     * This signal is emitted after the user agreed to steal a shortcut from
     * an action. This is only done for local shortcuts. So you can be sure \a
     * action is one of the actions you provided with setCheckActionList() or
     * setCheckActionCollections().
     *
     * If you listen to that signal and don't call applyStealShortcut() you
     * are supposed to steal the shortcut and save this change.
     */
    void stealShortcut(const QKeySequence &seq, QAction *action);

public Q_SLOTS:

    /**
     * Capture a shortcut from the keyboard. This call will only return once a key sequence
     * has been captured or input was aborted.
     * If a key sequence was input, keySequenceChanged() will be emitted.
     *
     * @see setModifierlessAllowed()
     */
    void captureKeySequence();

    /**
     * Set the key sequence.
     *
     * If @p val == Validate, and the call is actually changing the key sequence,
     * conflictuous shortcut will be checked.
     */
    void setKeySequence(const QKeySequence &seq, Validation val = NoValidate);

    /**
     * Clear the key sequence.
     */
    void clearKeySequence();

    /**
     * Actually remove the shortcut that the user wanted to steal, from the
     * action that was using it. This only applies to actions provided to us
     * by setCheckActionCollections() and setCheckActionList().
     *
     * Global and Standard Shortcuts have to be stolen immediately when the
     * user gives his consent (technical reasons). That means those changes
     * will be active even if you never call applyStealShortcut().
     *
     * To be called before you apply your changes. No local shortcuts are
     * stolen until this function is called.
     */
    void applyStealShortcut();

private:
    Q_PRIVATE_SLOT(d, void doneRecording())

private:
    friend class KKeySequenceWidgetPrivate;
    KKeySequenceWidgetPrivate *const d;

    Q_DISABLE_COPY(KKeySequenceWidget)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KKeySequenceWidget::ShortcutTypes)

#endif //KKEYSEQUENCEWIDGET_H
