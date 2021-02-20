/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_FONT_FAMILY_COMBO_BOX_H
#define KIS_FONT_FAMILY_COMBO_BOX_H

#include <QObject>
#include <KisSqueezedComboBox.h>
#include <QList>
#include <QFont>
#include <QFontDatabase>
#include <QStyledItemDelegate>

#include "kritawidgetutils_export.h"

/**
 * @brief The KisFontComboBoxes class
 * This is a little widget with two comboboxes.
 * One is for the font family, and the other for the style, using the power of QFontDataBase.
 * This allows us to limit the amount of fonts visible in the fonts drop down,
 * as that can be a quite intense number when you have several 'style complete' fonts.
 */
class KRITAWIDGETUTILS_EXPORT KisFontComboBoxes : public QWidget
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

    // Current family name.
    QString currentFamily() const;
    // Current style
    QString currentStyle() const;

    /**
     * @brief currentFont the current QFont from both family and style combinations
     * @param pointSize as this widget has no idea about point size, input desired point size.
     * @return
     */
    QFont currentFont(int pointSize = 10) const;

    void refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems = QVector<QFontDatabase::WritingSystem>());
    void setInitialized();

Q_SIGNALS:
    void fontChanged(QString);
private Q_SLOTS:
    void fontFamilyChanged();
    void fontChange();
private:
    QComboBox *m_family;
    QComboBox *m_styles;
};

class KRITAWIDGETUTILS_EXPORT PinnedFontsSeparator : public QStyledItemDelegate {
public:
    PinnedFontsSeparator(QAbstractItemDelegate *_default, QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setSeparatorIndex(int index);
    void setSeparatorAdded();
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_separatorIndex;
    bool m_separatorAdded;
    QAbstractItemDelegate *m_defaultDelegate;
};

/**
 * @brief The KisFontFamilyComboBox class
 * A QCombobox that limits the amount of fonts it contains.
 * Amongst others it blacklists certain fonts, it also filters
 * out 'style' fonts, like those used for Bold and Italic,
 * and it allows you to limit the amount of fonts to certain writing systems
 */
class KRITAWIDGETUTILS_EXPORT KisFontFamilyComboBox : public QComboBox
{
    Q_OBJECT
public:
    KisFontFamilyComboBox(QWidget *parent = 0);

    // List of writing systems to use. If empty will default to "all"
    void refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems = QVector<QFontDatabase::WritingSystem>());
    void setTopFont(const QString &family);
    void setInitialized();

private:
    QStringList m_pinnedFonts;
    QStringList m_blacklistedFonts;
    bool m_initilized {false};
    bool m_initializeFromConfig;
    int m_separatorIndex;
    PinnedFontsSeparator *m_fontSeparator;
};

#endif // KIS_FONT_FAMILY_COMBO_BOX_H
