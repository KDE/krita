#ifndef KOSTARTUPDIALOG_H
#define KOSTARTUPDIALOG_H

#include <QDialog>

/**
 * @brief The KoStartupDialog class shows the file selectioed, custom document
 * widgets and template lists. A bit like it was in KOffice 1.4...
 */
class KoStartupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KoStartupDialog(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // KOSTARTUPDIALOG_H
