/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_filter_combo.h"

#include <klocalizedstring.h>
#include <QStylePainter>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QProxyStyle>
#include <QStyleOption>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <QMouseEvent>




#include "kis_node_view_color_scheme.h"
#include "kis_debug.h"
#include "kis_icon_utils.h"
#include "krita_utils.h"
#include "kis_node.h"

enum AdditionalRoles {
    OriginalLabelIndex = Qt::UserRole + 1000
};


struct LabelFilteringModel : public QSortFilterProxyModel
{
    LabelFilteringModel(QObject *parent) : QSortFilterProxyModel(parent) {}

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        const int labelIndex = index.data(OriginalLabelIndex).toInt();


        return labelIndex < 0 || m_acceptedLabels.contains(labelIndex);
    }

    void setAcceptedLabels(const QSet<int> &value) {
        m_acceptedLabels = value;
        invalidateFilter();
    }

private:
    QSet<int> m_acceptedLabels;
};


class ComboEventFilter : public QObject
{
public:
    ComboEventFilter(KisColorFilterCombo *parent) : m_parent(parent), m_buttonPressed(false) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Leave) {
            m_buttonPressed = false;

        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            m_buttonPressed = mevent->button() == Qt::LeftButton;

        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            QModelIndex index = m_parent->view()->indexAt(mevent->pos());
            if (!index.isValid()) return false;

            /**
             * We should eat the first event that arrives exactly when
             * the drop down appears on screen.
             */
            if (!m_buttonPressed) return true;

            const bool toUncheckedState = index.data(Qt::CheckStateRole) == Qt::Checked;

            if (toUncheckedState) {
                m_parent->model()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
            } else {
                m_parent->model()->setData(index, Qt::Checked, Qt::CheckStateRole);
            }

            if (index.data(OriginalLabelIndex).toInt() == -1) {
                for (int i = 0; i < m_parent->model()->rowCount(); i++) {
                    const QModelIndex &other = m_parent->model()->index(i, 0);
                    if (other.data(OriginalLabelIndex) != -1) {
                        m_parent->model()->setData(other, toUncheckedState ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
                    }
                }
            } else {
                bool prevChecked = false;
                bool checkedVaries = false;
                QModelIndex allLabelsIndex;

                for (int i = 0; i < m_parent->model()->rowCount(); i++) {
                    const QModelIndex &other = m_parent->model()->index(i, 0);
                    if (other.data(OriginalLabelIndex) != -1) {
                        const bool currentChecked = other.data(Qt::CheckStateRole) == Qt::Checked;

                        if (i == 0) {
                            prevChecked = currentChecked;
                        } else {
                            if (prevChecked != currentChecked) {
                                checkedVaries = true;
                                break;
                            }
                        }
                    } else {
                        allLabelsIndex = other;
                    }
                }

                const bool allLabelsIndexShouldBeChecked =
                    prevChecked && !checkedVaries;

                if (allLabelsIndexShouldBeChecked !=
                    (allLabelsIndex.data(Qt::CheckStateRole) == Qt::Checked)) {

                    m_parent->model()->setData(allLabelsIndex, allLabelsIndexShouldBeChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
                }
            }

            emit m_parent->selectedColorsChanged();

            m_buttonPressed = false;
            return true;
        }

        return QObject::eventFilter(obj, event);
    }

private:
    KisColorFilterCombo *m_parent;
    bool m_buttonPressed;
};

class FullSizedListView : public QListView
{
public:
    QSize sizeHint() const override {
        return contentsSize();
    }
};

class PopupComboBoxStyle : public QProxyStyle
{
public:
    PopupComboBoxStyle(QStyle *baseStyle = nullptr) : QProxyStyle(baseStyle) {}

