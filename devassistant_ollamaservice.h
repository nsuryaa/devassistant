#ifndef DEVASSISTANT_OLLAMASERVICE_H
#define DEVASSISTANT_OLLAMASERVICE_H

#include "devassistant_global.h"

/*Request structure*/
struct S_OLLAMA_REQUEST {
    QString qsId;
    QString qsPrompt;
    QString qsModel;
    bool bStreaming;
    QPointer<QObject> qpObjreceiver;

    QByteArray qba_SuccessSlot;
    QByteArray qba_ErrorSlot;
    QByteArray qba_PartialSlot;  /*Only for streaming*/
};

class DevAssistant_OllamaService : public QObject
{
    Q_OBJECT

public:
    static DevAssistant_OllamaService* instance();

    ~DevAssistant_OllamaService();

    /*Non-streaming request (used by suggestions pane)*/
    QString requestCompletion(const QString &qsPrompt, const QString &qsModel,
                              QObject *qobjReceiver, const char *cSuccessSlot,
                              const char *cErrorSlot);

    /*Streaming request (used by AI Assistant sidebar)*/
    QString requestStreaming(const QString &qsPrompt, const QString &qsModel,
                             QObject *qobjReceiver, const char *cPartialSlot,
                             const char *cfinishedSlot, const char *cErrorSlot);

    void cancelRequest(const QString &qsRequestId);
    void cancelAllRequestsFor(QObject *qObjReceiver);

    /*Configuration*/
    void setServerUrl(const QString &url);
    QString serverUrl() const { return m_qsServerUrl; }

    /*Model management*/
    void fetchAvailableModels();
    QStringList cachedModels() const { return m_qslCachedModels; }/*unused*/

signals:
    void SGNL_modelsReceived(const QStringList &qsl_Models);
    void SGNL_modelsFetchError(const QString &qsError);

private:
    explicit DevAssistant_OllamaService(QObject *parent = nullptr);
    static DevAssistant_OllamaService *s_instance;

    void onReplyFinished();
    void onModelsReplyFinished();
    void onStreamingReadyRead();

    QString generateRequestId();

    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_qnrModelsReply;

    QString m_qsServerUrl;
    QStringList m_qslCachedModels;

    /*Track active requests: requestId -> reply + metadata*/
    QMap<QString, QNetworkReply *> m_qmapActiveReplies;
    QMap<QString, S_OLLAMA_REQUEST> m_qmapActiveRequests;
    QMap<QString, QByteArray> m_qmapStreamBuffers;  /*Streaming buffers*/

    int m_requestCounter;
};

#endif // DEVASSISTANT_OLLAMASERVICE_H
