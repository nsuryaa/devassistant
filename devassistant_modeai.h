#ifndef DEVASSISTANT_MODEAI_H
#define DEVASSISTANT_MODEAI_H

#include "devassistant_global.h"

#include "devassistant_mainai.h"
#include "devassistant_suggestor.h"
#include "devassistant_ollamamanager.h"

using namespace DevAssistant::Internal;

class DevAssistant_ModeAI : public Core::IMode
{
    Q_OBJECT
public:
    explicit DevAssistant_ModeAI(QObject *parent = nullptr);
    ~DevAssistant_ModeAI();

private slots:
    void grabEditorManager(Core::Id mode);
    void onDocumentChanged();
    void onCursorPositionChanged();
    void requestSuggestionsDelayed();

public:
    DevAssistantSuggestor *suggestionsWidget() const { return m_DevAssistantSuggestor; }
    DevAssistant_OllamaManager *ollamaManager() const { return m_ollamaManager; }

private:
    void setupConnections();

    Core::MiniSplitter *m_splitter;
    QVBoxLayout *m_rightSplitWidgetLayout;

    QPointer<DevAssistantSuggestor> m_DevAssistantSuggestor;
    QPointer<DevAssistant_OllamaManager> m_ollamaManager;
    QPointer<QTimer> m_requestTimer;
    bool m_isModeActive;
};

#endif // DEVASSISTANT_MODEAI_H
