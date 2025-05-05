#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCompleter.h>
#include <QFileSystemModel.h>
#include "FileProcessor.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startProcessing(const QStringList& files, quint64 xorKey,bool delete_after_use);

private slots:
    void onApplyClicked();;

private:
    Ui::MainWindow* ui;
    QFileSystemModel* dirModel_;
    QCompleter* dirCompleter_;
    QFileSystemModel *saveDirModel_;
    QCompleter *saveDirCompleter_;
    QThread *workerThread_;
    FileProcessor *fileProcessor_;


    quint64 getXorKeyFromUser();
    QStringList findFilesByMask(const QString &dirPath, const QString &mask);

    void onProcessingFinished();
    void onProgressChanged(int value);
    void onErrorOccurred(const QString &message);
    void unlockInterface();
};
#endif // MAINWINDOW_H
