#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox.h>
#include <QCompleter.h>
#include <QFileSystemModel.h>
#include <QInputDialog>
#include <QMainWindow>
#include <QThread>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dirModel_(nullptr)
    , dirCompleter_(nullptr)
    , saveDirModel_(nullptr)
    , saveDirCompleter_(nullptr)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    // Автодополнение для filesPathField
    dirCompleter_ = new QCompleter(this);
    dirModel_ = new QFileSystemModel(dirCompleter_);
    dirModel_->setRootPath("");
    dirModel_->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    dirCompleter_->setModel(dirModel_);
    ui->filesPathField->setCompleter(dirCompleter_);

    // Автодополнение для filesSaveField
    saveDirCompleter_ = new QCompleter(this);
    saveDirModel_ = new QFileSystemModel(saveDirCompleter_);
    saveDirModel_->setRootPath("");
    saveDirModel_->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    saveDirCompleter_->setModel(saveDirModel_);
    ui->filesSaveField->setCompleter(saveDirCompleter_);


    ui->progressBar->setVisible(false);

    connect(ui->applyButton, &QPushButton::clicked, this, &MainWindow::onApplyClicked);

    workerThread_ = new QThread(this);
    fileProcessor_ = new FileProcessor();
    fileProcessor_->moveToThread( workerThread_);

    connect(workerThread_, &QThread::finished, fileProcessor_, &QObject::deleteLater);
    connect(this, &MainWindow::startProcessing, fileProcessor_, &FileProcessor::processFiles);
    connect(fileProcessor_, &FileProcessor::progressChanged, this, &MainWindow::onProgressChanged);
    connect(fileProcessor_, &FileProcessor::finished, this, &MainWindow::onProcessingFinished);
    connect(fileProcessor_, &FileProcessor::errorOccurred, this, &MainWindow::onErrorOccurred);

    workerThread_->start();

}

MainWindow::~MainWindow()
{
    workerThread_->quit();
    workerThread_->wait();
    delete ui;
}

QStringList MainWindow::findFilesByMask(const QString &dirPath, const QString &mask) {
    QDir dir(dirPath);


    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << dirPath;
        return QStringList();
    }


    QStringList files = dir.entryList(QStringList() << mask,QDir::Files | QDir::NoDotAndDotDot,QDir::Name);

    for (QString &file : files) {
        file = QDir::toNativeSeparators(dir.absoluteFilePath(file));
    }

    return files;
}
quint64 MainWindow::getXorKeyFromUser() {
    bool ok;
    QString text = QInputDialog::getText(
        this,
        "Enter the XOR key",
        "8-byte value (hex, for example 0xFFFFFFFFFFFFFFFF):",
        QLineEdit::Normal,
        "0x",
        &ok
        );

    if (!ok || text.isEmpty()) {
        return 0;
    }


    return text.toULongLong(&ok, 16);
}

void MainWindow::onApplyClicked() {

    ui->applyButton->setEnabled(false);
    ui->applyButton->setText("Processing...");

    const QString Path = ui->filesPathField->text();
    const QString Mask = ui->filesMaskField->text();


    if (Path.isEmpty()) {
        QMessageBox::warning(this, "Error!", "Field File Path is empty!");
        unlockInterface();
        return;
    }

    if (Mask.isEmpty()) {
        QMessageBox::warning(this, "Error!", "File mask is empty!");
        unlockInterface();
        return;
    }

    if (ui->filesSaveField->text().isEmpty()) {
        ui->filesSaveField->setText(Path);
    }

    QStringList files = findFilesByMask(Path, Mask);
    if (files.isEmpty()) {
        QMessageBox::information(this, "Info", "No files found matching the mask: " + Mask);
        unlockInterface();
        return;
    }

    quint64 XOR_value = getXorKeyFromUser();
    if (XOR_value == 0) {
        unlockInterface();
        return;
    }

    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(files.size());


    bool delete_after_use = ui->deleteOriginalFileBox->isChecked();
    emit startProcessing(files, XOR_value, delete_after_use);

}
void MainWindow::onProgressChanged(int value) {
    ui->progressBar->setValue(value);
}

void MainWindow::unlockInterface() {
    ui->applyButton->setEnabled(true);
    ui->applyButton->setText("Apply");
    ui->progressBar->setVisible(false);
}
void MainWindow::onProcessingFinished() {
    QMessageBox::information(this, "", "Done!");
    unlockInterface();
}

void MainWindow::onErrorOccurred(const QString &message) {
    QMessageBox::warning(this, "Error", message);
    unlockInterface();
}
