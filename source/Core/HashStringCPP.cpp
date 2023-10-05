#include "HashStringCPP.h"

#include <QCryptographicHash>


/* ************************************************************************************************
 * Public Constructors & Destructor
 * ************************************************************************************************/

/*! Default constructor
 * ************************************************************************************************/
HashStringCPP::HashStringCPP(QObject *parent)
    : QObject{parent}
{

}


/* ************************************************************************************************
 * Public Functions
 * ************************************************************************************************/

/*!
 * Hash a string with Md5.
 *
 * \param str is string that be hash.
 */
QString HashStringCPP::hashString(QString str)
{
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5;
    QByteArray hashedStr   = QCryptographicHash::hash(str.toStdString(),   algorithm);

    return hashedStr;
}

/*!
 * Compare two string models.
 *
 * \param strModelFirst is the first string model.
 * \param strModelSecound is the secound string model.
 */
bool HashStringCPP::compareStringModels(QString strModelFirst, QString strModelSecound)
{
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5;
    QByteArray modelFirst   = QCryptographicHash::hash(strModelFirst.toStdString(),   algorithm);
    QByteArray modelSecound = QCryptographicHash::hash(strModelSecound.toStdString(), algorithm);

    return (modelFirst.compare(modelSecound) == 0);
}
