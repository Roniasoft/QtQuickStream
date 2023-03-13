#include "QSRepositoryCpp.h"
#include "QSObjectCpp.h"

#include <QMetaObject>
#include <QMetaMethod>

/* ************************************************************************************************
 * Public Constructors & Destructor
 * ************************************************************************************************/
/*! Default constructor
 * ************************************************************************************************/
QSRepositoryCpp::QSRepositoryCpp(QObject *parent)
  : QSObjectCpp    {parent}
  , m_addedObjects  ()
  , m_deletedObjects()
  , m_forwardedRepos()
  , m_updatedObjects()
  , m_isLoading     (false)
  , m_name          ("Repo")
  , m_objects       ()
  , m_rootObject    (nullptr)
{
    // Propagate availability changes to qsobjects
    connect(this, &QSObjectCpp::isAvailableChanged, this, &QSRepositoryCpp::onIsAvailableChanged);
}

/*! Fall-through virtual descructor
 * ************************************************************************************************/
QSRepositoryCpp::~QSRepositoryCpp()
{
    // Fall-through
}

/* ************************************************************************************************
 * Public Slots
 * ************************************************************************************************/
/*! Adds all objects of qsRepository to local repo in order to forward them, and subscribes to
 *  their changes.
 * ************************************************************************************************/
bool QSRepositoryCpp::forwardRepo(QSRepositoryCpp *qsRepository)
{
    // Sanity check
    if (qsRepository == nullptr) {
        return false;
    }

    // Sanity check: Abort if already forwarded
    if (m_forwardedRepos.contains(QVariant::fromValue(qsRepository))) {
        return false;
    }

    // Add all existing objects
    for (const QVariant &qsObjectVar : qsRepository->m_objects) {
        if (QSObjectCpp *qsObject = qsObjectVar.value<QSObjectCpp*>()) {
            addObject(qsObject->getUuidStr(), qsObject, true);
        }
    }

    // Subscribe to insert added objects
    connect(qsRepository, &QSRepositoryCpp::objectAdded, this, [=](QSObjectCpp *addedObject) {
        // Sanity check
        if (addedObject == nullptr) {
            qWarning() << "Attempted to add nullptr object. Skipping!";
            return;
        }

        addObject(addedObject->getUuidStr(), addedObject, true);
    });

    // Subscribe to remove deleted objects
    connect(qsRepository, &QSRepositoryCpp::objectDeleted, this, [=](const QString &deletedId) {
        delObject(deletedId);
    });

    qDebug() << "Forwarding Repo: " << qsRepository->getUuidStr();

    m_forwardedRepos.append(QVariant::fromValue(qsRepository));
    emit forwardedReposChanged();

    return true;
}

/*! Removes all forwarded objects of qsRepository, and unsubscribes from changes.
 * ************************************************************************************************/
bool QSRepositoryCpp::unforwardRepo(QSRepositoryCpp *qsRepository)
{
    // Sanity check
    if (qsRepository == nullptr) {
        return false;
    }

    // Sanity check: Abort if not forwarded
    if (!m_forwardedRepos.contains(QVariant::fromValue(qsRepository))) {
        return false;
    }

    // Unsubscribe everything
    disconnect(qsRepository, nullptr, this, nullptr);

    // Remove all objects of repo
    for (const QVariant &qsObjectVar : qsRepository->m_objects) {
        if (QSObjectCpp *qsObject = qsObjectVar.value<QSObjectCpp*>()) {
            delObject(qsObject->getUuidStr());
        }
    }

    qDebug() << "Stopped Forwarding Repo: " << qsRepository->getUuidStr();
    m_forwardedRepos.removeAll(QVariant::fromValue(qsRepository));
    emit forwardedReposChanged();

    return true;
}

/*! Registers an object with the repository
 * ************************************************************************************************/
bool QSRepositoryCpp::registerObject(QSObjectCpp *qsObject)
{
    // Sanity check
    if (qsObject == nullptr) {
        return false;
    }

    // If a file is being read, ignore any created objects
    if (m_isLoading) {
//        qDebug() << "Ignoring newly created object: " << qsObject->objectName();
        return true;
    }

    addObject(qsObject->getUuidStr(), qsObject);

//    qDebug() << "Registered an object with UUID:" << qsObject->getUuidStr();

    return true;
}

/*! Removes the object form the repository
 * ************************************************************************************************/
bool QSRepositoryCpp::unregisterObject(QSObjectCpp *qsObject)
{
    // Sanity check
    if (qsObject == nullptr) {
        return true;
    }

    delObject(qsObject->getUuidStr());

//    qDebug() << "Unregistered an object with UUID:" << qsObject->getUuidStr();

    return true;
}

/* ************************************************************************************************
 * Protected Slots
 * ************************************************************************************************/
/*! Adds an QSObject to the repository
 * ************************************************************************************************/
