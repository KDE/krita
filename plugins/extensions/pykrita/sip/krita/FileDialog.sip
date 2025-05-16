class FileDialog : public QDialog /NoDefaultCtors/
{
%TypeHeaderCode
#include "FileDialog.h"
%End

public:
    enum DialogType
    {
        OpenFile,
        OpenFiles,
        OpenDirectory,
        ImportFile,
        ImportFiles,
        ImportDirectory,
        SaveFile
    };

    //explicit FileDialog(QWidget *parent = nullptr, const QString type, const QString &dialogName);
    explicit FileDialog(QWidget *parent = 0, FileDialog::DialogType type = FileDialog::DialogType::OpenFile, const QString &dialogName = QString());
protected Q_SLOTS:
    static QString getOpenFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString(),const QString &selectedFilter = QString(), const QString &dialogName = QString());
    static QStringList getOpenFileNames(QWidget *parent = 0, const QString &caption = QString(), const QString &directory = QString(), QString filter = QString(), const QString &selectedFilter = QString(), const QString &dialogName = QString());
    static QString getExistingDirectory(QWidget *parent = 0, const QString &caption = QString(), const QString &directory = QString(), const QString &dialogName = QString());
    static QString getSaveFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &directory = QString(), QString filter = QString(), const QString &selectedFilter = QString(), const QString &dialogName = QString());

    void setCaption(const QString &caption);
    void setDefaultDir(const QString &defaultDir, bool force);
    void setDirectoryUrl(const QUrl &defaultUri);
    void setImageFilters();
    void setMimeTypeFilters(const QStringList &mimeTypeList, QString defaultMimeType);
    void setNameFilter(const QString &filter);
    void selectNameFilter(const QString &filter);
    QStringList filenames();
    QString filename();
    QString selectedNameFilter() const;
    QString selectedMimeType() const;
public Q_SLOTS:
    void onFilterSelected(const QString &filter);
};
