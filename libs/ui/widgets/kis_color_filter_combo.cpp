/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include <QStyleFactory>



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
    /**
      * if the combobox is in the filter mode
      *     (when no colors are selected)
      *     it will show the filter icon ("view-filter")
      *     otherwise it will show tag icon ("tag")
      */
    bool filterMode {true};
    /**
      * If the combobox is in the circle mode,
      *     it will show the selected colors as circle
      *     otherwise it will show it in a rectangle
      */
    bool circleMode {true};
};

KisColorFilterCombo::KisColorFilterCombo(QWidget *parent, bool filterMode, bool circleMode)
    : QComboBox(parent),
      m_d(new Private)
{
    m_d->filterMode = filterMode;
    m_d->circleMode = circleMode;


    QStandardItemModel *newModel = new QStandardItemModel(this);
    setModel(newModel);

    QStyle* newStyle = QStyleFactory::create(style()->objectName());
    // proxy style steals the ownership of the style and deletes it later
    PopupComboBoxStyle *proxyStyle = new PopupComboBoxStyle(newStyle);

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
    if (!rootNode.isNull()) {
        collectAvailableLabels(rootNode, &labels);
    }

    updateAvailableLabels(labels);
}

void KisColorFilterCombo::updateAvailableLabels(const QSet<int> &labels)
{
    m_d->filteringModel->setAcceptedLabels(labels);
}

void KisColorFilterCombo::setModes(bool filterMode, bool circleMode)
{
    m_d->filterMode = filterMode;
    m_d->circleMode = circleMode;
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

void KisColorFilterCombo::paintColorPie(QStylePainter &painter, const QPalette& palette, const QList<int> &selectedColors, const QRect &rect, const int &baseSize)
{
    KisNodeViewColorScheme scm;
    const QPen oldPen = painter.pen();
    const QBrush oldBrush = painter.brush();
    const int border = 0;
    QColor shadowColor = palette.shadow().color();
    shadowColor.setAlpha(64);

    QRect pieRect(0, 0, baseSize - 2 * border, baseSize - 2 * border);
    pieRect.moveCenter(rect.center());

    if (selectedColors.size() == 1) {
        const int currentLabel = selectedColors.first();
        const QColor currentColor = scm.colorFromLabelIndex(currentLabel);
        const QBrush brush = QBrush(currentColor);
        painter.setBrush(brush);
        painter.setPen(QPen(shadowColor, 1));

        if (currentColor.alpha() > 0) {
            painter.drawEllipse(rect);
        } else if (currentLabel == 0) {
            QColor white = Qt::white;
            QColor grey = QColor(220,220,220);
            painter.setBrush(QBrush(shadowColor));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawEllipse(rect);
            const int step = 16 * 360 / 4;
            const int checkerSteps = 4;

            for (int i = 0; i < checkerSteps; i++) {
                QBrush checkerBrush = QBrush((i % 2) ? grey : white);
                painter.setPen(Qt::NoPen);
                painter.setBrush(checkerBrush);
                painter.drawPie(pieRect, step * i, step);
            }

        }
    } else {
        const int numColors = selectedColors.size();
        const int step = 16 * 360 / numColors;

        painter.setPen(QPen(shadowColor, 1));
        painter.setBrush(QColor(0,0,0,0));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawEllipse(rect);
        for (int i = 0; i < numColors; i++) {
            QColor color = scm.colorFromLabelIndex(selectedColors[i]);
            QBrush brush = color.alpha() > 0 ? QBrush(color) : QBrush(Qt::black, Qt::Dense4Pattern);
            painter.setPen(Qt::NoPen);
            painter.setBrush(brush);

            painter.drawPie(pieRect, step * i, step);
        }
    }

    painter.setPen(oldPen);
    painter.setBrush(oldBrush);
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
        const QRect editRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
        const int size = qMin(editRect.width(), editRect.height());

        const QList<int> selectedColors = this->selectedColors();

        if (selectedColors.size() == 0 || selectedColors.size() == model()->rowCount() - 1) {
            QIcon icon = KisIconUtils::loadIcon(m_d->filterMode ? "view-filter" : "tag");
            QPixmap pixmap = icon.pixmap(QSize(size, size), !isEnabled() ? QIcon::Disabled : QIcon::Normal);
            painter.drawPixmap(editRect.right() - size, editRect.top(), pixmap);

        } else {
            const int numColors = selectedColors.size();
            if (m_d->circleMode) {
                KisColorFilterCombo::paintColorPie(painter, opt.palette, selectedColors, editRect, size );
            } else {
                // show all colors in a rectangle
                KisNodeViewColorScheme scm;

                int oneColorWidth = editRect.width()/numColors;
                int currentWidth = 0;
                for (int i = 0; i < numColors; i++) {
                    QColor color = scm.colorFromLabelIndex(selectedColors[i]);
                    QBrush brush = color.alpha() > 0 ? QBrush(color) : QBrush(Qt::black, Qt::Dense4Pattern);
                    painter.setPen(color);
                    painter.setBrush(brush);
                    if (i == numColors - 1) {
                        // last color; let's fill up
                        painter.fillRect(currentWidth, editRect.top(), editRect.width() - currentWidth, editRect.height(), brush);
                    } else {
                        painter.fillRect(currentWidth, editRect.top(), oneColorWidth, editRect.height(), brush);
                    }

                    currentWidth += oneColorWidth;
                }




            }
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
