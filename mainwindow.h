#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QProcess>
#include <QtGui>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openFile();
    void addMark();
    void removeMark();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
    QProcess proc;
    QStringList src;
    QString path;
    QString bookTitle;
};

#endif // MAINWINDOW_H
