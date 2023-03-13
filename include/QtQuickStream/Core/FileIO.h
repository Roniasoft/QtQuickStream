#ifndef FILEIO_H
#define FILEIO_H

#include <QFile>
#include <QTextStream>
#include <qqml.h>

/*! ***********************************************************************************************
 * FileIO provides file reading/wiring functionality to QML
 * ************************************************************************************************/
class FileIO : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /* Public Constructors & Destructor
     * ****************************************************************************************/
public:
    explicit FileIO(QObject *parent = nullptr) : QObject(parent) {}

    /* Public Slots
     * ****************************************************************************************/
public slots:
    //! Writes data to file with fileName and returns whether successful
    bool write(const QString &fileName, const QByteArray &data)
    {
        if (fileName.isEmpty())                             { return false; }

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) { return false; }

        // File is closed automatically when if goes out of scope
        return file.write(data);
    }

    //! Writes data to file with fileUrl and returns whether successful
    bool write(const QUrl &fileUrl, const QByteArray &data) {
        return write(fileUrl.toLocalFile(), data);
    }

    //! Reads data from file with fileName, empty if failed
    QByteArray read(const QString &fileName)
    {
        if (fileName.isEmpty())                             { return ""; }

        QFile file(fileName);
        if (!file.open(QFile::ReadOnly))                    { return ""; }

        // File is closed automatically when if goes out of scope
        return file.readAll();
    }

    //! Reads data from file with fileUrl, empty if failed
    QByteArray read(const QUrl &fileUrl)
    {
        return read(fileUrl.toLocalFile());
    }
};

#endif // FILEIO_H
