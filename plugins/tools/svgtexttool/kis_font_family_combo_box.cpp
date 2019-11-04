/* This file is part of the KDE project
 *
 * Copyright 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_font_family_combo_box.h"
#include <QFontDatabase>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QComboBox>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QCompleter>
#include <klocalizedstring.h>
#include <kis_debug.h>
#include <QPainter>
#include <kis_config.h>

PinnedFontsSeparator::PinnedFontsSeparator(QAbstractItemDelegate *_default, QWidget *parent) :
    QStyledItemDelegate(parent), m_separatorIndex(0), m_separatorAdded(false), m_defaultDelegate(_default)
{
}

void PinnedFontsSeparator::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() == m_separatorIndex && m_separatorAdded) {
        QRect viewRect = option.rect;
        painter->setPen(Qt::gray);
        painter->drawLine((viewRect.topLeft() + viewRect.bottomLeft()) / 2 + QPoint(5, 0),
                          (viewRect.topRight() + viewRect.bottomRight()) / 2 - QPoint(5, 0));
    } else {
        m_defaultDelegate->paint(painter, option, index);
    }
}

QSize PinnedFontsSeparator::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index) * 1.25;
}

void PinnedFontsSeparator::setSeparatorIndex(int index)
{
    m_separatorIndex = index;
}

void PinnedFontsSeparator::setSeparatorAdded()
{
    m_separatorAdded = true;
}

KisFontFamilyComboBox::KisFontFamilyComboBox(QWidget *parent)
    : QComboBox(parent), m_initilized(false), m_initializeFromConfig(false)
{
    setEditable(true);
    completer()->setCompletionMode(QCompleter::InlineCompletion);
    completer()->setCaseSensitivity(Qt::CaseInsensitive);
    // The following are all helper fonts for LaTeX that no one but LaTeX would use
    // but because many people use LaTeX, they do show up on quite a few systems.
    m_blacklistedFonts << "bbold10" << "cmbsy10" << "cmmib10"
                       << "cmss10" << "cmex10" << "cmmi10"
                       << "cmr10" << "cmsy10" << "eufb10"
                       << "eufm10" << "eurb10" << "eurm10"
                       << "esint10" << "eufm10" << "eusb10"
                       << "eusm10" << "lasy10" << "lasyb10"
                       << "msam10" << "msbm10" << "rsfs10"
                       << "stmary10" << "wasy10" << "wasyb10";
    refillComboBox();
    QFontComboBox *temp = new QFontComboBox();
    m_fontSeparator = new PinnedFontsSeparator(temp->itemDelegate(), this);
    temp->setEnabled(true);
    temp->hide();
    m_separatorIndex = 0;
    m_pinnedFonts = KisConfig(true).readList("PinnedFonts", QStringList{});
}

void KisFontFamilyComboBox::refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems)
{
    QFontDatabase fonts = QFontDatabase();
    int maxWidth        = 0;
    this->clear();

    QStringList duplicateFonts;
    QStringList filteredFonts;

    if (writingSystems.isEmpty()) {
        writingSystems.append(QFontDatabase::Any);
    }

    for (int i = 0; i < writingSystems.size(); i++) {
        Q_FOREACH (QString family, fonts.families(writingSystems.at(i))) {
            // if it's a private family it shouldn't be added.
            bool addFont = !fonts.isPrivateFamily(family);

            if (addFont && filteredFonts.contains(family)) {
                addFont = false;
            }
            if (addFont && duplicateFonts.contains(family)) {
                addFont = false;
            }
            if (addFont && m_blacklistedFonts.contains(family)) {
                addFont = false;
            }
            if (addFont && !fonts.isSmoothlyScalable(family)) {
                addFont = false;
            }

            if (addFont) {
                // now, check for all possible familyname+style name combinations, so we can avoid those.
                Q_FOREACH (const QString style, fonts.styles(family)) {
                    duplicateFonts.append(family + " " + style);
                    duplicateFonts.append(family + "_" + style);
                }
                filteredFonts.append(family);
                int width = 1.5 * view()->fontMetrics()
                        .width(family + " " + fonts.writingSystemSample(QFontDatabase::Any));
                if (width > maxWidth) {
                    maxWidth = width;
                }
            }
        }
    }
    this->addItems(filteredFonts);
    if (this->count() > this->maxVisibleItems()) {
        maxWidth += view()->style()->pixelMetric(QStyle::PixelMetric::PM_ScrollBarExtent);
    }
    view()->setMinimumWidth(maxWidth);
} // KisFontFamilyComboBox::refillComboBox

void KisFontFamilyComboBox::setTopFont(const QString &family)
{
    if (family.isEmpty() || !m_initilized || m_pinnedFonts.contains(family)) {
        return;
    }

    if (m_pinnedFonts.count() > 4) {
        this->removeItem(4);
        m_pinnedFonts.pop_back();
        m_separatorIndex--;
    }

    if (m_pinnedFonts.isEmpty()) {
        this->insertSeparator(0);
        m_fontSeparator->setSeparatorAdded();
    }

    m_pinnedFonts.push_front(family);
    this->insertItem(0, family);
    m_separatorIndex++;
    m_fontSeparator->setSeparatorIndex(m_separatorIndex);
    KisConfig(false).writeList("PinnedFonts", m_pinnedFonts);
}

void KisFontFamilyComboBox::setInitialized()
{
    m_initilized = true;

    for(int i=m_pinnedFonts.count()-1; i>=0; i--){
        this->insertItem(0, m_pinnedFonts[i]);
        m_separatorIndex++;
    }

    if(m_pinnedFonts.count() > 0){
        this->insertSeparator(m_separatorIndex);
        m_fontSeparator->setSeparatorIndex(m_separatorIndex);
        m_fontSeparator->setSeparatorAdded();
    }

    this->setItemDelegate(m_fontSeparator);
}

KisFontComboBoxes::KisFontComboBoxes(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    this->setLayout(layout);

    m_family = new KisFontFamilyComboBox();
    m_family->setMinimumWidth(100);
    m_family->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    layout->addWidget(m_family);

    m_styles = new QComboBox();
    layout->addWidget(m_styles);
    fontFamilyChanged();
    m_family->setToolTip(i18n("Font Family"));
    m_styles->setToolTip(i18n("Font Style"));
    connect(m_family, SIGNAL(currentTextChanged(QString)), this, SLOT(fontFamilyChanged()));
    connect(m_family, SIGNAL(currentTextChanged(QString)), this, SLOT(fontChange()));
    connect(m_styles, SIGNAL(activated(int)), this, SLOT(fontChange()));
}

void KisFontComboBoxes::setCurrentFont(QFont font)
{
    setCurrentFamily(font.family());
    setCurrentStyle(QFontDatabase().styleString(font));
}

void KisFontComboBoxes::setCurrentFamily(const QString family)
{
    m_family->setCurrentText(family);
    fontFamilyChanged();
}

void KisFontComboBoxes::setCurrentStyle(QString style)
{
    QString properStyle = QString();
    for (int i = 0; i < m_styles->count(); i++) {
        QString item = m_styles->itemText(i);
        if (item == style) {
            properStyle = style;
        } else if (item.contains(style, Qt::CaseInsensitive)) {
            properStyle = item;
        } else if (item.contains("regular", Qt::CaseInsensitive)) {
            properStyle = item;
        }
    }
    m_styles->setCurrentText(properStyle);
}

QString KisFontComboBoxes::currentFamily() const
{
    return m_family->currentText();
}

QString KisFontComboBoxes::currentStyle() const
{
    return m_styles->currentText();
}

QFont KisFontComboBoxes::currentFont(int pointSize) const
{
    return QFontDatabase().font(m_family->currentText(), m_styles->currentText(), pointSize);
}

void KisFontComboBoxes::refillComboBox(QVector<QFontDatabase::WritingSystem> writingSystems)
{
    KisFontFamilyComboBox *cmb = qobject_cast<KisFontFamilyComboBox *>(m_family);
    cmb->refillComboBox(writingSystems);
}

void KisFontComboBoxes::fontFamilyChanged()
{
    QString currentText = m_styles->currentText();
    QFontDatabase fonts;
    const QString family = m_family->currentText();
    int maxWidth         = 0;
    m_styles->clear();
    QStringList styles;

    KisFontFamilyComboBox *cmb = qobject_cast<KisFontFamilyComboBox *>(m_family);
    cmb->setTopFont(family);

    if (fonts.styles(family).isEmpty()) {
        styles.append("Normal");
    }

    Q_FOREACH (const QString style, fonts.styles(family)) {
        int b      = fonts.weight(family, style);
        int bindex = 0;
        for (int i = 0; i < styles.size(); i++) {
            if (b > fonts.weight(family, styles.at(i))) {
                bindex = i;
            }
        }
        if (!styles.contains(style)) {
            styles.insert(bindex, style);
            maxWidth = qMax(m_styles->view()->fontMetrics().width(style + "  "), maxWidth);
        }
    }
    m_styles->addItems(styles);
    if (m_styles->count() > m_styles->maxVisibleItems()) {
        maxWidth += m_styles->view()->style()->pixelMetric(QStyle::PixelMetric::PM_ScrollBarExtent);
    }
    m_styles->view()->setMinimumWidth(maxWidth);
    if (styles.contains(currentText)) {
        m_styles->setCurrentText(currentText);
    }
} // KisFontComboBoxes::fontFamilyChanged

void KisFontComboBoxes::fontChange()
{
    emit fontChanged(currentFont(10).toString());
}

void KisFontComboBoxes::setInitialized()
{
    KisFontFamilyComboBox *cmb = qobject_cast<KisFontFamilyComboBox *>(m_family);
    cmb->setInitialized();
}
