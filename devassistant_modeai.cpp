#include "devassistant_modeai.h"
#include "devassistant_ollamaservice.h"

DevAssistant_ModeAI::DevAssistant_ModeAI(QObject *parent)
    : Core::IMode(parent),
      m_splitter(new Core::MiniSplitter),
      m_rightSplitWidgetLayout(new QVBoxLayout),
      m_ollamaManager(new DevAssistant_OllamaManager(this)),
      m_requestTimer(new QTimer(this)),
      m_isModeActive(false)
{
    setObjectName(QLatin1String("DevAIMode"));
    setId("DevAssistant.AiMode");
    setDisplayName(tr("Dev AI"));
    setIcon(QIcon(":/images/mode_assistant.png"));
    setPriority(82);

    m_rightSplitWidgetLayout->setSpacing(0);
    m_rightSplitWidgetLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *rightSplitWidget = new QWidget;
    rightSplitWidget->setLayout(m_rightSplitWidgetLayout);
    auto editorPlaceHolder = new Core::EditorManagerPlaceHolder;
    m_rightSplitWidgetLayout->insertWidget(0, editorPlaceHolder);

    /*Create AI Suggestions widget*/
    m_DevAssistantSuggestor = new DevAssistantSuggestor(m_splitter);

    /*Create right pane with suggestions widget*/
    auto rightPaneSplitter = new Core::MiniSplitter;
    rightPaneSplitter->insertWidget(0, rightSplitWidget);
    /*Replace RightPanePlaceHolder*/
    rightPaneSplitter->insertWidget(1, m_DevAssistantSuggestor);
    rightPaneSplitter->setStretchFactor(0, 1);
    rightPaneSplitter->setStretchFactor(1, 0);

    auto splitter = new Core::MiniSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->insertWidget(0, rightPaneSplitter);
    QWidget *outputPane = new Core::OutputPanePlaceHolder(DevAssistant::Constants::MODE_DEVAI, splitter);
    outputPane->setObjectName(QLatin1String("DevAIModeOutputPanePlaceHolder"));
    splitter->insertWidget(1, outputPane);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 0);

    Core::NavigationWidgetPlaceHolder *leftNav =
            new Core::NavigationWidgetPlaceHolder(DevAssistant::Constants::MODE_DEVAI, Core::Side::Left);

    m_splitter->insertWidget(0, leftNav);
    m_splitter->insertWidget(1, splitter);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 4);

    m_requestTimer->setSingleShot(true);

    /*Request suggestions 2 seconds after typing stops*/
    m_requestTimer->setInterval(2000);

    setupConnections();

    connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged,
            this, &DevAssistant_ModeAI::grabEditorManager);
    m_splitter->setFocusProxy(editorPlaceHolder);

    auto modeContextObject = new Core::IContext(this);
    modeContextObject->setContext(Core::Context(DevAssistant::Constants::C_DEVAIMANAGER));
    modeContextObject->setWidget(m_splitter);
    Core::ICore::addContextObject(modeContextObject);

    setWidget(m_splitter);
    setContext(Core::Context("DevAssistant.AiMode"));
}

DevAssistant_ModeAI::~DevAssistant_ModeAI()
{
    m_isModeActive = false;

    /*Cancel any pending requests before destruction*/
    if(m_ollamaManager) {
        m_ollamaManager->cancelRequest();
    }

    /*Stop timer*/
    if(m_requestTimer) {
        m_requestTimer->stop();
    }

    /*The splitter and its children will be deleted automatically
      because of Qt's parent-child ownership*/
    delete m_splitter;
    m_splitter = nullptr;
}

