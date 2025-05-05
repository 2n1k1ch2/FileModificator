#pragma once
#include <QObject>
#include <QStringList>

class FileProcessor : public QObject {
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);

public slots:
    void processFiles(const QStringList &files, quint64 xor_value,bool delete_after_use);

signals:
    void progressChanged(int value);
    void finished();
    void errorOccurred(const QString &message);

private:
    void applyXor(const QString &filePath, quint64 xor_value);
};
