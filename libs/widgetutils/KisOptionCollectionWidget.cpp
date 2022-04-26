/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QVariant>
#include <QBoxLayout>

#include <kis_assert.h>

#include "KisOptionCollectionWidget.h"

class KisOptionCollectionWidgetWrapper : public QWidget
{
    Q_OBJECT

public:
    KisOptionCollectionWidgetWrapper(QWidget *parent, QWidget *widget)
        : QWidget(parent)
        , m_widget(widget)
        , m_margin(0)
    {
        m_widget->setParent(this);

        QBoxLayout *layoutWidget = new QBoxLayout(QBoxLayout::TopToBottom);
        layoutWidget->setContentsMargins(m_margin, 0, m_margin, 0);
        layoutWidget->setSpacing(0);
        layoutWidget->addWidget(m_widget);

        m_separator = new QFrame(this);
        m_separator->setFrameShape(QFrame::HLine);
        m_separator->setFrameShadow(QFrame::Sunken);
        m_separator->setFixedHeight(10);
    
        QBoxLayout *layoutMain = new QBoxLayout(QBoxLayout::LeftToRight);
        layoutMain->setContentsMargins(0, 0, 0, 0);
        layoutMain->setSpacing(0);
        layoutMain->addLayout(layoutWidget);
        layoutMain->addWidget(m_separator);

        setLayout(layoutMain);
    }

    ~KisOptionCollectionWidgetWrapper() override {}

    QWidget* widget() const
    {
        return m_widget;
    }

    void setSeparatorVisible(bool visible)
    {
        QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
        const bool isVisible = layoutMain->count() > 1;
        if (isVisible == visible) {
            return;
        }
        if (isVisible) {
            layoutMain->takeAt(1);
            m_separator->setVisible(false);
        } else {
            layoutMain->insertWidget(1, m_separator);
            m_separator->setVisible(true);
        }
    }

    void setOrientation(Qt::Orientation orientation)
    {
        QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
        QBoxLayout *layoutWidget = dynamic_cast<QBoxLayout*>(layoutMain->itemAt(0)->layout());
        if (orientation == Qt::Vertical) {
            if (layoutMain->direction() == QBoxLayout::TopToBottom) {
                return;
            }
            layoutWidget->setContentsMargins(m_margin, 0, m_margin, 0);
            m_separator->setFixedSize(QWIDGETSIZE_MAX, 10);
            m_separator->setFrameShape(QFrame::HLine);
            layoutMain->setDirection(QBoxLayout::TopToBottom);
        } else {
            if (layoutMain->direction() == QBoxLayout::LeftToRight) {
                return;
            }
            layoutWidget->setContentsMargins(0, 0, 0, 0);
            m_separator->setFixedSize(20, QWIDGETSIZE_MAX);
            m_separator->setFrameShape(QFrame::VLine);
            layoutMain->setDirection(QBoxLayout::LeftToRight);
        }
    }

    void setWidgetMargin(int margin)
    {
        if (m_margin == margin) {
            return;
        }
        m_margin = margin;
        QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
        if (layoutMain->direction() == QBoxLayout::TopToBottom) {
            QBoxLayout *layoutWidget = dynamic_cast<QBoxLayout*>(layoutMain->itemAt(0)->layout());
            layoutWidget->setContentsMargins(margin, 0, margin, 0);
        }
    }

private:
    QWidget *m_widget;
    QFrame *m_separator;
    int m_margin;
};

struct KisOptionCollectionWidgetWithHeader::Private
{
    KisOptionCollectionWidgetWithHeader *q{nullptr};
    QLabel *label{nullptr};
    QWidget *primaryWidget{nullptr};
    KisOptionCollectionWidget *widgetCollection{nullptr};
    QBoxLayout *layoutHeader{nullptr};
    QBoxLayout *layoutPrimaryWidget{nullptr};
    QBoxLayout *layoutWidgets{nullptr};
    Qt::Orientation orientation{Qt::Vertical};

    Private(KisOptionCollectionWidgetWithHeader *q) : q(q) {}

