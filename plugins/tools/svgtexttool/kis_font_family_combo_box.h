/* This file is part of the KDE project
 *
   Copyright 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

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
#ifndef KIS_FONT_FAMILY_COMBO_BOX_H
#define KIS_FONT_FAMILY_COMBO_BOX_H

#include<QObject>
#include<KisSqueezedComboBox.h>
#include<QList>
#include<QFont>
#include<QFontDatabase>

/**
 * @brief The KisFontComboBoxes class
 * This is a little widget with two comboboxes.
 * One is for the font family, and the other for the style, using the power of QFontDataBase.
 * This allows us to limit the amount of fonts visible in the fonts drop down,
 * as that can be a quite intense number when you have several 'style complete' fonts.
 */
class KisFontComboBoxes : public QWidget
{
    Q_OBJECT
public:
    KisFontComboBoxes(QWidget *parent = 0);

    /**
     * @brief setCurrentFont
     * sets the style and font comboboxes appropriately.
     * @param font the QFont to set.
     */
    void setCurrentFont(QFont font);

    void setCurrentFamily(const QString family);
    void setCurrentStyle(QString style);

    //Current family name.
    QString currentFamily() const;
    //Current style
    QString currentStyle() const;
    /**
     * @brief currentFont the current QFont from both family and style combinations
     * @param pointSize as this widget has no idea about point size, input desired point size.
     * @return
     */
    QFont currentFont(int pointSize = 10) const;

    void refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems = QVector<QFontDatabase::WritingSystem>());
Q_SIGNALS:
    void fontChanged(QString);
private Q_SLOTS:
    void fontFamilyChanged();
    void fontChange();
private:
    QComboBox *m_family;
    QComboBox *m_styles;
};

/**
 * @brief The KisFontFamilyComboBox class
 * A QCombobox that limits the amount of fonts it contains.
 * Amongst others it blacklists certain fonts, it also filters
 * out 'style' fonts, like those used for Bold and Italic,
 * and it allows you to limit the amount of fonts to certain writing systems
 */
class KisFontFamilyComboBox : public QComboBox
{
    Q_OBJECT
public:
    KisFontFamilyComboBox(QWidget *parent = 0);

    //List of writing systems to use. If empty will default to "all"
    void refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems = QVector<QFontDatabase::WritingSystem>());

private:

    QStringList m_blacklistedFonts;

};

#endif // KIS_FONT_FAMILY_COMBO_BOX_H