void DevAssistant_ModeAI::setupConnections()
{
    if((!m_ollamaManager) || (!m_DevAssistantSuggestor) || (!m_requestTimer)) {
        qWarning() << "DevAssistant_ModeAI: Failed to create required objects";
        return;
    }

    /*Use shared service for models*/
    DevAssistant_OllamaService *service = DevAssistant_OllamaService::instance();
    if(!service) {
        qWarning() << "DevAssistant_ModeAI: Failed to get ollama service instance";
        return;
    }

    /*Connect Ollama manager to suggestions widget*/
    connect(m_ollamaManager, &DevAssistant_OllamaManager::SGNL_suggestionsReceived,
            this, [this](const QStringList &suggestions) {
        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->showLoading(false);
            m_DevAssistantSuggestor->setSuggestions(suggestions);
        }
    });

    connect(m_ollamaManager, &DevAssistant_OllamaManager::SGNL_errorOccurred,
            this, [this](const QString &error) {
        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->showLoading(false);
            m_DevAssistantSuggestor->setStatus(QString("Error: %1").arg(error), true);

            /*Still show error in suggestions list*/
            QStringList errorMsg = { QString("❌ Error: %1").arg(error) };
            m_DevAssistantSuggestor->setSuggestions(errorMsg);
        }
    });

    connect(m_ollamaManager, &DevAssistant_OllamaManager::SGNL_requestStarted,
            this, [this]() {
        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->showLoading(true);
        }
    });

    connect(m_ollamaManager, &DevAssistant_OllamaManager::SGNL_requestFinished,
            this, [this]() {
        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->showLoading(false);
        }
    });

    /*Connect model selection changes*/
    connect(m_DevAssistantSuggestor, &DevAssistantSuggestor::SGNL_modelChanged,
            this, [this](const QString &model) {
        if(m_ollamaManager) {
            m_ollamaManager->setModel(model);
        }

        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->setStatus(QString("Model: %1").arg(model));
        }
    });

    /*Connect models received from Ollama server*/
    connect(service, &DevAssistant_OllamaService::SGNL_modelsReceived,
            this, [this](const QStringList &models) {

        if((!m_DevAssistantSuggestor) || (!m_ollamaManager)) {
            return;
        }

        m_DevAssistantSuggestor->setAvailableModels(models);

        /*Set the first model as default if available*/
        if(!models.isEmpty()) {
            /*Find a good code model preference*/
            QString preferredModel;
            preferredModel = models.first();

            m_ollamaManager->setModel(preferredModel);
            m_DevAssistantSuggestor->setStatus(QString("Ready - Using %1").arg(preferredModel));
        }
    });

    /*Connect editor changes to request timer*/
    connect(m_requestTimer, &QTimer::timeout,
            this, &DevAssistant_ModeAI::requestSuggestionsDelayed);

    /*Monitor editor changes*/
    connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
            this, [this](Core::IEditor *editor) {
        if((!editor) || (!m_DevAssistantSuggestor) || (!m_isModeActive)) {
            return;
        }

        auto textEditor = qobject_cast<TextEditor::BaseTextEditor *>(editor);
        if(!textEditor) {
            return;
        }

        auto editorWidget = textEditor->editorWidget();
        if(!editorWidget) {
            return;
        }

        QTextDocument *doc = editorWidget->document();
        if(doc) {
            connect(doc, &QTextDocument::contentsChanged,
                    this, &DevAssistant_ModeAI::onDocumentChanged, Qt::UniqueConnection);
        }

        connect(editorWidget, &TextEditor::TextEditorWidget::cursorPositionChanged,
                this, &DevAssistant_ModeAI::onCursorPositionChanged, Qt::UniqueConnection);
    });

    /*Connect refresh button in suggestions widget*/
    connect(m_DevAssistantSuggestor, &DevAssistantSuggestor::SGNL_refreshRequested,
            this, &DevAssistant_ModeAI::requestSuggestionsDelayed);

    connect(service, &DevAssistant_OllamaService::SGNL_modelsFetchError,
            this, [this](const QString &error) {
        if(m_DevAssistantSuggestor) {
            m_DevAssistantSuggestor->setStatus(error, true);
        }
    });

    /*Fetch available models from Ollama server*/
    if(m_DevAssistantSuggestor) {
        m_DevAssistantSuggestor->setStatus(tr("Connecting to Ollama..."));
    }

    /*Fetch available models from Ollama server*/
    /*Use a small delay to ensure the mode is fully initialized*/
    QTimer::singleShot(500, this, [service]() {
        if(service) {
            service->fetchAvailableModels();
        }
    });

}

