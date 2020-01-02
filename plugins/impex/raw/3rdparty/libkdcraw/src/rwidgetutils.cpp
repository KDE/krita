/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2014-09-12
 * @brief  Simple helper widgets collection
 *
 * @author Copyright (C) 2014-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rwidgetutils.h"

// Qt includes

#include <QWidget>
#include <QByteArray>
#include <QBuffer>
#include <QImage>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QFileInfo>
#include <QPainter>
#include <QStandardPaths>
#include <QVector>
#include <QColorDialog>
#include <QStyleOptionButton>
#include <qdrawutil.h>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "libkdcraw_debug.h"

namespace KDcrawIface
{

RActiveLabel::RActiveLabel(const QUrl& url, const QString& imgPath, QWidget* const parent)
    : QLabel(parent)
{
    setMargin(0);
    setScaledContents(false);
    setOpenExternalLinks(true);
    setTextFormat(Qt::RichText);
    setFocusPolicy(Qt::NoFocus);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    QImage img = QImage(imgPath);

    updateData(url, img);
}

RActiveLabel::~RActiveLabel()
{
}

void RActiveLabel::updateData(const QUrl& url, const QImage& img)
{
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    setText(QString::fromLatin1("<a href=\"%1\">%2</a>")
            .arg(url.url())
            .arg(QString::fromLatin1("<img src=\"data:image/png;base64,%1\">")
            .arg(QString::fromLatin1(byteArray.toBase64().data()))));
}

// ------------------------------------------------------------------------------------

RLineWidget::RLineWidget(Qt::Orientation orientation, QWidget* const parent)
    : QFrame(parent)
{
    setLineWidth(1);
    setMidLineWidth(0);

    if (orientation == Qt::Vertical)
    {
        setFrameShape(QFrame::VLine);
        setFrameShadow(QFrame::Sunken);
        setMinimumSize(2, 0);
    }
    else
    {
        setFrameShape(QFrame::HLine);
        setFrameShadow(QFrame::Sunken);
        setMinimumSize(0, 2);
    }

    updateGeometry();
}

RLineWidget::~RLineWidget()
{
}

// ------------------------------------------------------------------------------------

RHBox::RHBox(QWidget* const parent)
    : QFrame(parent)
{
    QHBoxLayout* const layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

RHBox::RHBox(bool /*vertical*/, QWidget* const parent)
    : QFrame(parent)
{
    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

RHBox::~RHBox()
{
}

void RHBox::childEvent(QChildEvent* e)
{
    switch (e->type())
    {
        case QEvent::ChildAdded:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->addWidget(w);
            }

            break;
        }

        case QEvent::ChildRemoved:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->removeWidget(w);
            }

            break;
        }

        default:
            break;
    }

    QFrame::childEvent(e);
}

QSize RHBox::sizeHint() const
{
    RHBox* const b = const_cast<RHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded);

    return QFrame::sizeHint();
}

QSize RHBox::minimumSizeHint() const
{
    RHBox* const b = const_cast<RHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded );

    return QFrame::minimumSizeHint();
}

void RHBox::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void RHBox::setMargin(int margin)
{
    layout()->setMargin(margin);
}

void RHBox::setStretchFactor(QWidget* const widget, int stretch)
{
    static_cast<QBoxLayout*>(layout())->setStretchFactor(widget, stretch);
}

// ------------------------------------------------------------------------------------

RVBox::RVBox(QWidget* const parent)
  : RHBox(true, parent)
{
}

RVBox::~RVBox()
{
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN RAdjustableLabel::Private
{
public:

    Private()
    {
        emode = Qt::ElideMiddle;
    }

    QString           ajdText;
    Qt::TextElideMode emode;
};

RAdjustableLabel::RAdjustableLabel(QWidget* const parent)
    : QLabel(parent),
      d(new Private)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

RAdjustableLabel::~RAdjustableLabel()
{
    delete d;
}

void RAdjustableLabel::resizeEvent(QResizeEvent*)
{
    adjustTextToLabel();
}

QSize RAdjustableLabel::minimumSizeHint() const
{
    QSize sh = QLabel::minimumSizeHint();
    sh.setWidth(-1);
    return sh;
}

QSize RAdjustableLabel::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    int maxW     = QApplication::desktop()->screenGeometry(this).width() * 3 / 4;
    int currentW = fm.width(d->ajdText);

    return (QSize(currentW > maxW ? maxW : currentW, QLabel::sizeHint().height()));
}

