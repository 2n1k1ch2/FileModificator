#include "FileProcessor.h"
#include <QFile>
#include <QDebug>
#include <QFileInfo>
FileProcessor::FileProcessor(QObject *parent) : QObject(parent) {}

void FileProcessor::applyXor(const QString &filePath, quint64 xor_value) {

    QFileInfo fileInfo(filePath);
    QString tmp = "0x" + QString::number(xor_value, 16).toUpper();

    if (fileInfo.baseName().startsWith("0x", Qt::CaseInsensitive)) {
        qDebug() << "Skipping file with prefix 0x: " << filePath;
        return;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open file: " + filePath);
        return;
    }


    QString newFileName = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_" + QString("0x%1").arg(xor_value, 0, 16).toUpper() + "." + fileInfo.suffix();
    QFile newFile(newFileName);

    if (!newFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to create new file: " + newFileName);
        return;
    }


    const qint64 chunkSize = 8;
    QByteArray buffer(chunkSize, 0);

    qint64 bytesRead = 0;
    while ((bytesRead = file.read(buffer.data(), chunkSize)) > 0) {
        if (bytesRead == chunkSize) {
            for (qint64 i = 0; i < bytesRead / sizeof(quint64); ++i) {
                quint64* dataPtr = reinterpret_cast<quint64*>(buffer.data() + i * sizeof(quint64));
                *dataPtr ^= xor_value;
            }
        } else {
            for (qint64 i = 0; i < bytesRead; ++i) {
                buffer[i] ^= (xor_value & 0xFF);
            }
        }
        newFile.write(buffer, bytesRead);
    }
    file.close();
    newFile.close();
}

void FileProcessor::processFiles(const QStringList &files, quint64 xor_value, bool delete_after_use) {
    for (int i = 0; i < files.size(); ++i) {
        const QString &inputFile = files[i];

        applyXor(inputFile, xor_value);

        if (delete_after_use) {
            QFile originalFile(inputFile);
            if (!originalFile.remove()) {
                emit errorOccurred(tr("Failed to delete file: %1").arg(inputFile));
            }
        }
        emit progressChanged(i + 1);
    }
    emit finished();
}


