/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISMEDIAENCODERPREFERENCESDIALOG
#define KISMEDIAENCODERPREFERENCESDIALOG

#include <QDialog>
#include <QVariantMap>

#include <kritaui_export.h>

class KisMediaEncoderFormat;

class KRITAUI_EXPORT KisMediaEncoderPreferencesDialog : public QDialog
{
public:
    explicit KisMediaEncoderPreferencesDialog(KisMediaEncoderFormat *format,
                                              const QVariantMap &preferences,
                                              QWidget *parent = nullptr);

    QVariant preferences() const;

private Q_SLOTS:
    void slotReset();

private:
    KisMediaEncoderFormat *m_format;
    QWidget *m_widget;
};

#endif
