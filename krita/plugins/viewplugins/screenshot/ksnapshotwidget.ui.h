/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void KSnapshotWidget::slotModeChanged( int mode )
{
    switch ( mode )
    {
    case 0:
    lblDelay->setEnabled(true);
    spinDelay->setEnabled(true);
    cbIncludeDecorations->setEnabled(false);
    break;
    case 1:
    lblDelay->setEnabled(true);
    spinDelay->setEnabled(true);
    cbIncludeDecorations->setEnabled(true);
    break;
    case 2:
    lblDelay->setEnabled(false);
    spinDelay->setEnabled(false);
    cbIncludeDecorations->setEnabled(false);
    default:
    break;
    }
}


void KSnapshotWidget::setPreview( const QPixmap &pm )
{
    lblImage->setPixmap(pm);
}


void KSnapshotWidget::setDelay( int i )
{
    spinDelay->setValue(i);
}


void KSnapshotWidget::setIncludeDecorations( bool b )
{
    cbIncludeDecorations->setChecked(b);
}


void KSnapshotWidget::setMode( int mode )
{
    comboMode->setCurrentIndex(mode);
    slotModeChanged(mode);
}


int KSnapshotWidget::delay()
{
    return spinDelay->value();
}


bool KSnapshotWidget::includeDecorations()
{
    return cbIncludeDecorations->isChecked();
}


int KSnapshotWidget::mode()
{
    return comboMode->currentIndex();
}


void KSnapshotWidget::slotNewClicked()
{
    emit newClicked();
}


void KSnapshotWidget::slotSaveClicked()
{
    emit saveClicked();
}


void KSnapshotWidget::slotPrintClicked()
{
    emit printClicked();
}


void KSnapshotWidget::slotStartDrag()
{
    emit startImageDrag();
}


QPixmap KSnapshotWidget::preview()
{
    return *lblImage->pixmap();
}


int KSnapshotWidget::previewWidth()
{
    return lblImage->width();
}


int KSnapshotWidget::previewHeight()
{
    return lblImage->height();
}