void RAdjustableLabel::setAdjustedText(const QString& text)
{
    d->ajdText = text;

    if (d->ajdText.isNull())
        QLabel::clear();

    adjustTextToLabel();
}

QString RAdjustableLabel::adjustedText() const
{
    return d->ajdText;
}

void RAdjustableLabel::setAlignment(Qt::Alignment alignment)
{
    QString tmp(d->ajdText);
    QLabel::setAlignment(alignment);
    d->ajdText = tmp;
}

void RAdjustableLabel::setElideMode(Qt::TextElideMode mode)
{
    d->emode = mode;
    adjustTextToLabel();
}

void RAdjustableLabel::adjustTextToLabel()
{
    QFontMetrics fm(fontMetrics());
    QStringList adjustedLines;
    int lblW      = size().width();
    bool adjusted = false;

    Q_FOREACH(const QString& line, d->ajdText.split(QLatin1Char('\n')))
    {
        int lineW = fm.width(line);

        if (lineW > lblW)
        {
            adjusted = true;
            adjustedLines << fm.elidedText(line, d->emode, lblW);
        }
        else
        {
            adjustedLines << line;
        }
    }

    if (adjusted)
    {
        QLabel::setText(adjustedLines.join(QStringLiteral("\n")));
        setToolTip(d->ajdText);
    }
    else
    {
        QLabel::setText(d->ajdText);
        setToolTip(QString());
    }
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN RFileSelector::Private
{
public:

    Private()
    {
        edit      = 0;
        btn       = 0;
        fdMode    = QFileDialog::ExistingFile;
        fdOptions = QFileDialog::DontUseNativeDialog;
    }

    QLineEdit*            edit;
    QPushButton*          btn;

    QFileDialog::FileMode fdMode;
    QString               fdFilter;
    QString               fdTitle;
    QFileDialog::Options  fdOptions;
};

RFileSelector::RFileSelector(QWidget* const parent)
    : RHBox(parent),
      d(new Private)
{
    d->edit    = new QLineEdit(this);
    d->btn     = new QPushButton(i18n("Browse..."), this);
    setStretchFactor(d->edit, 10);

    connect(d->btn, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

RFileSelector::~RFileSelector()
{
    delete d;
}

QLineEdit* RFileSelector::lineEdit() const
{
    return d->edit;
}

void RFileSelector::setFileDlgMode(QFileDialog::FileMode mode)
{
    d->fdMode = mode;
}

void RFileSelector::setFileDlgFilter(const QString& filter)
{
    d->fdFilter = filter;
}

void RFileSelector::setFileDlgTitle(const QString& title)
{
    d->fdTitle = title;
}

void RFileSelector::setFileDlgOptions(QFileDialog::Options opts)
{
    d->fdOptions = opts;
}

void RFileSelector::slotBtnClicked()
{
    if (d->fdMode == QFileDialog::ExistingFiles)
    {
        qCDebug(LIBKDCRAW_LOG) << "Multiple selection is not supported";
        return;
    }

    QFileDialog* const fileDlg = new QFileDialog(this);
    fileDlg->setOptions(d->fdOptions);
    fileDlg->setDirectory(QFileInfo(d->edit->text()).dir());
    fileDlg->setFileMode(d->fdMode);

    if (!d->fdFilter.isNull())
        fileDlg->setNameFilter(d->fdFilter);

    if (!d->fdTitle.isNull())
        fileDlg->setWindowTitle(d->fdTitle);

    connect(fileDlg, SIGNAL(urlSelected(QUrl)),
            this, SIGNAL(signalUrlSelected(QUrl)));

    emit signalOpenFileDialog();

    if (fileDlg->exec() == QDialog::Accepted)
    {
        QStringList sel = fileDlg->selectedFiles();

        if (!sel.isEmpty())
        {
            d->edit->setText(sel.first());
        }
    }

    delete fileDlg;
}

// ---------------------------------------------------------------------------------------

WorkingPixmap::WorkingPixmap()
{
    QPixmap pix(QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("libkdcraw/pics/process-working.png")));
    QSize   size(22, 22);

    if (pix.isNull())
    {
        qCWarning(LIBKDCRAW_LOG) << "Invalid pixmap specified.";
        return;
    }

    if (!size.isValid())
    {
        size = QSize(pix.width(), pix.width());
    }

    if (pix.width() % size.width() || pix.height() % size.height())
    {
        qCWarning(LIBKDCRAW_LOG) << "Invalid framesize.";
        return;
    }

    const int rowCount = pix.height() / size.height();
    const int colCount = pix.width()  / size.width();
    m_frames.resize(rowCount * colCount);

    int pos = 0;

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            QPixmap frm     = pix.copy(col * size.width(), row * size.height(), size.width(), size.height());
            m_frames[pos++] = frm;
        }
    }
}

