#ifndef DEVASSISTANT_CLIENT_H
#define DEVASSISTANT_CLIENT_H

#include "devassistant_global.h"

class DevAssistant_Client : public QObject
{
    Q_OBJECT
public:
    explicit DevAssistant_Client(QObject *parent = 0);
    ~DevAssistant_Client();

    QString m_selectedModel;

    QString selectedModel() const { return m_selectedModel; }
    void SendPromptoAi(const QString &qsUserPromptToai);
    void setSelectedModel(const QString &model) { m_selectedModel = model; }

signals:
    void SGNL_ResponseStartedFromAi();
    void SGNL_partialResponseFromAi(const QString &chunk);
    void SGNL_AiResponseFinished();
    void SGNL_AiError(const QString &error);

private slots:
    void SLOT_receiveAiReponse(const QString &chunk);
    void SLOT_ReponseFromAI_Finished();
    void SLOT_AiError(const QString &error);

private:
    QString m_qsCurrentRequestId;
};

#endif // DEVASSISTANT_CLIENT_H