    void adjustPrimaryWidget()
    {
        if (!primaryWidget || !primaryWidget->isVisible()) {
            return;
        }

        if (orientation == Qt::Horizontal) {
            if (layoutHeader->direction() == QBoxLayout::LeftToRight) {
                return;
            }
            layoutHeader->setDirection(QBoxLayout::LeftToRight);
            layoutHeader->setSpacing(10);
            layoutPrimaryWidget->setContentsMargins(0, 0, 0, 0);
            return;
        }
        // label.width + primaryWidget.size + 10px spacing + 4px extra (just in case)
        const int minimumHeaderWidth = label->minimumSizeHint().width() +
                                       primaryWidget->minimumSizeHint().width() +
                                       10 + 4;
        if (q->width() < minimumHeaderWidth) {
            if (layoutHeader->direction() == QBoxLayout::TopToBottom) {
                return;
            }
            layoutHeader->setDirection(QBoxLayout::TopToBottom);
            layoutHeader->setSpacing(5);
            layoutPrimaryWidget->setContentsMargins(5, 0, 0, 0);
        } else {
            if (layoutHeader->direction() == QBoxLayout::LeftToRight) {
                return;
            }
            layoutHeader->setDirection(QBoxLayout::LeftToRight);
            layoutHeader->setSpacing(10);
            layoutPrimaryWidget->setContentsMargins(0, 0, 0, 0);
        }
    }

    void adjustWidgetCollection()
    {
        QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(q->layout());
        if (layoutMain->indexOf(layoutWidgets) == -1) {
            if (widgetCollection->numberOfVisibleWidgets() == 0) {
                return;
            }
            layoutMain->insertLayout(1, layoutWidgets, 1);
            widgetCollection->setVisible(true);
        } else {
            if (widgetCollection->numberOfVisibleWidgets() != 0) {
                return;
            }
            layoutMain->takeAt(1);
            widgetCollection->setVisible(false);
        }
    }
    
};

struct KisOptionCollectionWidget::Private
{
    KisOptionCollectionWidget *q{nullptr};
    QVector<KisOptionCollectionWidgetWrapper*> widgetWrappers;
    bool showSeparators{false};
    Qt::Orientation orientation{Qt::Vertical};
    int widgetsMargin{10};

    Private(KisOptionCollectionWidget *q) : q(q) {}

    void insertWidget(int index, const QString &id, QWidget *widget)
    {
        KisOptionCollectionWidgetWrapper *widgetWrapper =
            new KisOptionCollectionWidgetWrapper(q, widget);
        widgetWrapper->setProperty("id", id);
        widgetWrapper->setSeparatorVisible(showSeparators);
        widgetWrapper->setOrientation(orientation);
        widgetWrapper->setWidgetMargin(widgetsMargin);
        widgetWrappers.insert(index, widgetWrapper);

        QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(q->layout());
        int indexLayout;
        for (indexLayout = 0; indexLayout < layoutMain->count(); ++indexLayout) {
            const QWidget *prevWidget = layoutMain->itemAt(indexLayout)->widget();
            const int prevIndex = q->widgetIndexFromId(prevWidget->property("id").toString());
            if (prevIndex >= index) {
                break;
            }
        }
        layoutMain->insertWidget(indexLayout, widgetWrapper);
        adjustSeparators();
    }

    void adjustSeparators()
    {
        if (q->layout()->count() == 0) {
            return;
        }
        for (int i = 0; i < q->layout()->count() - 1; ++i) {
            KisOptionCollectionWidgetWrapper *widgetWrapper =
                qobject_cast<KisOptionCollectionWidgetWrapper*>(q->layout()->itemAt(i)->widget());
            widgetWrapper->setSeparatorVisible(showSeparators);
        }
        KisOptionCollectionWidgetWrapper *lastWidgetWrapper =
            qobject_cast<KisOptionCollectionWidgetWrapper*>(q->layout()->itemAt(q->layout()->count() - 1)->widget());
        lastWidgetWrapper->setSeparatorVisible(false);
    }

