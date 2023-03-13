#ifndef QSSREPOSITORYCPP_H
#define QSSREPOSITORYCPP_H

#include <QObject>
#include <QUuid>
#include <qqml.h>

#include "QSObjectCpp.h"

/*! ***********************************************************************************************
 * QSRepositoryCpp is the container that stores and manages QSObjects. It can be de/serialized
 * from/to the disk, and will enable enable other mechanisms in the future (e.g., RPCs, etc.).
 * ************************************************************************************************/

class QSRepositoryCpp : public QSObjectCpp
{
    Q_OBJECT

    /* Properties
     * ****************************************************************************************/
    Q_PROPERTY(QSObjectCpp   *qsRootObject   MEMBER  m_rootObject        NOTIFY rootObjectChanged)

    Q_PROPERTY(QVariantList  _addedObjects   MEMBER  m_addedObjects      NOTIFY addedObjectsChanged)
    Q_PROPERTY(QVariantList  _deletedObjects MEMBER  m_deletedObjects    NOTIFY deletedObjectsChanged)
    Q_PROPERTY(QVariantList  _forwardedRepos MEMBER  m_forwardedRepos    NOTIFY forwardedReposChanged)
    Q_PROPERTY(QVariantList  _updatedObjects MEMBER  m_updatedObjects    NOTIFY updatedObjectsChanged)

    Q_PROPERTY(QVariantMap   _qsObjects      MEMBER  m_objects           NOTIFY objectsChanged)

    Q_PROPERTY(bool          _isLoading      MEMBER  m_isLoading         NOTIFY isLoadingChanged)

    Q_PROPERTY(QString       name            MEMBER  m_name              NOTIFY nameChanged)

    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit QSRepositoryCpp(QObject *parent = nullptr);
    virtual ~QSRepositoryCpp();

public slots:
    /* Public Slots
     * ****************************************************************************************/
    bool forwardRepo     (QSRepositoryCpp *qsRepository);
    bool unforwardRepo   (QSRepositoryCpp *qsRepository);

    bool registerObject  (QSObjectCpp *qsObject);
    bool unregisterObject(QSObjectCpp *qsObject);

signals:
    /* Signals
     * ****************************************************************************************/
    void addedObjectsChanged();
    void deletedObjectsChanged();
    void forwardedReposChanged();
    void updatedObjectsChanged();

    void objectAdded     (QSObjectCpp *qsObject);
    void objectDeleted   (const QString &uuidStr);

    void isLoadingChanged();
    void nameChanged();
    void objectsChanged();
    void rootObjectChanged();

    // RPC incoming
    void messageReceived (const QString &sourceId, const QByteArray &msg);
    void sendMessage     (const QVariantList &targetIds, const QByteArray &msg);
    void sendMessageToAll(const QByteArray &msg);

protected slots:
    /* Protected Slots
     * ****************************************************************************************/
    bool addObject   (const QString &uuidStr, QSObjectCpp *qsObject,
                      bool force = false);
    bool clearObjects();
    bool delObject   (const QString &uuidStr, bool suppressSignal = false);

    void onIsAvailableChanged();
    void onObjectChanged();

private:
    /* Private Functions
     * ****************************************************************************************/
    void observeObject  (QSObjectCpp *qsObject);
    void unobserveObject(QSObjectCpp *qsObject);

    /* Attributes
     * ****************************************************************************************/
    QVariantList        m_addedObjects;
    QVariantList        m_deletedObjects;
    QVariantList        m_forwardedRepos;
    QVariantList        m_updatedObjects;

    bool                m_isLoading;

    QString             m_name;

    QVariantMap         m_objects;

    QSObjectCpp        *m_rootObject;
};

#endif // QSREPOSITORYCPP_H
