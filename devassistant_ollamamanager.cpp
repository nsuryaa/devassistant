#include "devassistant_ollamamanager.h"
#include "devassistant_ollamaservice.h"

using namespace DevAssistant::Internal;

DevAssistant_OllamaManager::DevAssistant_OllamaManager(QObject *parent)
    : QObject(parent),
      m_qtdebounceTimer(new QTimer(this)),
      m_qsModel("codellama")
{
    m_qtdebounceTimer->setSingleShot(true);

    /*1 second debounce*/
    m_qtdebounceTimer->setInterval(1000);

    connect(m_qtdebounceTimer, &QTimer::timeout,
            this, &DevAssistant_OllamaManager::SLOT_onDebounceTimeout);
}

DevAssistant_OllamaManager::~DevAssistant_OllamaManager()
{
    cancelRequest();
}

void DevAssistant_OllamaManager::setModel(const QString &qsModelName)
{
    //qDebug() << m_qsModel;
    m_qsModel = qsModelName;
}

void DevAssistant_OllamaManager::requestSuggestions(const QString &qsCode, int iCursorPosition)
{
    /*Store for debounced request*/
    m_qsPendingCode = qsCode;
    m_iPendingCursorPosition = iCursorPosition;

    /*Restart debounce timer*/
    m_qtdebounceTimer->start();
}

void DevAssistant_OllamaManager::SLOT_onDebounceTimeout()
{
    /*Cancel any existing request*/
    cancelRequest();

    emit SGNL_requestStarted();

    QString qsPrompt = buildPrompt(m_qsPendingCode, m_iPendingCursorPosition);

    /*Use the shared service*/
    m_qsCurrentRequestId = DevAssistant_OllamaService::instance()->requestCompletion(
                qsPrompt,
                m_qsModel,
                this,
                "SLOT_onOllamaResponse",
                "SLOT_onOllamaError"
                );

    qDebug() << "OllamaManager: Requested suggestions with ID:" << m_qsCurrentRequestId;
}

void DevAssistant_OllamaManager::SLOT_onOllamaResponse(const QString &response)
{
    m_qsCurrentRequestId.clear();
    emit SGNL_requestFinished();

    qDebug() << "OllamaManager: Received response, length:" << response.length();

    QStringList qslSuggestions = parseSuggestions(response);

    if(qslSuggestions.isEmpty())
    {
        qslSuggestions << "//No suggestions available";
    }

    emit SGNL_suggestionsReceived(qslSuggestions);
}

void DevAssistant_OllamaManager::SLOT_onOllamaError(const QString &qsError)
{
    m_qsCurrentRequestId.clear();
    emit SGNL_requestFinished();
    emit SGNL_errorOccurred(qsError);
}

void DevAssistant_OllamaManager::cancelRequest()
{
    if(!m_qsCurrentRequestId.isEmpty())
    {
        DevAssistant_OllamaService::instance()->cancelRequest(m_qsCurrentRequestId);
        m_qsCurrentRequestId.clear();
        emit SGNL_requestFinished();
    }
}

QString DevAssistant_OllamaManager::buildPrompt(const QString &qsCode, int iCursorPosition)
{
    QString qsBeforeCursor = qsCode.left(iCursorPosition);
    QString qsAfterCursor = qsCode.mid(iCursorPosition);

    QString qsPrompt = QString(
                "You are a code completion assistant. Given the following code context, "
                "suggest 3-5 relevant code completions or improvements.\n\n"
                "Code before cursor:\n%1\n\n"
                "Code after cursor:\n%2\n\n"
                "Provide suggestions as a JSON array of strings. Each suggestion should be:\n"
                "- A complete, valid code snippet\n"
                "- Contextually relevant\n"
                "- Properly formatted\n"
                "- Ready to insert at the cursor position\n\n"
                "Format: [\"suggestion1\", \"suggestion2\", \"suggestion3\"]\n"
                "Only return the JSON array, nothing else."
                ).arg(qsBeforeCursor, qsAfterCursor);

    return qsPrompt;
}

QStringList DevAssistant_OllamaManager::parseSuggestions(const QString &qsResponse)
{
    QStringList qslSuggestions;

    /*Try to extract JSON array from response*/
    QJsonDocument qjsondoc = QJsonDocument::fromJson(qsResponse.toUtf8());

    if(qjsondoc.isArray())
    {
        QJsonArray qsaarray = qjsondoc.array();
        for(const QJsonValue &value : qsaarray)
        {
            if(value.isString())
            {
                qslSuggestions.append(value.toString());
            }
        }
    }
    else
    {
        /*Fallback: split by newlines if not JSON*/
        QStringList qslLines = qsResponse.split('\n', QString::SkipEmptyParts);
        for(const QString &qsline : qslLines)
        {
            QString qsTrimmed = qsline.trimmed();
            if((!qsTrimmed.isEmpty()) && (qsTrimmed.length() > 5))
            {
                qslSuggestions.append(qsTrimmed);
            }
        }
    }

    /*Limit to 5 suggestions*/
    if(qslSuggestions.size() > 5)
    {
        qslSuggestions = qslSuggestions.mid(0, 5);
    }

    return qslSuggestions;
}