    int widgetIndexFromId(QStringRef id) const
    {
        for (int i = 0; i < widgetWrappers.size(); ++i) {
            if (widgetWrappers[i]->property("id").toString() == id) {
                return i;
            }
        }
        return -1;
    }

    bool containsWidget(QStringRef id) const
    {
        return widgetIndexFromId(id) != -1;
    }

    QWidget* widget(int index) const
    {
        return widgetWrappers[index]->widget();
    }

    QWidget* widget(QStringRef id) const
    {
        return widget(widgetIndexFromId(id));
    }

    QWidget* findWidget(QStringRef path, QWidget *candidate = nullptr) const
    {
        while (path.startsWith('/')) {
            path = path.right(path.size() - 1);
        }
        if (path.isEmpty()) {
            return candidate;
        }
        const int slashPosition = path.indexOf('/');
        if (slashPosition == -1) {
            return widget(path);
        }
        QWidget *nextCandidate = widget(path.left(slashPosition));
        if (nextCandidate) {
            KisOptionCollectionWidget *optionCollectionWidget =
                qobject_cast<KisOptionCollectionWidget*>(nextCandidate);
            if (optionCollectionWidget) {
                return optionCollectionWidget->m_d->findWidget(path.right(path.size() - slashPosition), nextCandidate);
            }
            KisOptionCollectionWidgetWithHeader *optionCollectionWidgetWithHeader =
                qobject_cast<KisOptionCollectionWidgetWithHeader*>(nextCandidate);
            if (optionCollectionWidgetWithHeader) {
                return optionCollectionWidgetWithHeader->m_d->widgetCollection->m_d->findWidget(path.right(path.size() - slashPosition), nextCandidate);
            }
            QStringRef rest = path.right(path.size() - slashPosition);
            while (rest.startsWith('/')) {
                rest = rest.right(rest.size() - 1);
            }
            if (rest.isEmpty()) {
                return nextCandidate;
            }
        }
        return nullptr;
    }
};

KisOptionCollectionWidget::KisOptionCollectionWidget(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private(this))
{
    QBoxLayout *layoutMain = new QBoxLayout(QBoxLayout::TopToBottom);
    layoutMain->setSpacing(5);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
}

KisOptionCollectionWidget::~KisOptionCollectionWidget()
{}

int KisOptionCollectionWidget::widgetIndexFromId(const QString &id) const
{
    return m_d->widgetIndexFromId(&id);
}

bool KisOptionCollectionWidget::containsWidget(const QString &id) const
{
    return m_d->containsWidget(&id);
}

QWidget* KisOptionCollectionWidget::widget(int index) const
{
    return m_d->widget(index);
}

QWidget* KisOptionCollectionWidget::widget(const QString &id) const
{
    return m_d->widget(&id);
}

QWidget* KisOptionCollectionWidget::findWidget(const QString &path) const
{
    return m_d->findWidget(&path);
}

void KisOptionCollectionWidget::insertWidget(int index, const QString &id, QWidget *widget)
{
    if (containsWidget(id)) {
        return;
    }
    m_d->insertWidget(index, id, widget);
}

void KisOptionCollectionWidget::appendWidget(const QString &id, QWidget *widget)
{
    insertWidget(m_d->widgetWrappers.size(), id, widget);
}

void KisOptionCollectionWidget::removeWidget(int index)
{
    delete takeWidget(index);
}

void KisOptionCollectionWidget::removeWidget(const QString &id)
{
    removeWidget(widgetIndexFromId(id));
}

QWidget* KisOptionCollectionWidget::takeWidget(int index)
{
    QWidget* widget = m_d->widgetWrappers[index]->widget();
    widget->setParent(nullptr);
    m_d->widgetWrappers.removeAt(index);
    m_d->adjustSeparators();
    return widget;
}

QWidget* KisOptionCollectionWidget::takeWidget(const QString &id)
{
    return takeWidget(widgetIndexFromId(id));
}

