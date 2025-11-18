#include "QSObjectCreatorCPP.h"
#include <QQmlContext>

QSObjectCreatorCPP::QSObjectCreatorCPP(QObject *parent)
    : QObject(parent)
    , m_engine(nullptr)
{
}

QObject * QSObjectCreatorCPP::createQmlObject(const QString &qsType, const QStringList &imports, QObject *parent, QMap<QString, QVariant> additionalProperties, QVariant callerContext)
{
    return createQmlObjects(1, qsType, imports, parent, additionalProperties, callerContext)[0].value<QObject *>();
}

QVariantList QSObjectCreatorCPP::createQmlObjects(int count, const QString& qsType,
                                             const QStringList& imports,
                                             QObject* parent,
                                             QMap<QString, QVariant> additionalProperties,
                                             QVariant callerContext)
{
    QVariantList createdObjects;
    if (count <= 0)
        return createdObjects;

    // Get QML engine from context if not already set
    if (!m_engine) {
        m_engine = qmlEngine(this);
        if (!m_engine) {
            qWarning() << "QSObjectCreatorCPP: Unable to get QML engine";
            return createdObjects;
        }
    }

    QQmlContext* context = callerContext.value<QQmlContext*>();
    if (!context && parent) {
        context = qmlContext(parent);
    }

    QUrl baseUrl = context ? context->baseUrl() : m_engine->baseUrl();

    // Build import string
    QString importString;
    for (const QString& import : imports) {
        importString += QString("import %1; ").arg(import);
    }

    // Build complete QML string
    QString qmlString = QString("%1%2{}").arg(importString, qsType);

    // Use serializer as parent if no parent provided
    QObject* actualParent = parent ? parent : this;

    // Create and compile component ONCE
    QQmlComponent component(m_engine);
    component.setData(qmlString.toUtf8(), baseUrl.resolved(QUrl("inline.qml")));

    if (component.isError()) {
        qCritical() << "QSObjectCreatorCPP: Error creating component:" << component.errors();
        return createdObjects;
    }

    // Create multiple instances from the same compiled component
    for (int i = 0; i < count; ++i) {
        QObject* obj = component.create();
        if (!obj) {
            qCritical() << "QSObjectCreatorCPP: Failed to create object" << i;
            continue;
        }

        QQmlEngine::setObjectOwnership(obj, QQmlEngine::JavaScriptOwnership);

        // Set parent
        obj->setParent(actualParent);

        // Set properties
        for (auto it = additionalProperties.constBegin(); it != additionalProperties.constEnd(); ++it) {
            obj->setProperty(it.key().toUtf8().constData(), it.value());
        }

        // Handle _qsParent property if parent is serializer
        if (actualParent == this) {
            obj->setProperty("_qsParent", QVariant::fromValue<QObject*>(actualParent));
        }

        createdObjects.append(QVariant::fromValue(obj));
    }

    return createdObjects;
}