    int styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override
    {
        // This flag makes ComboBox popup float ontop of its parent ComboBox, like in Fusion style.
        // Only when this hint is set will Qt respect combobox popup size hints, otherwise the popup
        // can never exceed the width of its parent ComboBox, like in Breeze style.
        if (hint == QStyle::SH_ComboBox_Popup) {
            return true;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

struct KisColorFilterCombo::Private
{
    LabelFilteringModel *filteringModel;
};

KisColorFilterCombo::KisColorFilterCombo(QWidget *parent)
    : QComboBox(parent),
      m_d(new Private)
{
    QStandardItemModel *newModel = new QStandardItemModel(this);
    setModel(newModel);

    PopupComboBoxStyle *proxyStyle = new PopupComboBoxStyle(style());
    proxyStyle->setParent(this);
    setStyle(proxyStyle);

    setView(new FullSizedListView);
    m_eventFilters.append(new ComboEventFilter(this));
    m_eventFilters.append(new ComboEventFilter(this));

    view()->installEventFilter(m_eventFilters[0]);
    view()->viewport()->installEventFilter(m_eventFilters[1]);

    KisNodeViewColorScheme scm;

    QStandardItem* item = new QStandardItem(i18nc("combo box: show all layers", "All"));
    item->setCheckable(true);
    item->setCheckState(Qt::Unchecked);
    item->setData(QColor(Qt::transparent), Qt::BackgroundColorRole);
    item->setData(int(-1), OriginalLabelIndex);
    item->setData(QSize(30, scm.rowHeight()), Qt::SizeHintRole);
    newModel->appendRow(item);

    int labelIndex = 0;
    foreach (const QColor &color, scm.allColorLabels()) {
        const QString title = color.alpha() > 0 ? "" : i18nc("combo box: select all layers without a label", "No Label");

        QStandardItem* item = new QStandardItem(title);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setData(color, Qt::BackgroundColorRole);
        item->setData(labelIndex, OriginalLabelIndex);
        item->setData(QSize(30, scm.rowHeight()), Qt::SizeHintRole);
        newModel->appendRow(item);

        labelIndex++;
    }

    m_d->filteringModel = new LabelFilteringModel(this);
    QAbstractItemModel *originalModel = model();
    originalModel->setParent(m_d->filteringModel);

    m_d->filteringModel->setSourceModel(originalModel);
    setModel(m_d->filteringModel);
}

KisColorFilterCombo::~KisColorFilterCombo()
{
    qDeleteAll(m_eventFilters);
}

void collectAvailableLabels(KisNodeSP root, QSet<int> *labels)
{
    labels->insert(root->colorLabelIndex());

    KisNodeSP node = root->firstChild();
    while (node) {
        collectAvailableLabels(node, labels);
        node = node->nextSibling();
    }
}

void KisColorFilterCombo::updateAvailableLabels(KisNodeSP rootNode)
{
    QSet<int> labels;
    collectAvailableLabels(rootNode, &labels);

    updateAvailableLabels(labels);
}

void KisColorFilterCombo::updateAvailableLabels(const QSet<int> &labels)
{
    m_d->filteringModel->setAcceptedLabels(labels);
}

QList<int> KisColorFilterCombo::selectedColors() const
{
    QList<int> colors;
    for (int i = 0; i < model()->rowCount(); i++) {
        const QModelIndex &other = model()->index(i, 0);
        const int label = other.data(OriginalLabelIndex).toInt();

        if (label != -1 &&
            other.data(Qt::CheckStateRole) == Qt::Checked) {

            colors << label;
        }
    }
    return colors;
}

void KisColorFilterCombo::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    {
        KisNodeViewColorScheme scm;
        const QRect editRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
        const int size = qMin(editRect.width(), editRect.height());

        const QList<int> selectedColors = this->selectedColors();

        if (selectedColors.size() == 0 || selectedColors.size() == model()->rowCount() - 1) {
            QIcon icon = KisIconUtils::loadIcon("view-filter");
            QPixmap pixmap = icon.pixmap(QSize(size, size), !isEnabled() ? QIcon::Disabled : QIcon::Normal);
            painter.drawPixmap(editRect.right() - size, editRect.top(), pixmap);

        } else if (selectedColors.size() == 1) {
            const int currentLabel = selectedColors.first();
            QColor currentColor = scm.colorLabel(currentLabel);

            if (currentColor.alpha() > 0) {
                painter.fillRect(editRect, currentColor);
            } else if (currentLabel == 0) {
                QPen oldPen = painter.pen();

                const int border = 4;
                QRect crossRect(0, 0, size - 2 * border, size - 2 * border);
                crossRect.moveCenter(editRect.center());

                QColor shade = opt.palette.dark().color();
                painter.setPen(QPen(shade, 2));
                painter.drawLine(crossRect.topLeft(), crossRect.bottomRight());
                painter.drawLine(crossRect.bottomLeft(), crossRect.topRight());
            }
        } else {
            const int border = 0;
            QRect pieRect(0, 0, size - 2 * border, size - 2 * border);
            pieRect.moveCenter(editRect.center());

            const int numColors = selectedColors.size();
            const int step = 16 * 360 / numColors;

            int currentAngle = 0;

            //painter.save(); // optimize out!
            painter.setRenderHint(QPainter::Antialiasing);

            for (int i = 0; i < numColors; i++) {
                QColor color = scm.colorLabel(selectedColors[i]);
                QBrush brush = color.alpha() > 0 ? QBrush(color) : QBrush(Qt::black, Qt::Dense4Pattern);
                painter.setPen(color);
                painter.setBrush(brush);

                painter.drawPie(pieRect, currentAngle, step);
                currentAngle += step;
            }
            //painter.restore(); // optimize out!
        }
    }

    // draw the icon and text
    //painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

QSize KisColorFilterCombo::minimumSizeHint() const
{
    return sizeHint();
}

QSize KisColorFilterCombo::sizeHint() const
{
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    const QStyleOption *baseOption = qstyleoption_cast<const QStyleOption *>(&opt);
    const int arrowSize = style()->pixelMetric(QStyle::PM_ScrollBarExtent, baseOption, this);

    const QSize originalHint = QComboBox::sizeHint();
    QSize sh(3 * arrowSize, originalHint.height());

    return sh;
}
