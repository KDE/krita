/*
 * klangbutton.h - Button with language selection drop down menu.
 *                 Derived from the KLangCombo class by Hans Petter Bieker.
 *
 * SPDX-FileCopyrightText: 1999-2003 Hans Petter Bieker <bieker@kde.org>
 * SPDX-FileCopyrightText: 2001 Martijn Klingens <klingens@kde.org>
 * SPDX-FileCopyrightText: 2007 David Jarvie <software@astrojar.org.uk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KLANGUAGEBUTTON_H
#define KLANGUAGEBUTTON_H

#include "kritawidgetutils_export.h"
#include <QWidget>

class QAction;
class KLanguageButtonPrivate;

/**
 * KLanguageButton is a pushbutton which allows a language to be selected from
 * a popup list.
 *
 * Languages are identified by their ISO 639-1 codes, e.g. en, pt_BR.
 *
 * \image html klanguagebutton.png "KDE Language Selection Widget"
 *
 * @author Hans Petter Bieker <bieker@kde.org>, Martijn Klingens <klingens@kde.org>,
 *         David Jarvie <software@astrojar.org.uk>
 */
class KRITAWIDGETUTILS_EXPORT KLanguageButton : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a button whose text is determined by the current language
     * in the popup list.
     *
     * @param parent the parent of the button
     */
    explicit KLanguageButton(QWidget *parent = 0);

    /**
     * Constructs a button with static text.
     *
     * @param text the text of the button
     * @param parent the parent of the button
     */
    explicit KLanguageButton(const QString &text, QWidget *parent = 0);

    /**
     * Deconstructor
     */
    ~KLanguageButton() override;

    /**
     * Sets the locale to display language names. By default, QLocale::system().name() is used.
     *
     * @param locale locale to use
     */
    void setLocale(const QString &locale);

    /**
     * Sets a static button text.
     *
     * @param text button text
     */
    void setText(const QString &text);

    /**
     * Specifies whether language codes should be shown alongside language names
     * in the popup. Calling this method does not affect any previously
     * inserted language texts, so it should normally be called before
     * populating the list.
     *
     * @param show true to show codes, false to hide codes
     */
    void showLanguageCodes(bool show);

    /**
     * Load all known languages into the popup list.
     * The current language in the list is set to the default language for the
     * current locale (as modified by setLocale()).
     */
    void loadAllLanguages();

    /**
     * Inserts a language into the combo box.
     * Normally the display name of the language is obtained automatically, but
     * if either the language code does not exist, or there are special display
     * requirements, the name of the language can be specified in @p name.
     *
     * @param languageCode the code for the language
     * @param name language name. If empty, the name is obtained automatically.
     * @param index the insertion position, or -1 to insert in alphabetical order
     */
    void insertLanguage(const QString &languageCode, const QString &name = QString(), int index = -1);

    /**
     * Inserts a separator item into the combo box. A negative index will append the item.
     *
     * @param index the insertion position
     */
    void insertSeparator(int index = -1);

    /**
     * Returns the number of items in the combo box.
     */
    int count() const;

    /**
     * Removes all combobox items.
     */
    void clear();

    /**
     * Returns the language code of the combobox's current item.
     *
     * @return the current item's language code
     */
    QString current() const;

    /**
     * Checks whether the specified language is in the popup list.
     *
     * @param languageCode the language's code
     * @return true if in the list
     */
    bool contains(const QString &languageCode) const;

    /**
     * Sets a given language to be the current item.
     *
     * @param languageCode the language's code
     */
    void setCurrentItem(const QString &languageCode);

Q_SIGNALS:
    /**
     * This signal is emitted when a new item is activated.
     *
     * @param languageCode code of the activated language
     */
    void activated(const QString &languageCode);
    /**
     * This signal is emitted when a new item is highlighted.
     *
     * @param languageCode code of the highlighted language
     */
    void highlighted(const QString &languageCode);

private Q_SLOTS:
    void slotTriggered(QAction *);
    void slotHovered(QAction *);

private:
    KLanguageButtonPrivate *const d;
};

#endif
