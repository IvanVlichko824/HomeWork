#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QXmlStreamAttribute>
#include <QXmlStreamReader>
#include <QFile>
#include <QUrl>
#include <QWebView>

//Функция удаления папки
int removeFolder(QDir & dir)
{
  int res = 0;
  //Получаем список каталогов
  QStringList lstDirs = dir.entryList(QDir::Dirs |
                  QDir::AllDirs |
                  QDir::NoDotAndDotDot);
  //Получаем список файлов
  QStringList lstFiles = dir.entryList(QDir::Files);

  //Удаляем файлы
  foreach (QString entry, lstFiles)
  {
   QString entryAbsPath = dir.absolutePath() + "/" + entry;
   QFile::setPermissions(entryAbsPath, QFile::ReadOwner | QFile::WriteOwner);
   QFile::remove(entryAbsPath);
  }

  //Для папок делаем рекурсивный вызов
  foreach (QString entry, lstDirs)
  {
   QString entryAbsPath = dir.absolutePath() + "/" + entry;
   QDir dr(entryAbsPath);
   removeFolder(dr);
  }

  //Удаляем обрабатываемую папку
  if (!QDir().rmdir(dir.absolutePath()))
  {
    res = 1;
  }
  return res;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->action, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->action_3, SIGNAL(triggered()), this, SLOT(addMark()));
    connect(ui->action_4, SIGNAL(triggered()), this, SLOT(removeMark()));
    connect(&proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::currentRowChanged(int currentRow) {
    ui->action_3->setEnabled(true);
    ui->webView->setUrl(QUrl(src[currentRow]));
}

void MainWindow::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) return;

    QFile file("tmp\\META-INF\\container.xml");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка файла", "Файл не найден!", QMessageBox::Ok);
        return;
    }

    QString content_file;
    QXmlStreamReader reader;
    reader.setDevice(&file);
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isStartElement() && reader.name() == "rootfile")
            foreach (const QXmlStreamAttribute &attr, reader.attributes()) // Read the attributes
                if (attr.name().toString() == "full-path")
                    content_file = "tmp\\" + attr.value().toString();
        reader.readNext();
    }
    file.close();

   // Считываем файл с описанием контента
    QFile file1(content_file);
    if (!file1.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка файла", content_file, QMessageBox::Ok);
        return;
    }

   QXmlStreamReader reader1;
   reader1.setDevice(&file1);
   reader1.readNext();

   while (!reader1.atEnd()) {
       if (reader1.isStartElement() && reader1.name() == "title")
           bookTitle = reader1.readElementText();
       reader1.readNext();
    }
    file1.close();

    this->setWindowTitle("EPub reader / " + bookTitle);

    // Определеляем директорию с контентом
    path=content_file.remove(content_file.length()-12, 12); // отнимаем имя файла content.xxx

    // Считываем файл с оглавлением
    QFile file2(path+"\\toc.ncx");
    if (!file2.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка файла", "Файл не найден!", QMessageBox::Ok);
        return;
    }

    src.clear();

  //  while (ui->listWidget->count() > 0) {
    //  delete ui->listWidget->takeItem(0);
 //     QMessageBox::warning(this, "ОK", QString(5), QMessageBox::Ok);
  //  }

    QXmlStreamReader toc;
    toc.setDevice(&file2);
    toc.readNext();
    while (!toc.atEnd()) {
        if (toc.isStartElement() && toc.name() == "navPoint") {
            while (!toc.isStartElement() || toc.name() != "text")
                toc.readNext();
            ui->listWidget->addItem(toc.readElementText());
            while (!toc.isStartElement() || toc.name() != "content")
                toc.readNext();
            foreach (const QXmlStreamAttribute &attr, toc.attributes())
                if (attr.name().toString() == "src")
                    src.append(path+"\\"+attr.value().toString());
        }
       toc.readNext();
    }
    file2.close();
    ui->listWidget->setCurrentRow(0);
}


void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open book"),
                                                     "/home",
                                                     tr("ePub (*.epub)"));
    if (fileName != "")
    {
       QDir d = QDir("tmp");
       removeFolder(d);
       proc.start("\"C:\\unzip.exe\" " + fileName + " -d tmp");
    }
}

void MainWindow::addMark()
{
    //delete ui->listWidget->takeItem(0);
    ui->action_4->setDisabled(false);
    ui->listWidget_2->addItem(bookTitle + " / " + ui->listWidget->currentItem()->text());
}

void MainWindow::removeMark()
{
    delete ui->listWidget_2->currentItem();
    if (ui->listWidget_2->count()==0)
        ui->action_4->setDisabled(true);

}
