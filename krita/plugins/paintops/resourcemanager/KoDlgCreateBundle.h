#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <kdialog.h>

namespace Ui
{
class WdgDlgCreateBundle;
}

class KoDlgCreateBundle : public KDialog
{
    Q_OBJECT

public:
    explicit KoDlgCreateBundle(QWidget *parent = 0);
    ~KoDlgCreateBundle();

    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;
    QString saveLocation() const;
    QString previewImage() const;

    QStringList selectedBrushes() const { return m_selectedBrushes; }
    QStringList selectedPresets() const { return m_selectedPresets; }
    QStringList selectedGradients() const { return m_selectedGradients; }
    QStringList selectedPatterns() const { return m_selectedPatterns; }
    QStringList selectedPalettes() const { return m_selectedPalettes; }
    QStringList selectedWorkspaces() const { return m_selectedWorkspaces; }

private slots:

    void accept();
    void selectSaveLocation();
    void addSelected();
    void removeSelected();
    void resourceTypeSelected(int idx);
    void getPreviewImage();

private:
    QWidget *m_page;
    Ui::WdgDlgCreateBundle *m_ui;

    QStringList m_selectedBrushes;
    QStringList m_selectedPresets;
    QStringList m_selectedGradients;
    QStringList m_selectedPatterns;
    QStringList m_selectedPalettes;
    QStringList m_selectedWorkspaces;

    QString m_previewImage;
};

#endif // KOBUNDLECREATIONWIDGET_H