bool QSRepositoryCpp::addObject(const QString &uuidStr, QSObjectCpp *qsObject, bool force)
{
    // Sanity check: skip if already added
    if (m_objects.contains(uuidStr)) {
        // If forced, unobserve old object
        if (force) {
            unobserveObject(m_objects[uuidStr].value<QSObjectCpp*>());
        } else {
            qWarning() << "Skipped adding object" << uuidStr << "-- uuid already registered!";
            return false;
        }
    }

    // Remove from deleted objects if rquired
    m_deletedObjects.removeAll(uuidStr);

    // Add to local administration
    m_objects[uuidStr] = QVariant::fromValue(qsObject);

    // Start listening to changes on object (local only)
    observeObject(qsObject);

    emit objectAdded(qsObject);

    if (!m_isLoading) {
        emit objectsChanged();
    }

    // Record added object
    if (!m_addedObjects.contains(QVariant::fromValue(qsObject))) {
        m_addedObjects.append(QVariant::fromValue(qsObject));

        if (!m_isLoading) {
            emit addedObjectsChanged();
        }
    }

    return true;
}

/*! Removes all QSObjects from the repository
 * ************************************************************************************************/
bool QSRepositoryCpp::clearObjects()
{
    // Sanity check: skip if nothing to be done
    if (m_objects.empty()) { return false; }

    bool isChanged = false;

    // Remove all
    while (!m_objects.empty()) {
        // Make a copy of key to prevent deletion of temporary
        const QString key = m_objects.firstKey();
        isChanged |= delObject(key, true);
    }

    if (!m_isLoading) {
        emit objectsChanged();
        emit deletedObjectsChanged();
    }

    return isChanged;
}

/*! Removes an QSObject from the repository (by UUID)
 * ************************************************************************************************/
bool QSRepositoryCpp::delObject(const QString &uuidStr, bool suppressSignal)
{
    // Sanity check: skip if already added
    if (!m_objects.contains(uuidStr)) { return false; }

    // Remove the objet and disconnect all signals if we can find the object
    if (QSObjectCpp *qsObject = m_objects.take(uuidStr).value<QSObjectCpp*>()) {
        unobserveObject(qsObject);

        // Remove from pending changes
        // \note No signals emitted as the system should alreay have been triggered when added/updated
        m_addedObjects.removeAll(QVariant::fromValue(qsObject));
        m_updatedObjects.removeAll(QVariant::fromValue(qsObject));

        emit objectDeleted(qsObject->getUuidStr());
    }

    if (!(m_isLoading || suppressSignal)) {
        emit objectsChanged();
    }

    // Record deleted uuid
    if (!m_deletedObjects.contains(uuidStr)) {
        m_deletedObjects.append(uuidStr);

        if (!(m_isLoading || suppressSignal)) {
            emit deletedObjectsChanged();
        }
    }

    return true;
}

/*! Makes all QSObjects in the repository unavailable when repo becomes unavailable. A re-sync of
 *  object availability will need to make these available again -- that way we prevent recently
 *  removed objects from appearing available after a reconnect, while they are not.
 * ************************************************************************************************/
void QSRepositoryCpp::onIsAvailableChanged()
{
    if (!getIsAvailable()) {
        for (QVariant &var : m_objects) {
            if (QSObjectCpp *qsObject = var.value<QSObjectCpp*>()) {
                qsObject->setIsAvailable(false);
            }
        }
    }
}

void QSRepositoryCpp::onObjectChanged()
{
    // Store reference to updated object
    if (!m_updatedObjects.contains(QVariant::fromValue(sender()))) {
//        qDebug() << sender()->metaObject()->method(senderSignalIndex()).name();

        m_updatedObjects.append(QVariant::fromValue(sender()));
        emit updatedObjectsChanged();
    }
}

/* Private Functions
 * ************************************************************************************************/
/*! Connects to all object Changed() signals, exluding 'private' properties starting with _ and
 *  properties of derived classes if an interface is used
 * ************************************************************************************************/
void QSRepositoryCpp::observeObject(QSObjectCpp *qsObject)
{
    // Cache targeted slot: QSRepositoryCpp::onObjectChanged()
    static const QMetaMethod metaOnObjectChanged =
        metaObject()->method(QSRepositoryCpp::staticMetaObject.indexOfSlot("onObjectChanged()"));

    // Connect all Changed() signals of qsObject by iterating over all its superclasses
    const QMetaObject* metaObject = qsObject->metaObject();

    // Limit observation of properties to most specific interface (if applicable)
    int lastMethod = qsObject->getInterfaceMetaObject() != nullptr
                   ? qsObject->getInterfaceMetaObject()->methodCount()
                   : metaObject->methodCount();

    // Observe all propChanged() signals
    for(int i = 0; i < lastMethod; ++i) {
        const QMetaMethod metaMethod = metaObject->method(i);

        if (metaMethod.methodType() == QMetaMethod::Signal
                && !metaMethod.name().startsWith("_")
                && metaMethod.name().endsWith("Changed"))
        {
            connect(qsObject, metaMethod, this, metaOnObjectChanged);
//            qDebug() << "CONNECTED " << metaObject->className() << metaMethod.name();
        }
    }
}

/*! Disconnects all signals between object and this repo
 * ************************************************************************************************/
void QSRepositoryCpp::unobserveObject(QSObjectCpp *qsObject)
{
    disconnect(qsObject, nullptr, this, nullptr);
}
