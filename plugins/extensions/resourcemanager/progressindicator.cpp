#include "progressindicator_p.h"

#include "kjob.h"

#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QLayout>

#include <QIcon>
#include <kpixmapsequencewidget.h>

ProgressIndicator::ProgressIndicator(QWidget *parent)
    : QFrame(parent)
    , m_busyPixmap(QIcon::fromTheme(QStringLiteral("process-working")))
    , m_errorPixmap(QIcon::fromTheme(QStringLiteral("dialog-error")))
{
    setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);

    //busy widget

    busyWidget = new KPixmapSequenceWidget(this);
    busyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    busyWidget->setVisible(false);
    hbox->addWidget(busyWidget);

    m_statusLabel = new QLabel();
    hbox->addWidget(m_statusLabel);
}

void ProgressIndicator::busy(const QString &message)
{
    m_statusLabel->setText(message);
    busyWidget->setVisible(true);
    busyWidget->setSequence(m_busyPixmap);
}

void ProgressIndicator::error(const QString &message)
{
    m_statusLabel->setText(message);
    busyWidget->setVisible(true);
    busyWidget->setSequence(m_errorPixmap);
}

void ProgressIndicator::idle(const QString &message)
{
    m_statusLabel->setText(message);
    busyWidget->setVisible(false);
}
