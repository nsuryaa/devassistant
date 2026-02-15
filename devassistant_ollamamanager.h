#ifndef DEVASSISTANT_OLLAMAMANAGER_H
#define DEVASSISTANT_OLLAMAMANAGER_H

#include "devassistant_global.h"

namespace DevAssistant {
namespace Internal {

class DevAssistant_OllamaManager : public QObject
{
    Q_OBJECT

public:
    explicit DevAssistant_OllamaManager(QObject *parent = nullptr);
    ~DevAssistant_OllamaManager();

    void requestSuggestions(const QString &qsCode, int iCursorPosition);
    void setModel(const QString &qsModelName);
    void cancelRequest();

signals:
    void SGNL_suggestionsReceived(const QStringList &qslSuggestions);
    void SGNL_errorOccurred(const QString &qsError);
    void SGNL_requestStarted();
    void SGNL_requestFinished();

private slots:
    void SLOT_onDebounceTimeout();
    void SLOT_onOllamaResponse(const QString &response);
    void SLOT_onOllamaError(const QString &error);

private:
    QString buildPrompt(const QString &code, int cursorPosition);
    QStringList parseSuggestions(const QString &response);

    QTimer *m_qtdebounceTimer;
    QString m_qsModel;
    QString m_qsCurrentRequestId;
    QString m_qsPendingCode;
    int m_iPendingCursorPosition;
};

} // namespace Internal
} // namespace DevAssistant

#endif // DEVASSISTANT_OLLAMAMANAGER_H
