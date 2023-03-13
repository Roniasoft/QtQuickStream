#include "QSCoreCpp.h"

#include <QFile>
#include <QRandomGenerator>
#include <QSysInfo>

/* ************************************************************************************************
 * Public Constructors & Destructor
 * ************************************************************************************************/
/*! Default constructor
 * ************************************************************************************************/
QSCoreCpp::QSCoreCpp(QObject *parent)
  : QObject         {parent}
  , m_defaultRepo   (nullptr)
  , m_qsRepos      ()
{
    // Attempt to read ID information from Disk
    {
        QFile cfgFile("QSCore.cfg");

        if (cfgFile.open(QFile::ReadOnly)) {
            m_coreId = QUuid(cfgFile.readAll());
            cfgFile.close();
        }
    }

    // Generate ID information and safe to Disk
    if (m_coreId.isNull()) {
        QByteArray systemId = QSysInfo::machineUniqueId();

        // Generate info
        m_coreId =
            QUuid(
                *reinterpret_cast<uint*>(systemId.data()),              // System ID
                26567 + QRandomGenerator::global()->generate() % 1000,  // Port
                0,                                                      // Repo ID
                0, 0, 0, 0, 0, 0, 0, 0                                  // Object ID
            );

        QFile cfgFile("QSCore.cfg");

        if (cfgFile.open(QFile::WriteOnly)) {
            cfgFile.write(m_coreId.toString().toLatin1());
            cfgFile.close();
        }
    }
}

/* ************************************************************************************************
 * Public Getters
 * ************************************************************************************************/
QString QSCoreCpp::getCoreIdStr() const
{
    return m_coreId.toString();
}

QSRepositoryCpp *QSCoreCpp::getDefaultRepo() const
{
    return m_defaultRepo;
}

/*! Routes the message from the repository to whereever it needs to go.
 * ************************************************************************************************/
void QSCoreCpp::onRepoMessage(const QVariantList &targetIds, const QByteArray &msg)
{
    QSRepositoryCpp *repo = qobject_cast<QSRepositoryCpp*>(QObject::sender());

    // Sanity check
    if (repo == nullptr) { return; }


}

void QSCoreCpp::onRepoMessageToAll(const QByteArray &msg)
{
}

/*! Adds a Repository to the list of locally known repos
 * ************************************************************************************************/
void QSCoreCpp::addRepo(QSRepositoryCpp *repo)
{
    // Sanity check
    if (repo == nullptr || m_qsRepos.contains(repo->getUuidStr())) { return; }

    // Add repo
    m_qsRepos[repo->getUuid().toString()] = QVariant::fromValue(repo);
    qInfo() << "[QSCoreCpp] Added new repo" << repo->getUuidStr();

    // Inform observers
    emit qsReposChanged();
}

/* ************************************************************************************************
 * Private Setters
 * ************************************************************************************************/
/*! Sets the default repo (the primary one to be used by the application)
 * ************************************************************************************************/
void QSCoreCpp::setDefaultRepo(QSRepositoryCpp *repo)
{
    // Sanity check
    if (m_defaultRepo == repo) { return; }

    qDebug() << "[QSCoreCpp] Setting default repo";

    // Assign
    m_defaultRepo = repo;

    // Inform observers
    emit defaultRepoChanged();
}
