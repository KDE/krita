// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef KIS_SPLASH_PREVIEW_H
#define KIS_SPLASH_PREVIEW_H
#include <QPixmap>
#include <QWidget>

class KisSplashPreview : public QWidget
{
    Q_OBJECT
public:
    explicit KisSplashPreview(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
};

#endif
