#ifndef KOFILEDIALOGTESTER_H
#define KOFILEDIALOGTESTER_H

#include <QWidget>

namespace Ui {
class KoFileDialogTester;
}

class KoFileDialogTester : public QWidget
{
    Q_OBJECT
    
public:
    explicit KoFileDialogTester(QWidget *parent = 0);
    ~KoFileDialogTester();
    
private slots:

    void testOpenFile();
    void testOpenFiles();
    void testOpenDirectory();
    void testOpenDirectories();
    void testImportFile();
    void testImportFiles();
    void testImportDirectory();
    void testImportDirectories();
    void testSaveFile();
    void testSaveFiles();

private:
    Ui::KoFileDialogTester *ui;
};

#endif // KOFILEDIALOGTESTER_H
