#include "QSObjectCpp.h"
#include "QSRepositoryCpp.h"

#include <QMetaObject>
#include <QSysInfo>

QVariantMap QSObjectCpp::m_qsInterfacePropNamesCache;

/* ************************************************************************************************
 * Public Constructors & Destructor
 * ************************************************************************************************/
/*! Default constructor
 * ************************************************************************************************/
QSObjectCpp::QSObjectCpp(QObject *parent)
  : QObject                 {parent}
  , m_interfaceMetaObject   (nullptr)
  , m_interfaceType         ()
  , m_isAvailable           (true)
  , m_repo                  (nullptr)
  , m_uuid                  (QSObjectCpp::createUuid())
  , m_type                  ()
{
    // Set repo autmatically from parent
    if (QSObjectCpp *qsObjParent = qobject_cast<QSObjectCpp*>(parent)) {
        m_repo = qsObjParent->m_repo;
    }
    // Set repo automatically if parent is a repo
    else if (QSRepositoryCpp *qsRepoParent = qobject_cast<QSRepositoryCpp*>(parent)) {
        m_repo = qsRepoParent;
    }

    // Register if repo set
    if (m_repo != nullptr) {
        m_repo->registerObject(this);
    }
}

/*! Fall-through virtual descructor
 * ************************************************************************************************/
QSObjectCpp::~QSObjectCpp()
{
    // Fall-through
}

/* ************************************************************************************************
 * Public Getters
 * ************************************************************************************************/
/*! Returns the most specific superclass that whos name starts with I_
 * ************************************************************************************************/
const QMetaObject *QSObjectCpp::getInterfaceMetaObject()
{
    // Return cached version (note m_type is checked as m_interfaceType can legitimately be empty)
    if (m_type.isNull()) { deriveTypes(); }

    return m_interfaceMetaObject;
}

QString QSObjectCpp::getInterfaceType()
{
    // Return cached version (note m_type is checked as m_interfaceType can legitimately be empty)
    if (m_type.isNull()) { deriveTypes(); }

    return m_interfaceType;
}

/*! Returns whether the object is available
 * ************************************************************************************************/
bool QSObjectCpp::getIsAvailable() const
{
    return m_isAvailable;
}

/*! Returns the repository
 * ************************************************************************************************/
QSRepositoryCpp *QSObjectCpp::getRepo() const
{
    return m_repo;
}

/*! Returns the type of the object, which can be instantiated using QML: Qt.createQmlObject()
 * ************************************************************************************************/
QString QSObjectCpp::getType()
{
    if (m_type.isNull()) { deriveTypes(); }

    return m_type;
}

/*! Returns the UUID
 * ************************************************************************************************/
QUuid QSObjectCpp::getUuid() const
{
    return m_uuid;
}

/*! Returns the UUID in string format (for JS/QML use)
 * ************************************************************************************************/
QString QSObjectCpp::getUuidStr() const
{
    return m_uuid.toString();
}

/*! Sets the availavility of the object
 *  \todo this should not be public
 * ************************************************************************************************/
void QSObjectCpp::setIsAvailable(bool available)
{
    // Sanity check
    if (m_isAvailable == available) { return; }

    m_isAvailable = available;
    emit isAvailableChanged();
}

/* ************************************************************************************************
 * Public Slots
 * ************************************************************************************************/
/*! Returns the property names belonging to the interface of this object or an empty list if no
 *  interface is set. Caches the properties for future use.
 *
 *  \todo Consider whether this should be cached in a property, to reduce QML<->C++ memcopies
 *  \todo Consider whether this could & should cache properties for regular types as well
 * ************************************************************************************************/
QVariantList QSObjectCpp::getInterfacePropNames()
{
    // Sanity check
    if (getInterfaceMetaObject() == nullptr) { return QVariantList(); }

    // Cache if this interface is missing
    if (!m_qsInterfacePropNamesCache.contains(getInterfaceType())) {
        const QMetaObject *metaObject = getInterfaceMetaObject();

        // List all properties of this interface
        QVariantList ifaceProps;
        for(int i = 0; i < metaObject->propertyCount(); ++i) {
            ifaceProps.append(metaObject->property(i).name());
        }

        // Store it for future use
        m_qsInterfacePropNamesCache[getInterfaceType()] = ifaceProps;
    }

    return m_qsInterfacePropNamesCache[getInterfaceType()].toList();
}

/* ************************************************************************************************
 * Private Setters
 * ************************************************************************************************/
/*! Finds and caches the type and interfaceType
 * ************************************************************************************************/
void QSObjectCpp::deriveTypes()
{
    // Sanity check: abort if already cached
    if (!m_type.isNull()) { return; }

    /* Finds actual type
     * ****************************************************************************************/
    QString className = metaObject()->className();

    int qmlTypeIdx = className.indexOf("_QMLTYPE");

    // Return chopped type if it's a QML type
    m_type = (qmlTypeIdx != -1) ? className.first(qmlTypeIdx) : className;

    /* Finds interface type
     * ****************************************************************************************/
    // Search for most specific superclass with _qsInterface property
    const QMetaObject* metaObject = this->metaObject();
    while (metaObject != nullptr) {
        if (strncmp(metaObject->className(), "I_", 2) == 0) {
            m_interfaceMetaObject = metaObject;

            className = metaObject->className();

            qmlTypeIdx = className.indexOf("_QMLTYPE");

            // Return chopped type if it's a QML type
            m_interfaceType = (qmlTypeIdx != -1) ? className.first(qmlTypeIdx) : className;

            break;
        }

        metaObject = metaObject->superClass();
    }
}

/*! Sets the Repo and registers object (recurses children) -- For QtQuickStream internal use only!
 * ************************************************************************************************/
bool QSObjectCpp::setRepo(QSRepositoryCpp *newRepo)
{
    // Sanity check: skip if same repo as before
    if (m_repo == newRepo) { return true; }

    // Unregister form old repo
    if (m_repo != nullptr) {
        m_repo->unregisterObject(this);
    }

    // Automatically register with new Repo
    if (newRepo != nullptr) {
        // Overwrite first part of UUID if not set
        if (m_uuid.data1 == 0 && m_uuid.data2 == 0) {
            m_uuid.data1 = newRepo->m_uuid.data1;
            m_uuid.data2 = newRepo->m_uuid.data2;
        }

        // Return false if registration failed
        if (!newRepo->registerObject(this)) {
            return false;
        }
    }

    // Update admnistration
    m_repo = newRepo;
    emit repoChanged();

    // Do the same for all children
    for (QObject *child : children()) {
        if (QSObjectCpp *qsChild = qobject_cast<QSObjectCpp*>(child)) {
            qsChild->setRepo(newRepo);
        }
    }

    return true;
}

/*! Sets the UUID (only if not registered with repo) -- For QtQuickStream internal use only!
 * ************************************************************************************************/
bool QSObjectCpp::setUuidStr(const QString &uuid)
{
    // Refuse to change if object registered with repo
    if (m_uuid.data1 != 0 && m_repo != nullptr) {
        qWarning() << "Cannot change UUID of Registered object "
                   << getType() << objectName() << m_uuid.toString();
        return false;
    }

    m_uuid = QUuid::fromString(uuid);
    emit uuidChanged();

    return true;
}

/* ************************************************************************************************
 * Private Functions
 * ************************************************************************************************/
QUuid QSObjectCpp::createUuid()
{
    QUuid id = QUuid::createUuid();

    // These will be assigned by repo
    id.data1 = 0;
    id.data2 = 0;
    id.data3 = 0;

    return id;
}
