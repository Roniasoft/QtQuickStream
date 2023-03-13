#ifndef QSOBJECTCPP_H
#define QSOBJECTCPP_H

#include <QObject>
#include <QUuid>
#include <qqml.h>

class QSRepositoryCpp;

/*! ***********************************************************************************************
 * QSSObject provides UUID and (JSON) serialization functionality.
 *
 * \note    Starting interface types with I_ will prevent serialization of properties in subclasses
 *
 * \warning DO NOT CHANGE the UUID after repo is set
 * \warning DO NOT USE QT's list<SomeType> as it will not deserialize
 * \warning (Private) attributes starting with _ will not be de/serialized
 *
 * \todo Add mutex locking / thread safety
 * ************************************************************************************************/
class QSObjectCpp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString              _qsInterfaceType  READ getInterfaceType                      CONSTANT)
    Q_PROPERTY(QObject*             _qsParent         READ parent            WRITE setParent)
    Q_PROPERTY(QSRepositoryCpp*     _qsRepo           READ getRepo           WRITE setRepo       NOTIFY repoChanged)
    Q_PROPERTY(QString              _qsUuid           READ getUuidStr        WRITE setUuidStr    NOTIFY uuidChanged)
    Q_PROPERTY(bool                 qsIsAvailable     MEMBER m_isAvailable                       NOTIFY isAvailableChanged)
    Q_PROPERTY(QString              qsType            READ getType                               CONSTANT)
    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit QSObjectCpp(QObject *parent = nullptr);
    virtual ~QSObjectCpp();

    /* Public Getters
     * ****************************************************************************************/
    const QMetaObject   *getInterfaceMetaObject();
    QString             getInterfaceType();

    bool                getIsAvailable() const;
    QSRepositoryCpp    *getRepo() const;
    QString             getType();
    QUuid               getUuid() const;
    QString             getUuidStr() const;

    //! \todo this should not be public
    void                setIsAvailable(bool available);

public slots:
    /* Public Slots
     * ****************************************************************************************/
    QVariantList        getInterfacePropNames();

signals:
    /* Signals
     * ****************************************************************************************/
    void                isAvailableChanged();
    void                repoChanged();
    void                uuidChanged();

private:
    /* Private Setters
     * ****************************************************************************************/
    void                deriveTypes ();
    bool                setRepo     (QSRepositoryCpp *repo);
    bool                setUuidStr  (const QString &uuid);

    /* Private Functions
     * ****************************************************************************************/
    static QUuid        createUuid();

    /* Attributes
     * ****************************************************************************************/
    const QMetaObject   *m_interfaceMetaObject;
    QString             m_interfaceType;
    bool                m_isAvailable;
    QSRepositoryCpp    *m_repo;
    QString             m_type;
    QUuid               m_uuid;

    static QVariantMap  m_qsInterfacePropNamesCache;
};

#endif // QSOBJECTCPP_H