void KisOptionCollectionWidget::setWidgetVisible(int index, bool visible)
{
    KisOptionCollectionWidgetWrapper *widgetWrapper = m_d->widgetWrappers[index];

    QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());

    if (visible) {
        if (layoutMain->indexOf(widgetWrapper) != -1) {
            return;
        }
        int indexLayout;
        for (indexLayout = 0; indexLayout < layoutMain->count(); ++indexLayout) {
            const QWidget *prevWidget = layoutMain->itemAt(indexLayout)->widget();
            const int prevIndex = widgetIndexFromId(prevWidget->property("id").toString());
            if (prevIndex >= index) {
                break;
            }
        }
        layoutMain->insertWidget(indexLayout, widgetWrapper);
        widgetWrapper->setVisible(true);
    } else {
        if (layoutMain->indexOf(widgetWrapper) == -1) {
            return;
        }
        layoutMain->takeAt(layoutMain->indexOf(widgetWrapper));
        widgetWrapper->setVisible(false);
    }

    m_d->adjustSeparators();
}

void KisOptionCollectionWidget::setWidgetVisible(const QString &id, bool visible)
{
    setWidgetVisible(widgetIndexFromId(id), visible);
}

void KisOptionCollectionWidget::setWidgetsMargin(int margin)
{
    if (margin == m_d->widgetsMargin) {
        return;
    }
    m_d->widgetsMargin = margin;
    for (KisOptionCollectionWidgetWrapper *widgetWrapper : m_d->widgetWrappers) {
        widgetWrapper->setWidgetMargin(margin);
    }
}

void KisOptionCollectionWidget::setSeparatorsVisible(bool visible)
{
    if (visible == m_d->showSeparators) {
        return;
    }
    m_d->showSeparators = visible;
    QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
    layoutMain->setSpacing(visible ? 0 : 5);
    m_d->adjustSeparators();
}

