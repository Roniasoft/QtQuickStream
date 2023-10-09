#pragma once

#include <QtQmlIntegration>
#include <QObject>

/*! ***********************************************************************************************
 * HashStringCPP hash string and compare two hash string (Md5 method).
 * ************************************************************************************************/
class HashStringCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:

    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit HashStringCPP(QObject *parent = nullptr);

protected slots:
    /* Protected Slots
     * ****************************************************************************************/

    //! Hash a string with Md5
    QString hashString(QString str);

    //! Hash a string with Md5 then hex
    QString hexHashString(QString str);

    //! Compare two string models.
    bool compareStringModels(QString strModelFirst, QString strModelSecound);
};
