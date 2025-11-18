#ifndef QSOBJECTCREATORCPP_H
#define QSOBJECTCREATORCPP_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>
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

private:
    QQmlEngine* m_engine;
};

#endif // QSOBJECTCREATORCPP_H
