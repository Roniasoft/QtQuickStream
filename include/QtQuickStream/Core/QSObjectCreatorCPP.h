#ifndef QSOBJECTCREATORCPP_H
#define QSOBJECTCREATORCPP_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QStringList>

class QSObjectCreatorCPP : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(QSObjectCreatorCPP)
    QML_SINGLETON

public:
    explicit QSObjectCreatorCPP(QObject *parent = nullptr);

    Q_INVOKABLE QObject *createQmlObject(const QString& qsType,
                                             const QStringList& imports = {"QtQuickStream"},
                                             QObject* parent = nullptr,
                                             QMap<QString, QVariant> additionalProperties = {},
                                             QVariant callerContext = QVariant());
    Q_INVOKABLE QVariantList createQmlObjects(int count, const QString& qsType,
                                         const QStringList& imports = {"QtQuickStream"},
                                         QObject* parent = nullptr,
                                         QMap<QString, QVariant> additionalProperties = {},
                                         QVariant callerContext = QVariant());
    Q_INVOKABLE void createQmlObjectsAsync(
        int count,
        const QString &qsType,
        const QStringList &imports,
        QObject *parentObj,
        QMap<QString, QVariant> additionalProperties,
        QVariant callerContext = {}
        );
    QVariantList createQmlObjectsAsync(int count, ...);

private:
    QQmlEngine* m_engine;

    class QSObjectIncubator : public QQmlIncubator {
    public:
        QSObjectIncubator(QSObjectCreatorCPP *owner,
                          const QString &type,
                          QVariantList *resultList,
                          QObject *actualParent,
                          QMap<QString, QVariant> props,
                          QObject *qsParent,
                          int index);

    protected:
        void statusChanged(Status status) override;

    private:
        QSObjectCreatorCPP *m_owner;
        QVariantList* m_result;
        QPointer<QObject> m_parent;
        QMap<QString, QVariant> m_props;
        QPointer<QObject> m_qsParent;
        int m_index;
        QString m_type;
    };

signals:
    void objectReady(QObject *obj, const QString &qsType);
    void objectError(const QList<QQmlError> &errors, const QString &qsType);
};

#endif // QSOBJECTCREATORCPP_H
