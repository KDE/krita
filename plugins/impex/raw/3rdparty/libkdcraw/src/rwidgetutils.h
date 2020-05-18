/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2014-09-12
 * @brief  Simple helpher widgets collection
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

#ifndef RWIDGETUTILS_H
#define RWIDGETUTILS_H

// Qt includes

#include <QLabel>
#include <QFileInfo>
#include <QString>
#include <QFrame>
#include <QLineEdit>
#include <QSize>
#include <QPixmap>
#include <QFileDialog>
#include <QColor>
#include <QPushButton>

// Local includes



namespace KDcrawIface
{

/** A widget to host an image into a label with an active url which can be 
 *  open to default web browser using simple mouse click.
 */
class  RActiveLabel : public QLabel
{
    Q_OBJECT

public:

    explicit RActiveLabel(const QUrl& url=QUrl(), const QString& imgPath=QString(), QWidget* const parent=0);
    ~RActiveLabel() override;

    void updateData(const QUrl& url, const QImage& img);
};

// ------------------------------------------------------------------------------------

/**
 * A widget to show an horizontal or vertical line separator 
 **/
class  RLineWidget : public QFrame
{
    Q_OBJECT

public:

    explicit RLineWidget(Qt::Orientation orientation, QWidget* const parent=0);
    ~RLineWidget() override;
};

// ------------------------------------------------------------------------------------

/** An Horizontal widget to host children widgets
 */
class  RHBox : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(RHBox)

public:

    explicit RHBox(QWidget* const parent=0);
    ~RHBox() override;

    void setMargin(int margin);
    void setSpacing(int space);
    void setStretchFactor(QWidget* const widget, int stretch);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:

    RHBox(bool vertical, QWidget* const parent);

    void childEvent(QChildEvent* e) override;
};

// ------------------------------------------------------------------------------------

/** A Vertical widget to host children widgets
 */
class  RVBox : public RHBox
{
    Q_OBJECT
    Q_DISABLE_COPY(RVBox)

  public:

    explicit RVBox(QWidget* const parent=0);
    ~RVBox() override;
};

// ------------------------------------------------------------------------------------

/** A label to show text adjusted to widget size
 */
class  RAdjustableLabel : public QLabel
{
    Q_OBJECT

public:

    explicit RAdjustableLabel(QWidget* const parent=0);
    ~RAdjustableLabel() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint()        const override;

    void setAlignment(Qt::Alignment align);
    void setElideMode(Qt::TextElideMode mode);

    QString adjustedText() const;

public Q_SLOTS:

    void setAdjustedText(const QString& text=QString());

private:

    void resizeEvent(QResizeEvent*) override;
    void adjustTextToLabel();

    // Disabled methods from QLabel
    QString text() const { return QString(); }; // Use adjustedText() instead.
    void setText(const QString&) {};            // Use setAdjustedText(text) instead.
    void clear() {};                            // Use setdjustedText(QString()) instead.

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------------

/** A widget to chosse a single local file or path.
 *  Use line edit and file dialog properties to customize operation modes.
 */
class  RFileSelector : public RHBox
{
    Q_OBJECT

public:

    explicit RFileSelector(QWidget* const parent=0);
    ~RFileSelector() override;

    QLineEdit* lineEdit()   const;
    
    void setFileDlgMode(QFileDialog::FileMode mode);
    void setFileDlgFilter(const QString& filter);
    void setFileDlgTitle(const QString& title);
    void setFileDlgOptions(QFileDialog::Options opts);

Q_SIGNALS:

    void signalOpenFileDialog();
    void signalUrlSelected(const QUrl&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------------------------

/** A widget to draw progress wheel indicator over thumbnails.
 */
class  WorkingPixmap
{
public:

    explicit WorkingPixmap();
    ~WorkingPixmap();

    bool    isEmpty()          const;
    QSize   frameSize()        const;
    int     frameCount()       const;
    QPixmap frameAt(int index) const;

private:

    QVector<QPixmap> m_frames;
};

// ------------------------------------------------------------------------------------

/** A widget to chosse a color from a palette.
 */
class  RColorSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit RColorSelector(QWidget* const parent=0);
    ~RColorSelector() override;
    
    void setColor(const QColor& color);
    QColor color() const;

Q_SIGNALS:

    void signalColorSelected(const QColor&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    void paintEvent(QPaintEvent*) override;
    
private:

    class Private;
    Private* const d;
};

} // namespace KDcrawIface

#endif // RWIDGETUTILS_H
