/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KoImageSelectionWidget.h"

#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeRegistry.h>
#include <KoResourceManager.h>
#include <KoShape.h>

#include <kfilewidget.h>
#include <KIO/Job>
#include <KLocale>
#include <KDialog>
#include <KDebug>
#include <QGridLayout>
#include <QStackedWidget>
#include <QPainter>

class ImageFilePreview : public QWidget
{
public:
    ImageFilePreview(QWidget *parent = 0);
    void showPreview(KoImageData *imageData);

protected:
    virtual void paintEvent(QPaintEvent *paintEvent);
    virtual void resizeEvent(QResizeEvent *event);

private:
    QPixmap m_pixmap;
    KoImageData *m_imageData;
};

ImageFilePreview::ImageFilePreview(QWidget *parent)
    : QWidget(parent),
    m_imageData(0)
{
}

void ImageFilePreview::showPreview(KoImageData *imageData)
{
    m_imageData = imageData;
    update();
}

void ImageFilePreview::resizeEvent(QResizeEvent *)
{
    m_pixmap = QPixmap();
}

void ImageFilePreview::paintEvent(QPaintEvent *)
{
    if (!m_imageData || !m_imageData->isValid())
        return;
    if (m_pixmap.isNull()) {
        QSize pixmapSize = size();
        QSize origSize = m_imageData->image().size();
        const qreal xRatio = width() / (qreal) origSize.width();
        const qreal yRatio = height() / (qreal) origSize.height();
        if (xRatio > yRatio) // then lets make the vertical set the size.
            pixmapSize.setWidth(origSize.width() * yRatio);
        else
            pixmapSize.setHeight(origSize.height() * xRatio);
        m_pixmap = m_imageData->pixmap(pixmapSize);
    }
    QPainter painter(this);
    painter.drawPixmap(QRect(QPoint(), m_pixmap.size()), m_pixmap);
    painter.end();
}


class KoImageSelectionWidget::Private
{
public:
    Private(KoImageSelectionWidget *qq, KoImageCollection *c)
        : q(qq),
        collection(c),
        stack(0),
        fileWidget(0),
        filePreview(0),
        imageData(0)
    {
    }

    void acceptFileSelection();
    void setImageData(KJob *job);

    KoImageSelectionWidget *q;
    KoImageCollection *collection;
    QStackedWidget *stack;
    KFileWidget *fileWidget;
    ImageFilePreview *filePreview;
    KoImageData *imageData;
};

void KoImageSelectionWidget::Private::acceptFileSelection()
{
    fileWidget->accept();
    stack->setCurrentIndex(1);
    KUrl url = fileWidget->selectedUrl();
    if (!url.isEmpty()) {
        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, 0);
        connect(job, SIGNAL(result(KJob*)), q, SLOT(setImageData(KJob*)));
    }
}

void KoImageSelectionWidget::Private::setImageData(KJob *job)
{
    KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);
    delete imageData;
    imageData = collection->createImageData(transferJob->data());
    filePreview->showPreview(imageData);
    emit q->imageAvailable(imageData->isValid());
}


KoImageSelectionWidget::KoImageSelectionWidget(KoImageCollection *collection, QWidget *parent)
    : QWidget(parent),
    d (new Private(this, collection))
{
    Q_ASSERT(collection);
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);

    d->stack = new QStackedWidget(this);

    d->fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenDialog"), this);
    d->fileWidget->setOperationMode(KFileWidget::Opening);
    d->fileWidget->setFilter("image/png image/jpeg image/gif");
    //d->fileWidget->setMinimumSize( QSize(640,480) );
    d->filePreview = new ImageFilePreview(this);

    d->stack->addWidget(d->fileWidget);
    d->stack->addWidget(d->filePreview);

    layout->addWidget(d->stack, 0, 0, 1, -1);
    layout->setColumnStretch(0, 10);

    connect(d->fileWidget, SIGNAL(accepted()), this, SLOT(acceptFileSelection()));
}

KoImageSelectionWidget::~KoImageSelectionWidget()
{
    delete d;
}

bool KoImageSelectionWidget::hasValidImage() const
{
    return d->imageData && d->imageData->isValid();
}

KoImageData *KoImageSelectionWidget::imageData() const
{
    return d->imageData;
}

// static
KoImageData *KoImageSelectionWidget::selectImage(KoImageCollection *collection, QWidget *parent)
{
    KDialog *dialog = new KDialog(parent);
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    KoImageSelectionWidget *widget = new KoImageSelectionWidget(collection, dialog);
    dialog->setMainWidget(widget);
    connect(widget, SIGNAL(imageAvailable(bool)), dialog, SLOT(enableButtonOk(bool)));

    if (dialog->exec() == KDialog::Accepted)
        return widget->imageData();
    return 0;
}

// static
KoShape *KoImageSelectionWidget::selectImageShape(KoResourceManager *documentResources, QWidget *parent)
{
    if (!documentResources || !documentResources->imageCollection())
        return 0;
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("PictureShape");
    if (!factory) {
        kWarning(30003) << "No picture shape found, installation problem";
        return 0;
    }
    KoImageData *data = selectImage(documentResources->imageCollection(), parent);
    if (data) {
        KoShape *shape = factory->createDefaultShape(documentResources);
        shape->setUserData(data);
        shape->setSize(data->imageSize());
        return shape;
    }
    return 0;
}

#include <KoImageSelectionWidget.moc>