WorkingPixmap::~WorkingPixmap()
{
}

bool WorkingPixmap::isEmpty() const
{
    return m_frames.isEmpty();
}

QSize WorkingPixmap::frameSize() const
{
    if (isEmpty())
    {
        qCWarning(LIBKDCRAW_LOG) << "No frame loaded.";
        return QSize();
    }

    return m_frames[0].size();
}

int WorkingPixmap::frameCount() const
{
    return m_frames.size();
}

QPixmap WorkingPixmap::frameAt(int index) const
{
    if (isEmpty())
    {
        qCWarning(LIBKDCRAW_LOG) << "No frame loaded.";
        return QPixmap();
    }

    return m_frames.at(index);
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN RColorSelector::Private
{
public:

    Private()
    {
    }

    QColor color;
};

RColorSelector::RColorSelector(QWidget* const parent)
    : QPushButton(parent),
      d(new Private)
{
    connect(this, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

RColorSelector::~RColorSelector()
{
    delete d;
}

void RColorSelector::setColor(const QColor& color)
{
    if (color.isValid())
    {
        d->color = color;
        update();
    }
}

QColor RColorSelector::color() const
{
    return d->color;
}

void RColorSelector::slotBtnClicked()
{
    QColor color = QColorDialog::getColor(d->color);

    if (color.isValid())
    {
        setColor(color);
        emit signalColorSelected(color);
    }
}

void RColorSelector::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QStyle* const style = QWidget::style();

    QStyleOptionButton opt;

    opt.initFrom(this);
    opt.state    |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    opt.features  = QStyleOptionButton::None;
    opt.icon      = QIcon();
    opt.text.clear();

    style->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    QRect labelRect = style->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
    int shift       = style->pixelMetric(QStyle::PM_ButtonMargin, &opt, this) / 2;
    labelRect.adjust(shift, shift, -shift, -shift);
    int x, y, w, h;
    labelRect.getRect(&x, &y, &w, &h);

    if (isChecked() || isDown())
    {
        x += style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this);
        y += style->pixelMetric(QStyle::PM_ButtonShiftVertical,   &opt, this);
    }

    QColor fillCol = isEnabled() ? d->color : palette().color(backgroundRole());
    qDrawShadePanel(&painter, x, y, w, h, palette(), true, 1, 0);

    if (fillCol.isValid())
    {
        const QRect rect(x + 1, y + 1, w - 2, h - 2);

        if (fillCol.alpha() < 255)
        {
            QPixmap chessboardPattern(16, 16);
            QPainter patternPainter(&chessboardPattern);
            patternPainter.fillRect(0, 0, 8, 8, Qt::black);
            patternPainter.fillRect(8, 8, 8, 8, Qt::black);
            patternPainter.fillRect(0, 8, 8, 8, Qt::white);
            patternPainter.fillRect(8, 0, 8, 8, Qt::white);
            patternPainter.end();
            painter.fillRect(rect, QBrush(chessboardPattern));
        }

        painter.fillRect(rect, fillCol);
    }

    if (hasFocus())
    {
        QRect focusRect = style->subElementRect(QStyle::SE_PushButtonFocusRect, &opt, this);
        QStyleOptionFocusRect focusOpt;
        focusOpt.init(this);
        focusOpt.rect            = focusRect;
        focusOpt.backgroundColor = palette().background().color();
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, &painter, this);
    }
}

} // namespace KDcrawIface