void KisOptionCollectionWidget::setOrientation(Qt::Orientation orientation, bool recursive)
{
    if (orientation == m_d->orientation) {
        return;
    }
    m_d->orientation = orientation;
    for (KisOptionCollectionWidgetWrapper *widgetWrapper : m_d->widgetWrappers) {
        widgetWrapper->setOrientation(orientation);
        if (recursive) {
            KisOptionCollectionWidget *optionCollectionWidget =
                qobject_cast<KisOptionCollectionWidget*>(widgetWrapper->widget());
            if (optionCollectionWidget) {
                optionCollectionWidget->setOrientation(orientation, true);
                continue;
            }
            KisOptionCollectionWidgetWithHeader *optionCollectionWidgetWithHeader =
                qobject_cast<KisOptionCollectionWidgetWithHeader*>(widgetWrapper->widget());
            if (optionCollectionWidgetWithHeader) {
                optionCollectionWidgetWithHeader->setOrientation(orientation, true);
            }
        }
    }
    QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
    layoutMain->setDirection(orientation == Qt::Vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
}

int KisOptionCollectionWidget::size() const
{
    return m_d->widgetWrappers.size();
}

int KisOptionCollectionWidget::numberOfVisibleWidgets() const
{
    return layout()->count();
}

KisOptionCollectionWidgetWithHeader::KisOptionCollectionWidgetWithHeader(const QString &title,
                                                                         QWidget *parent)
    : QWidget(parent)
    , m_d(new Private(this))
{
    m_d->label = new QLabel(title, this);
    
    m_d->layoutPrimaryWidget = new QBoxLayout(QBoxLayout::LeftToRight);
    m_d->layoutPrimaryWidget->setSpacing(0);
    m_d->layoutPrimaryWidget->setContentsMargins(0, 0, 0, 0);

    m_d->layoutHeader = new QBoxLayout(QBoxLayout::LeftToRight);
    m_d->layoutHeader->setSpacing(10);
    m_d->layoutHeader->setContentsMargins(0, 0, 0, 0);
    m_d->layoutHeader->addWidget(m_d->label, 0);

    m_d->widgetCollection = new KisOptionCollectionWidget(this);
    m_d->widgetCollection->setWidgetsMargin(0);
    m_d->widgetCollection->setVisible(false);

    m_d->layoutWidgets = new QBoxLayout(QBoxLayout::TopToBottom);
    m_d->layoutWidgets->setSpacing(0);
    m_d->layoutWidgets->setContentsMargins(5, 0, 0, 0);
    m_d->layoutWidgets->addWidget(m_d->widgetCollection);

    QBoxLayout *layoutMain = new QBoxLayout(QBoxLayout::TopToBottom);
    layoutMain->setSpacing(5);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addLayout(m_d->layoutHeader, 0);
    
    setLayout(layoutMain);
}

KisOptionCollectionWidgetWithHeader::~KisOptionCollectionWidgetWithHeader()
{}

QSize KisOptionCollectionWidgetWithHeader::minimumSizeHint() const
{
    if (m_d->orientation == Qt::Horizontal) {
        return QWidget::minimumSizeHint();
    }

    const QSize widgetListMinimumSizeHint = m_d->layoutWidgets->minimumSize();
    // label.width + primaryWidget.size + 10px spacing + 4px extra (just in case)
    const int minimumHeaderWidth = m_d->label->minimumSizeHint().width() +
                                   (m_d->primaryWidget ? m_d->primaryWidget->minimumSizeHint().width() + 10 + 4 : 0);
    const QSize headerMinimumSizeHint =
        QSize(
            qMax(
                m_d->label->minimumSizeHint().width(),
                m_d->primaryWidget ? m_d->layoutPrimaryWidget->minimumSize().width() : 0
            ),
            width() < minimumHeaderWidth
            ? m_d->label->minimumSizeHint().height() +
              (m_d->primaryWidget ? m_d->layoutPrimaryWidget->minimumSize().height() + m_d->layoutHeader->spacing() : 0)
            : qMax(
                m_d->label->minimumSizeHint().height(),
                m_d->primaryWidget ? m_d->layoutPrimaryWidget->minimumSize().height() : 0
              )
        );

    return
        QSize(
            qMax(widgetListMinimumSizeHint.width(), headerMinimumSizeHint.width()),
            headerMinimumSizeHint.height() +
                (m_d->widgetCollection->numberOfVisibleWidgets() > 0 ? widgetListMinimumSizeHint.height() + layout()->spacing() : 0)
        );
}

QWidget* KisOptionCollectionWidgetWithHeader::primaryWidget() const
{
    return m_d->primaryWidget;
}

void KisOptionCollectionWidgetWithHeader::setPrimaryWidget(QWidget *widget)
{
    if (widget == m_d->primaryWidget) {
        return;
    }
    const bool wasPrimaryWidgetVisible =
        m_d->primaryWidget ? m_d->primaryWidget->isVisible() : true;
    removePrimaryWidget();
    if (!widget) {
        return;
    }
    m_d->primaryWidget = widget;
    widget->setParent(this);
    m_d->layoutPrimaryWidget->insertWidget(0, widget);

    if (!wasPrimaryWidgetVisible) {
        widget->setVisible(false);
        return;
    }

    m_d->layoutHeader->insertLayout(1, m_d->layoutPrimaryWidget, 1);
    widget->setVisible(true);
    m_d->adjustPrimaryWidget();
}

void KisOptionCollectionWidgetWithHeader::removePrimaryWidget()
{
    if (!m_d->primaryWidget) {
        return;
    }
    delete takePrimaryWidget();
}

QWidget* KisOptionCollectionWidgetWithHeader::takePrimaryWidget()
{
    QWidget *widget = m_d->primaryWidget;
    if (!widget) {
        return nullptr;
    }
    m_d->primaryWidget = nullptr;
    widget->setParent(nullptr);
    m_d->layoutHeader->takeAt(1);
    m_d->adjustPrimaryWidget();
    return widget;

}

void KisOptionCollectionWidgetWithHeader::setPrimaryWidgetVisible(bool visible)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->primaryWidget);

    if (visible == m_d->primaryWidget->isVisible()) {
        return;
    }

    if (m_d->primaryWidget->isVisible()) {
        m_d->layoutHeader->takeAt(1);
        m_d->primaryWidget->setVisible(false);
    } else {
        m_d->layoutHeader->insertLayout(1, m_d->layoutPrimaryWidget, 1);
        m_d->primaryWidget->setVisible(true);
        m_d->adjustPrimaryWidget();
    }
}

