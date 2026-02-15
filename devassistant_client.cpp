#include "devassistant_client.h"
#include "devassistant_ollamaservice.h"

DevAssistant_Client::DevAssistant_Client(QObject *parent)
    : QObject(parent),
      m_selectedModel("codellama")
{
}

DevAssistant_Client::~DevAssistant_Client()
{
    if(!m_qsCurrentRequestId.isEmpty())
    {
        DevAssistant_OllamaService::instance()->cancelRequest(m_qsCurrentRequestId);
        m_qsCurrentRequestId.clear();
    }
}

/*Send a user prompt to the AI backend*/
void DevAssistant_Client::SendPromptoAi(const QString &qsUserPromptToai)
{
    /*Cancel previous request if active*/
    if(!m_qsCurrentRequestId.isEmpty())
    {
        DevAssistant_OllamaService::instance()->cancelRequest(m_qsCurrentRequestId);
        m_qsCurrentRequestId.clear();
    }

    emit SGNL_ResponseStartedFromAi();

    /*Use shared OllamaService for streaming*/
    m_qsCurrentRequestId = DevAssistant_OllamaService::instance()->requestStreaming(
                qsUserPromptToai,
                m_selectedModel,
                this,
                "SLOT_receiveAiReponse",
                "SLOT_ReponseFromAI_Finished",
                "SLOT_AiError"
                );

    qDebug() << "DevAssistant_Client: Streaming request sent" << m_qsCurrentRequestId;
}

void DevAssistant_Client::SLOT_receiveAiReponse(const QString &chunk)
{
    emit SGNL_partialResponseFromAi(chunk);
}

void DevAssistant_Client::SLOT_ReponseFromAI_Finished()
{
    m_qsCurrentRequestId.clear();
    emit SGNL_AiResponseFinished();
}

void DevAssistant_Client::SLOT_AiError(const QString &error)
{
    m_qsCurrentRequestId.clear();
    emit SGNL_AiError(error);
    qDebug() << "DevAssistant_Client: Error - " << error;
}