void DevAssistant_ModeAI::grabEditorManager(Core::Id mode)
{
    if(mode != id()) {
        /*Leaving Dev AI mode*/
        m_isModeActive = false;

        /*Clean up when leaving this mode*/
        if(m_requestTimer) {
            m_requestTimer->stop();
        }

        if(m_ollamaManager) {
            m_ollamaManager->cancelRequest();
            DevAssistant_OllamaService::instance()->cancelAllRequestsFor(m_ollamaManager);
        }
        return;
    }

    /*Entering Dev AI mode*/
    m_isModeActive = true;

    /*Activate Projects widget in the left navigation*/
    QTimer::singleShot(100, this, []() {
        /*Find and activate the Projects navigation widget*/
        Core::Command *cmd = Core::ActionManager::command("QtCreator.Sidebar.Projects");
        if(cmd && cmd->action()) {
            qDebug() << "DevAssistant_ModeAI: Activating Projects widget";
            cmd->action()->trigger();
        } else {
            qDebug() << "DevAssistant_ModeAI: Could not find Projects command";

            /*Debug: Print all available sidebar commands*/
            QList<Core::Command *> commands = Core::ActionManager::instance()->commands();
            for(Core::Command *c : commands) {
                QString id = c->id().toString();
                if(id.contains("Project", Qt::CaseInsensitive) ||
                        id.contains("Sidebar", Qt::CaseInsensitive)) {
                    qDebug() << "  Found:" << id;
                }
            }
        }
    });

    /*Re-connect to current editor if one is open*/
    Core::IEditor *editor = Core::EditorManager::currentEditor();
    if(editor) {
        auto textEditor = qobject_cast<TextEditor::BaseTextEditor *>(editor);
        if(textEditor) {
            auto editorWidget = textEditor->editorWidget();
            if(editorWidget) {
                QTextDocument *doc = editorWidget->document();
                if(doc) {
                    connect(doc, &QTextDocument::contentsChanged,
                            this, &DevAssistant_ModeAI::onDocumentChanged, Qt::UniqueConnection);
                }
                connect(editorWidget, &TextEditor::TextEditorWidget::cursorPositionChanged,
                        this, &DevAssistant_ModeAI::onCursorPositionChanged, Qt::UniqueConnection);

                m_DevAssistantSuggestor->setStatus(
                            QString("Monitoring: %1").arg(editor->document()->displayName()));
            }
        }
    }

    if(Core::EditorManager::currentEditor()) {
        Core::EditorManager::currentEditor()->widget()->setFocus();
    }
}

void DevAssistant_ModeAI::onDocumentChanged()
{
    /*Only start the timer if mode is active*/
    if(!m_isModeActive) {
        return;
    }

    /*Restart timer on each change*/
    if(m_requestTimer) {
        m_requestTimer->start();
    }
}

void DevAssistant_ModeAI::onCursorPositionChanged()
{
    /*Only trigger if mode is active*/
    if(!m_isModeActive) {
        return;
    }

    //Optional: Request suggestions on cursor movement
    //if(m_requestTimer) {
    //    m_requestTimer->start();
    //}
}

void DevAssistant_ModeAI::requestSuggestionsDelayed()
{
    /*Only trigger if mode is active*/
    if(!m_isModeActive) {
        return;
    }

    if((!m_DevAssistantSuggestor) || (!m_ollamaManager)) {
        return;
    }

    QString model = m_DevAssistantSuggestor->currentModel();
    if(model.isEmpty()) {
        m_DevAssistantSuggestor->setStatus(tr("No model selected"), true);
        return;
    }

    m_ollamaManager->requestSuggestions(QString(), 0);
}