int KisOptionCollectionWidgetWithHeader::widgetIndexFromId(const QString &id) const
{
    return m_d->widgetCollection->widgetIndexFromId(id);
}

bool KisOptionCollectionWidgetWithHeader::containsWidget(const QString &id) const
{
    return m_d->widgetCollection->containsWidget(id);
}

QWidget* KisOptionCollectionWidgetWithHeader::widget(int index) const
{
    return m_d->widgetCollection->widget(index);
}

QWidget* KisOptionCollectionWidgetWithHeader::widget(const QString &id) const
{
    return m_d->widgetCollection->widget(id);
}

QWidget* KisOptionCollectionWidgetWithHeader::findWidget(const QString &path) const
{
    return m_d->widgetCollection->findWidget(path);
}

void KisOptionCollectionWidgetWithHeader::insertWidget(int index, const QString &id, QWidget *widget)
{
    m_d->widgetCollection->insertWidget(index, id, widget);
    m_d->adjustWidgetCollection();
}

void KisOptionCollectionWidgetWithHeader::appendWidget(const QString &id, QWidget *widget)
{
    m_d->widgetCollection->appendWidget(id, widget);
    m_d->adjustWidgetCollection();
}

void KisOptionCollectionWidgetWithHeader::removeWidget(int index)
{
    m_d->widgetCollection->removeWidget(index);
    m_d->adjustWidgetCollection();
}

void KisOptionCollectionWidgetWithHeader::removeWidget(const QString &id)
{
    m_d->widgetCollection->removeWidget(id);
    m_d->adjustWidgetCollection();
}

QWidget* KisOptionCollectionWidgetWithHeader::takeWidget(int index)
{
    QWidget *w = m_d->widgetCollection->takeWidget(index);
    m_d->adjustWidgetCollection();
    return w;
}

QWidget* KisOptionCollectionWidgetWithHeader::takeWidget(const QString &id)
{
    QWidget *w =  m_d->widgetCollection->takeWidget(id);
    m_d->adjustWidgetCollection();
    return w;
}

void KisOptionCollectionWidgetWithHeader::setWidgetVisible(int index, bool visible)
{
    m_d->widgetCollection->setWidgetVisible(index, visible);
    m_d->adjustWidgetCollection();
}

void KisOptionCollectionWidgetWithHeader::setWidgetVisible(const QString &id, bool visible)
{
    m_d->widgetCollection->setWidgetVisible(id, visible);
    m_d->adjustWidgetCollection();
}

void KisOptionCollectionWidgetWithHeader::setWidgetsMargin(int margin)
{
    m_d->widgetCollection->setWidgetsMargin(margin);
}

void KisOptionCollectionWidgetWithHeader::setSeparatorsVisible(bool visible)
{
    m_d->widgetCollection->setSeparatorsVisible(visible);
}

void KisOptionCollectionWidgetWithHeader::setOrientation(Qt::Orientation orientation, bool recursive)
{
    if (orientation == m_d->orientation) {
        return;
    }
    m_d->orientation = orientation;
    m_d->adjustPrimaryWidget();
    m_d->widgetCollection->setOrientation(orientation, recursive);
    m_d->layoutWidgets->setDirection(orientation == Qt::Vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    QBoxLayout *layoutMain = dynamic_cast<QBoxLayout*>(layout());
    layoutMain->setDirection(orientation == Qt::Vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
}

int KisOptionCollectionWidgetWithHeader::size() const
{
    return m_d->widgetCollection->size();
}

int KisOptionCollectionWidgetWithHeader::numberOfVisibleWidgets() const
{
    return m_d->widgetCollection->numberOfVisibleWidgets();
}

void KisOptionCollectionWidgetWithHeader::resizeEvent(QResizeEvent*)
{
    m_d->adjustPrimaryWidget();
}

#include "KisOptionCollectionWidget.moc"
