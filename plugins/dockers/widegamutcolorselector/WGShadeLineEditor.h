/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGSHADELINEEDITOR_H
#define WGSHADELINEEDITOR_H

#include <WGConfig.h>

#include <KisVisualColorModel.h>
#include <QIcon>
#include <QFrame>
#include <QScopedPointer>

class Ui_WGShadeLineEditor;
class WGShadeSlider;

class WGShadeLineEditor : public QFrame
{
    Q_OBJECT
public:
    explicit WGShadeLineEditor(QWidget *parent = nullptr);
    ~WGShadeLineEditor();

    WGConfig::ShadeLine configuration() const;
    void setConfiguration(const WGConfig::ShadeLine &cfg, int lineIndex);
    QIcon generateIcon(const WGConfig::ShadeLine &cfg);

protected:
    void hideEvent(QHideEvent *event) override;

private Q_SLOTS:
    void slotValueChanged();
    void slotPatchCountChanged(int value);
    void slotSliderModeChanged(bool enabled);

Q_SIGNALS:
    void sigEditorClosed(int lineIndex);

private:
    KisVisualColorModelSP m_model;
    QScopedPointer<Ui_WGShadeLineEditor> m_ui;
    WGShadeSlider *m_previewLine;
    WGShadeSlider *m_iconSlider;
    int m_lineIndex {0};
};

#endif // WGSHADELINEEDITOR_H
