#include "devassistant_ollamaservice.h"

DevAssistant_OllamaService *DevAssistant_OllamaService::s_instance = nullptr;

DevAssistant_OllamaService* DevAssistant_OllamaService::instance()
{
    if(!s_instance) {
        s_instance = new DevAssistant_OllamaService();
    }
    return s_instance;
}

DevAssistant_OllamaService::DevAssistant_OllamaService(QObject *parent)
    : QObject(parent),
      m_networkAccessManager(new QNetworkAccessManager(this)),
      m_qnrModelsReply(nullptr),
      m_qsServerUrl("http://localhost:11434"),
      m_requestCounter(0)
{
    qDebug() << "DevAssistant_OllamaService: Initialized";
}

DevAssistant_OllamaService::~DevAssistant_OllamaService()
{
    /*Abort all active replies*/
    for(auto it = m_qmapActiveReplies.begin(); it != m_qmapActiveReplies.end(); ++it) {
        if(it.value()) {
            it.value()->abort();
            it.value()->deleteLater();
        }
    }
    m_qmapActiveReplies.clear();
    m_qmapActiveRequests.clear();
    m_qmapStreamBuffers.clear();

    if(m_qnrModelsReply)
    {
        m_qnrModelsReply->abort();
        m_qnrModelsReply->deleteLater();
    }
    qDebug() << "DevAssistant_OllamaService: Destroyed";
}

void DevAssistant_OllamaService::setServerUrl(const QString &url)
{
    m_qsServerUrl = url;
    qDebug() << "DevAssistant_OllamaService: Server URL set to" << m_qsServerUrl;
}

QString DevAssistant_OllamaService::requestCompletion(const QString &qsPrompt, const QString &qsModel,
                                                      QObject *qobjReceiver, const char *cSuccessSlot, const char *cErrorSlot)
{
    QString qsRequestId = generateRequestId();

    S_OLLAMA_REQUEST SOllamaRequest;
    SOllamaRequest.qsId = qsRequestId;
    SOllamaRequest.qsPrompt = qsPrompt;
    SOllamaRequest.qsModel = qsModel;
    SOllamaRequest.bStreaming = false;
    SOllamaRequest.qpObjreceiver = qobjReceiver;
    SOllamaRequest.qba_SuccessSlot = QByteArray(cSuccessSlot);
    SOllamaRequest.qba_ErrorSlot = QByteArray(cErrorSlot);

    QJsonObject qjObj_body;
    qjObj_body["model"] = qsModel;
    qjObj_body["prompt"] = qsPrompt;
    qjObj_body["stream"] = false;

    QJsonObject qjobjOptions;
    qjobjOptions["temperature"] = 0.3;
    qjobjOptions["top_p"] = 0.9;
    qjobjOptions["top_k"] = 40;
    qjObj_body["options"] = qjobjOptions;

    QNetworkRequest qnreqRequest(QUrl(m_qsServerUrl + "/api/generate"));
    qnreqRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *qnrReply = m_networkAccessManager->post(qnreqRequest, QJsonDocument(qjObj_body).toJson());

    /*Use a unique property to track which request this reply belongs to*/
    qnrReply->setProperty("requestId", qsRequestId);

    m_qmapActiveReplies[qsRequestId] = qnrReply;
    m_qmapActiveRequests[qsRequestId] = SOllamaRequest;

    connect(qnrReply, &QNetworkReply::finished, this, &DevAssistant_OllamaService::onReplyFinished);

    qDebug() << "OllamaService: Non-streaming request sent" << qsRequestId;
    return qsRequestId;
}

QString DevAssistant_OllamaService::requestStreaming(const QString &qsPrompt, const QString &qsModel, QObject *qObjReceiver, const char *cPartialSlot, const char *cFinishedSlot, const char *cErrorSlot)
{
    QString qsRequestId = generateRequestId();

    S_OLLAMA_REQUEST SOllamaRequest;
    SOllamaRequest.qsId = qsRequestId;
    SOllamaRequest.qsPrompt = qsPrompt;
    SOllamaRequest.qsModel = qsModel;
    SOllamaRequest.bStreaming = true;
    SOllamaRequest.qpObjreceiver = qObjReceiver;
    SOllamaRequest.qba_SuccessSlot = QByteArray(cFinishedSlot);
    SOllamaRequest.qba_ErrorSlot = QByteArray(cErrorSlot);
    SOllamaRequest.qba_PartialSlot = QByteArray(cPartialSlot);

    QJsonObject qjObj_body;
    qjObj_body["model"] = qsModel;
    qjObj_body["prompt"] = qsPrompt;
    qjObj_body["stream"] = true;

    QNetworkRequest qnreqRequest(QUrl(m_qsServerUrl + "/api/generate"));
    qnreqRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *qnrReply = m_networkAccessManager->post(qnreqRequest, QJsonDocument(qjObj_body).toJson());
    qnrReply->setProperty("requestId", qsRequestId);

    m_qmapActiveReplies[qsRequestId]  = qnrReply;
    m_qmapActiveRequests[qsRequestId] = SOllamaRequest;
    m_qmapStreamBuffers[qsRequestId]  = QByteArray();

    connect(qnrReply, &QNetworkReply::readyRead, this, &DevAssistant_OllamaService::onStreamingReadyRead);
    connect(qnrReply, &QNetworkReply::finished, this, &DevAssistant_OllamaService::onReplyFinished);

    qDebug() << "OllamaService: Streaming request sent" << qsRequestId;
    return qsRequestId;
}

