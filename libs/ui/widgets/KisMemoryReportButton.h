/*
 *  SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMEMORYREPORTBUTTON_H
#define KISMEMORYREPORTBUTTON_H

#include <QPushButton>
#include <kritaui_export.h>

class KRITAUI_EXPORT KisMemoryReportButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KisMemoryReportButton(QWidget *parent = 0);


    void setMaximumMemory(qint64 max);

    void setCurrentMemory(qint64 memory);

    void setImageWeight(qint64 memory);

    void paintEvent(QPaintEvent *e) override;

private:
    qint64 m_maxbytes;
    qint64 m_curbytes;
    qint64 m_imgbytes;
};

#endif // KISMEMORYREPORTBUTTON_H
