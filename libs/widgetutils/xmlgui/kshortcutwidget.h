/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSHORTCUTWIDGET_H
#define KSHORTCUTWIDGET_H

#include <kritawidgetutils_export.h>

#include <QKeySequence>
#include <QList>
#include <QWidget>

class KActionCollection;
class KShortcutWidgetPrivate;

/**
 * \image html kshortcutwidget.png "KDE Shortcut Widget"
 */
class KRITAWIDGETUTILS_EXPORT KShortcutWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool modifierlessAllowed READ isModifierlessAllowed WRITE setModifierlessAllowed)
public:
    KShortcutWidget(QWidget *parent = 0);
    ~KShortcutWidget() override;

    void setModifierlessAllowed(bool allow);
    bool isModifierlessAllowed();

    void setClearButtonsShown(bool show);

    QList<QKeySequence> shortcut() const;

    /**
     * Set a list of action collections to check against for conflictuous shortcut.
     *
     * If there is a conflictuous shortcut with a KAction, and that his shortcut can be configured
     * (KAction::isShortcutConfigurable() returns true) the user will be prompted for eventually steal
     * the shortcut from this action
     *
     * Global shortcuts are automatically checked for conflicts
     *
     * Don't forget to call applyStealShortcut to actually steal the shortcut.
     *
     * @since 4.1
     */
    void setCheckActionCollections(const QList<KActionCollection *> &actionCollections);

Q_SIGNALS:
    void shortcutChanged(const QList<QKeySequence> &cut);

public Q_SLOTS:
    void setShortcut(const QList<QKeySequence> &cut);
    void clearShortcut();

    /**
     * Actually remove the shortcut that the user wanted to steal, from the
     * action that was using it.
     *
     * To be called before you apply your changes.
     * No shortcuts are stolen until this function is called.
     */
    void applyStealShortcut();

private:
    Q_PRIVATE_SLOT(d, void priKeySequenceChanged(const QKeySequence &))
    Q_PRIVATE_SLOT(d, void altKeySequenceChanged(const QKeySequence &))

private:
    friend class KShortcutWidgetPrivate;
    KShortcutWidgetPrivate *const d;
};

#endif //KSHORTCUTWIDGET_H