void DevAssistant_OllamaService::cancelRequest(const QString &qsRequestId)
{
    QNetworkReply *qnrReply = m_qmapActiveReplies.value(qsRequestId, nullptr);
    if(qnrReply)
    {
        qnrReply->abort();
        qnrReply->deleteLater();
        m_qmapActiveReplies.remove(qsRequestId);
        m_qmapActiveRequests.remove(qsRequestId);
        m_qmapStreamBuffers.remove(qsRequestId);
        qDebug() << "OllamaService: Cancelled request" << qsRequestId;
    }
}

void DevAssistant_OllamaService::cancelAllRequestsFor(QObject *qObjReceiver)
{
    QStringList toRemove;

    for(auto it = m_qmapActiveRequests.begin(); it != m_qmapActiveRequests.end(); ++it) {
        if(it.value().qpObjreceiver.data() == qObjReceiver)
        {
            toRemove.append(it.key());
        }
    }

    for(const QString &qsId : toRemove)
    {
        cancelRequest(qsId);
        qDebug() << "OllamaService: Cancelled request for destroyed receiver:" << qsId;
    }
}

void DevAssistant_OllamaService::onReplyFinished()
{
    QNetworkReply *qnrReply = qobject_cast<QNetworkReply *>(sender());
    if(!qnrReply)
    {
        return;
    }

    QString qsRequestId = qnrReply->property("requestId").toString();
    S_OLLAMA_REQUEST SOllamaRequest = m_qmapActiveRequests.value(qsRequestId);

    /*Receiver was already destroyed — just clean up, don't invoke anything*/
    if(SOllamaRequest.qpObjreceiver.isNull())
    {
        qDebug() << "OllamaService: Receiver destroyed, skipping callback" << qsRequestId;
        qnrReply->deleteLater();
        m_qmapActiveReplies.remove(qsRequestId);
        m_qmapActiveRequests.remove(qsRequestId);
        m_qmapStreamBuffers.remove(qsRequestId);
        return;
    }

    qDebug() << "OllamaService: Request finished" << qsRequestId
             << "Streaming:" << SOllamaRequest.bStreaming;

    /*Handle errors*/
    if((qnrReply->error() != QNetworkReply::NoError) &&
            (qnrReply->error() != QNetworkReply::OperationCanceledError))
    {
        QString qsError;
        if(qnrReply->error() == QNetworkReply::ConnectionRefusedError)
        {
            qsError = "Cannot connect to Ollama. Is it running? Start with: ollama serve";
        }
        else
        {
            qsError = QString("Request failed: %1").arg(qnrReply->errorString());
        }

        if((SOllamaRequest.qpObjreceiver) && (!SOllamaRequest.qba_ErrorSlot.isEmpty())) {
            qDebug() << "OllamaService: Invoking error slot:" << SOllamaRequest.qba_ErrorSlot;

            QMetaObject::invokeMethod(SOllamaRequest.qpObjreceiver,
                                      SOllamaRequest.qba_ErrorSlot.constData(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, qsError));
        }
    }
    /*Non-streaming: read full response here*/
    else if((!SOllamaRequest.bStreaming) && (qnrReply->error() == QNetworkReply::NoError))
    {
        QByteArray qbaData = qnrReply->readAll();
        QJsonDocument qjdocDoc = QJsonDocument::fromJson(qbaData);

        if(qjdocDoc.isObject())
        {
            QString qsResponse = qjdocDoc.object().value("response").toString();
            if((SOllamaRequest.qpObjreceiver) && (!SOllamaRequest.qba_SuccessSlot.isEmpty())) {
                qDebug() << "OllamaService: Invoking success slot:" << SOllamaRequest.qba_SuccessSlot;

                QMetaObject::invokeMethod(SOllamaRequest.qpObjreceiver,
                                          SOllamaRequest.qba_SuccessSlot.constData(),
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, qsResponse));
            }
        }
        else
        {
            if((SOllamaRequest.qpObjreceiver) && (!SOllamaRequest.qba_ErrorSlot.isEmpty()))
            {
                qDebug() << "OllamaService: Invoking error slot:" << SOllamaRequest.qba_ErrorSlot;

                QMetaObject::invokeMethod(SOllamaRequest.qpObjreceiver,
                                          SOllamaRequest.qba_ErrorSlot.constData(),
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, "Invalid response from Ollama"));
            }
        }
    }
    /*Streaming: emit finished signal*/
    else if((SOllamaRequest.bStreaming) && (qnrReply->error() == QNetworkReply::NoError))
    {
        if((SOllamaRequest.qpObjreceiver) && (!SOllamaRequest.qba_SuccessSlot.isEmpty()))
        {
            qDebug() << "OllamaService: Invoking finished slot:" << SOllamaRequest.qba_SuccessSlot;

            QMetaObject::invokeMethod(SOllamaRequest.qpObjreceiver,
                                      SOllamaRequest.qba_SuccessSlot.constData(),
                                      Qt::QueuedConnection);
        }
    }

    /*Cleanup*/
    qnrReply->deleteLater();
    m_qmapActiveReplies.remove(qsRequestId);
    m_qmapActiveRequests.remove(qsRequestId);
    m_qmapStreamBuffers.remove(qsRequestId);
}

