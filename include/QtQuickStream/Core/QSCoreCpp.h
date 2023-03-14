#ifndef QSCORECPP_H
#define QSCORECPP_H

#include <QObject>
#include <qqml.h>

#include "QSRepositoryCpp.h"


/*! ***********************************************************************************************
 * QSCoreCpp is the heart of any QtQuickStream application -- it provides access to the defaultRepo
 * (local) repos.
 *
 * \todo Extend documentation
 * ************************************************************************************************/
class QSCoreCpp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString          coreId      READ    getCoreIdStr                            CONSTANT)
    Q_PROPERTY(QSRepositoryCpp* defaultRepo READ    getDefaultRepo  WRITE   setDefaultRepo  NOTIFY defaultRepoChanged)
    Q_PROPERTY(QVariantMap      qsRepos    MEMBER   m_qsRepos                              NOTIFY qsReposChanged)
    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit QSCoreCpp(QObject *parent = nullptr);

    /* Public Getters
     * ****************************************************************************************/
    QString             getCoreIdStr() const;
    QSRepositoryCpp    *getDefaultRepo() const;

signals:
    /* Signals
     * ****************************************************************************************/
    void                defaultRepoChanged();
    void                qsReposChanged();

    void                sigCreateRepo       (const QString &repoId, bool isRemote = false);

protected slots:
    /* Protected Slots
     * ****************************************************************************************/
    void                onRepoMessage       (const QVariantList &targetIds, const QByteArray &msg);
    void                onRepoMessageToAll  (const QByteArray &msg);

    void                addRepo             (QSRepositoryCpp *repo);

private:
    /* Private Setters
     * ****************************************************************************************/
    void                setDefaultRepo      (QSRepositoryCpp* repo);

    /* Attributes
     * ****************************************************************************************/
    QUuid               m_coreId;
    QSRepositoryCpp    *m_defaultRepo;
    QVariantMap         m_qsRepos;
};

#endif // QSCORECPP_H