void DevAssistant_OllamaService::onModelsReplyFinished()
{
    if(!m_qnrModelsReply)
    {
        return;
    }

    QStringList qslModels;

    if(m_qnrModelsReply->error() != QNetworkReply::NoError)
    {
        emit SGNL_modelsFetchError(QString("Failed to fetch models: %1").arg(m_qnrModelsReply->errorString()));
        if(!m_qslCachedModels.isEmpty())
        {
            qslModels = m_qslCachedModels;
        }
    }
    else
    {
        QByteArray qbaData = m_qnrModelsReply->readAll();
        QJsonDocument qjsonDoc = QJsonDocument::fromJson(qbaData);

        if(qjsonDoc.isObject())
        {
            QJsonArray qjsa_ModelsArray = qjsonDoc.object().value("models").toArray();
            for(const QJsonValue &qsonvalue : qjsa_ModelsArray)
            {
                if(qsonvalue.isObject())
                {
                    QString qsName = qsonvalue.toObject().value("name").toString();
                    if(!qsName.isEmpty())
                    {
                        if(qsName.endsWith(":latest"))
                        {
                            qsName = qsName.left(qsName.length() - 7);
                        }

                        if(!qslModels.contains(qsName))
                        {
                            qslModels.append(qsName);
                        }
                    }
                }
            }

            m_qslCachedModels = qslModels;
        }
    }

    m_qnrModelsReply->deleteLater();
    m_qnrModelsReply = nullptr;

    emit SGNL_modelsReceived(qslModels);
}

void DevAssistant_OllamaService::fetchAvailableModels()
{
    if(m_qnrModelsReply)
    {
        return;
    }

    QNetworkRequest qnrRequest(QUrl(m_qsServerUrl + "/api/tags"));
    qnrRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_qnrModelsReply = m_networkAccessManager->get(qnrRequest);
    connect(m_qnrModelsReply, &QNetworkReply::finished,
            this, &DevAssistant_OllamaService::onModelsReplyFinished);
}

void DevAssistant_OllamaService::onStreamingReadyRead()
{
    QNetworkReply *qnrReply = qobject_cast<QNetworkReply *>(sender());
    if(!qnrReply)
    {
        return;
    }

    QString qsRequestId = qnrReply->property("requestId").toString();
    S_OLLAMA_REQUEST SOllamaRequest = m_qmapActiveRequests.value(qsRequestId);

    /*Receiver was deleted (e.g. mode switch) — abort silently*/
    if(SOllamaRequest.qpObjreceiver.isNull())
    {
        qDebug() << "OllamaService: Receiver destroyed, aborting streaming" << qsRequestId;
        cancelRequest(qsRequestId);
        return;
    }

    /*Accumulate data*/
    m_qmapStreamBuffers[qsRequestId] += qnrReply->readAll();

    /*Process complete JSON lines*/
    while(true)
    {
        int iNewlineIndex = m_qmapStreamBuffers[qsRequestId].indexOf('\n');
        if(iNewlineIndex < 0)
        {
            break;
        }

        QByteArray qbaLine = m_qmapStreamBuffers[qsRequestId].left(iNewlineIndex);
        m_qmapStreamBuffers[qsRequestId].remove(0, iNewlineIndex + 1);

        QJsonDocument qjsonDoc = QJsonDocument::fromJson(qbaLine);
        if(!qjsonDoc.isObject())
        {
            continue;
        }

        QJsonObject qjsonObj = qjsonDoc.object();
        QString qsChunk = qjsonObj.value("response").toString();

        if(!qsChunk.isEmpty())
        {
            QMetaObject::invokeMethod(SOllamaRequest.qpObjreceiver,
                                      SOllamaRequest.qba_PartialSlot.constData(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, qsChunk));
        }

        if(qjsonObj.value("done").toBool())
        {
            break;
        }
    }
}

QString DevAssistant_OllamaService::generateRequestId()
{
    return QString("req_%1").arg(++m_requestCounter);
}
