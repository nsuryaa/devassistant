#include "devassistant_mainai.h"

DevAssistant_MainAI::DevAssistant_MainAI(QWidget *parent)
    : QWidget(parent),
      m_bAiActive(false),
      m_bCodeReplacement(false)
{
    setupUI();
    applyDarkTheme();

    /* ================= BACKEND ================= */
    m_obj_devassist_client = new DevAssistant_Client(this);

    /* ================= SIGNALS ================= */
    connect(m_qpshbtn_SendToAI, &QPushButton::clicked,
            this, &DevAssistant_MainAI::SLOT_UpdatesUserRequestToAi);

    connect(m_obj_devassist_client, &DevAssistant_Client::SGNL_ResponseStartedFromAi,
            this, &DevAssistant_MainAI::SLOT_onAIResponseStarted);

    connect(m_obj_devassist_client, &DevAssistant_Client::SGNL_partialResponseFromAi,
            this, &DevAssistant_MainAI::SLOT_ReceivePartialAiOutput);

    connect(m_obj_devassist_client, &DevAssistant_Client::SGNL_AiResponseFinished,
            this, &DevAssistant_MainAI::SLOT_onAIStreamingResponseFinished);

    connect(m_obj_devassist_client, &DevAssistant_Client::SGNL_AiError,
            this, &DevAssistant_MainAI::SLOT_AiThrowedError);

    connect(DevAssistant_OllamaService::instance(),
            &DevAssistant_OllamaService::SGNL_modelsReceived,
            this, &DevAssistant_MainAI::onModelsFetched);

    DevAssistant_OllamaService::instance()->fetchAvailableModels();
}

DevAssistant_MainAI::~DevAssistant_MainAI()
{
    qDebug() << "Main AI destructor called";
}

/* ============================================================
 * UI SETUP - NEW MODERN INTERFACE
 * ============================================================ */
void DevAssistant_MainAI::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    /*Setup toolbar*/
    setupToolbar();
    mainLayout->addWidget(m_toolbar);

    /*Chat scroll area*/
    qsa_chatScrollArea = new QScrollArea(this);
    qsa_chatScrollArea->setWidgetResizable(true);
    qsa_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsa_chatScrollArea->setFrameShape(QFrame::NoFrame);

    qW_chatContainer = new QWidget;
    qvbl_chatLayout = new QVBoxLayout(qW_chatContainer);
    qvbl_chatLayout->setContentsMargins(12, 12, 12, 12);
    qvbl_chatLayout->setSpacing(12);
    qvbl_chatLayout->setAlignment(Qt::AlignTop);

    qsa_chatScrollArea->setWidget(qW_chatContainer);
    mainLayout->addWidget(qsa_chatScrollArea, 1);

    /*Setup input area*/
    setupInputArea();

    mainLayout->addSpacing(8);

    auto* inputWrapper = new QWidget(this);
    auto* wrapperLayout = new QHBoxLayout(inputWrapper);
    wrapperLayout->setContentsMargins(8, 0, 8, 8);
    wrapperLayout->addWidget(m_inputContainer);

    mainLayout->addWidget(inputWrapper);
}

void DevAssistant_MainAI::setupToolbar()
{
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(40);

    auto* toolbarLayout = new QHBoxLayout(m_toolbar);
    toolbarLayout->setContentsMargins(8, 4, 8, 4);
    toolbarLayout->setSpacing(4);

    /*File icon (attachment)*/
    auto* btnAttach = new QToolButton(this);
    btnAttach->setIcon(QIcon(":/images/attach-file-white.png"));
    btnAttach->setStyleSheet(
                "QToolButton {"
                "    background: transparent;"
                "    border: none;"
                "    color: #ffffff;"
                "    font-size: 16px;"
                "    padding: 4px;"
                "}"
                "QToolButton:hover {"
                "    background: #404142;"
                "}"
                );
    toolbarLayout->addWidget(btnAttach);

    /*Plus button*/
    auto* btnPlus = new QToolButton(this);
    btnPlus->setIcon(QIcon(":/images/add-white.png"));
    btnPlus->setStyleSheet(
                "QToolButton {"
                "    background: transparent;"
                "    border: none;"
                "    color: #ffffff;"
                "    font-size: 18px;"
                "    font-weight: bold;"
                "    padding: 4px 8px;"
                "}"
                "QToolButton:hover {"
                "    background: #404142;"
                "}"
                );
    toolbarLayout->addWidget(btnPlus);

    /*Current file label (you can update this dynamically)*/
    m_lblCurrentFile = new QLabel("Dev AI", this);
    m_lblCurrentFile->setStyleSheet(
                "QLabel {"
                "    color: #4a9eff;"
                "    font-size: 13px;"
                "    padding: 4px 8px;"
                "    background: transparent;"
                "}"
                );
    toolbarLayout->addWidget(m_lblCurrentFile);

    toolbarLayout->addStretch();

    /*Mode dropdown (replaces old combo box, now styled as toolbar button)*/
    m_btnAgent = new ToolbarButton("Ask", true, this);
    connect(m_btnAgent, &QPushButton::clicked, this, &DevAssistant_MainAI::onAgentMenuRequested);
    toolbarLayout->addWidget(m_btnAgent);

    /*Initialize mode combo (hidden, for logic only)*/
    m_qcmbx_AiMode = new QComboBox(this);
    m_qcmbx_AiMode->addItems({"Ask", "Review", "Replace", "Explain", "Optimize"});
    m_qcmbx_AiMode->hide(); //Hidden, controlled by menu

    /*Model selection (Auto dropdown)*/
    m_btnAuto = new ToolbarButton("Auto", true, this);
    connect(m_btnAuto, &QPushButton::clicked, this, &DevAssistant_MainAI::onAutoMenuRequested);
    toolbarLayout->addWidget(m_btnAuto);

    /*Initialize model combo (hidden, for logic only)*/
    m_cmbxModelComboBox = new QComboBox(this);
    m_cmbxModelComboBox->addItem("Loading available models...");
    m_cmbxModelComboBox->setDisabled(true);
    m_cmbxModelComboBox->hide(); //Hidden, controlled by menu

    /*Settings button (gear icon)*/
    m_btnSettings = new QPushButton(this);
    m_btnSettings->setIcon(QIcon(":/images/settings-white.png"));
    m_btnSettings->setStyleSheet(
                "QPushButton {"
                "    background: transparent;"
                "    border: none;"
                "    color: #ffffff;"
                "    font-size: 16px;"
                "    padding: 4px 8px;"
                "}"
                "QPushButton:hover {"
                "    background: #404142;"
                "    color: #ffffff;"
                "}"
                );
    connect(m_btnSettings, &QToolButton::clicked, this, &DevAssistant_MainAI::onSettingsClicked);
    toolbarLayout->addWidget(m_btnSettings);
}

void DevAssistant_MainAI::setupInputArea()
{
    m_inputContainer = new QWidget(this);
    m_inputContainer->setObjectName("InputContainer");

    /*Main layout for the container*/
    auto *inputLayout = new QVBoxLayout(m_inputContainer);
    inputLayout->setContentsMargins(12, 8, 12, 8);  //Balanced padding
    inputLayout->setSpacing(8);

    /*Text input*/
    m_qtxtedt_UserPrompt = new QTextEdit(this);
    m_qtxtedt_UserPrompt->setPlaceholderText("Ask Developer Assistant");
    m_qtxtedt_UserPrompt->setMaximumHeight(200);  //Allow more expansion
    m_qtxtedt_UserPrompt->setMinimumHeight(24);   //Start smaller
    m_qtxtedt_UserPrompt->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_qtxtedt_UserPrompt->installEventFilter(this);

    /*Set placeholder text color*/
    QPalette palette = m_qtxtedt_UserPrompt->palette();
    palette.setColor(QPalette::PlaceholderText, QColor("#9ca3af"));  //Light gray placeholder
    m_qtxtedt_UserPrompt->setPalette(palette);

    /*Bottom row with button*/
    auto* bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(8);
    bottomRow->setContentsMargins(0, 0, 0, 4);

    bottomRow->addStretch();  /*Pushes button to the right*/

    /*Send button - circular*/
    m_qpshbtn_SendToAI = new QPushButton(this);
    m_qpshbtn_SendToAI->setIcon(QIcon(":/images/send-white.png"));
    m_qpshbtn_SendToAI->setIconSize(QSize(16, 16));
    m_qpshbtn_SendToAI->setFixedSize(32, 32);  //Slightly larger circle
    m_qpshbtn_SendToAI->setCursor(Qt::PointingHandCursor);
    m_qpshbtn_SendToAI->setToolTip("Send message");

    bottomRow->addWidget(m_qpshbtn_SendToAI);

    /*Add widgets to container*/
    inputLayout->addWidget(m_qtxtedt_UserPrompt);
    inputLayout->addLayout(bottomRow);

    /*Initially adjust the height of the input area*/
    adjustInputHeight();

    /*Connect text changed to auto-resize*/
    connect(m_qtxtedt_UserPrompt, &QTextEdit::textChanged,
            this, &DevAssistant_MainAI::adjustInputHeight);
}

/*Add this helper method to auto-resize the input as user types*/
void DevAssistant_MainAI::adjustInputHeight()
{
    if(!m_qtxtedt_UserPrompt)
    {
        return;
    }

    QTextDocument* doc = m_qtxtedt_UserPrompt->document();
    qreal docHeight = doc->size().height();

    /*Adjust height based on content, with min/max bounds*/
    int newHeight = qBound(24, static_cast<int>(docHeight) + 12, 200);
    m_qtxtedt_UserPrompt->setFixedHeight(newHeight);
}

void DevAssistant_MainAI::applyDarkTheme()
{
    /*Main widget dark theme*/
    setStyleSheet(
                "DevAssistant_MainAI {"
                "    background-color: #212222;"
                "}"
                );

    /*Toolbar styling*/
    if(m_toolbar)
    {
        m_toolbar->setStyleSheet(
                    "QWidget {"
                    "    background-color: #212222;"
                    "    border-bottom: 1px solid #ffffff;"
                    "}"
                    );
    }

    /*Input container styling*/
    if(m_inputContainer)
    {
        m_inputContainer->setStyleSheet(
                    "#InputContainer {"
                    "  background-color: #212222;"
                    "  border: 1px solid #d1d5db;"   //Slightly darker border for better visibility
                    "  border-radius: 16px;"         //Rounder corners
                    "  padding: 4px;"                //Inner padding
                    "}"
                    );
    }

    /*Text input styling*/
    if(m_qtxtedt_UserPrompt)
    {
        m_qtxtedt_UserPrompt->setStyleSheet(
                    "QTextEdit {"
                    "  background: transparent;"
                    "  border: none;"
                    "  font-size: 15px;"
                    "  color: #ffffff;"
                    "  padding: 4px 0px;"
                    "  selection-background-color: #e5e7eb;"
                    "}"
                    "QTextEdit:focus {"
                    "  outline: none;"
                    "}"
                    "QScrollBar:vertical {"
                    "  background: transparent;"
                    "  width: 8px;"
                    "  border: none;"
                    "}"
                    "QScrollBar::handle:vertical {"
                    "  background: #d1d5db;"
                    "  border-radius: 4px;"
                    "  min-height: 20px;"
                    "}"
                    "QScrollBar::handle:vertical:hover {"
                    "  background: #9ca3af;"
                    "}"
                    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                    "  height: 0px;"
                    "}"
                    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                    "  background: transparent;"
                    "}"
                    );
    }

    /*Action buttons styling*/
    if(m_qpshbtn_SendToAI)
    {
        m_qpshbtn_SendToAI->setStyleSheet(
                    "QPushButton {"
                    "  background-color: #2563eb;"    //Blue like send button
                    "  border: none;"
                    "  border-radius: 16px;"          //Perfect circle (32px / 2)
                    "  padding: 0px;"
                    "}"
                    "QPushButton:hover {"
                    "  background-color: #1d4ed8;"    //Darker blue on hover
                    "}"
                    "QPushButton:pressed {"
                    "  background-color: #1e40af;"    //Even darker when pressed
                    "}"
                    "QPushButton:disabled {"
                    "  background-color: #d1d5db;"    //Gray when disabled
                    "  opacity: 0.5;"
                    "}"
                    );
    }

    /*Scroll area styling*/
    if(qsa_chatScrollArea)
    {
        qsa_chatScrollArea->setStyleSheet(
                    "QScrollArea {"
                    "    background-color: #212222;"
                    "    border: none;"
                    "}"
                    "QScrollBar:vertical {"
                    "    background: #212222;"
                    "    width: 12px;"
                    "    border-radius: 6px;"
                    "}"
                    "QScrollBar::handle:vertical {"
                    "    background: #555555;"
                    "    border-radius: 6px;"
                    "    min-height: 20px;"
                    "}"
                    "QScrollBar::handle:vertical:hover {"
                    "    background: #666666;"
                    "}"
                    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                    "    height: 0px;"
                    "}"
                    );
    }
}

/* ============================================================
 * EVENT FILTER - CTRL+ENTER TO SEND
 * ============================================================ */
bool DevAssistant_MainAI::eventFilter(QObject* obj, QEvent* event)
{
    if((obj == m_qtxtedt_UserPrompt) && (event->type() == QEvent::KeyPress))
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        /*Ctrl+Enter to send*/
        if((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) &&
                keyEvent->modifiers() & Qt::ControlModifier)
        {
            SLOT_UpdatesUserRequestToAi();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

/* ============================================================
 * MENU HANDLERS
 * ============================================================ */
void DevAssistant_MainAI::onAgentMenuRequested()
{
    QMenu menu(this);
    menu.setStyleSheet(
                "QMenu {"
                "    background-color: #404142;"
                "    color: #ffffff;"
                "    border: 1px solid #555555;"
                "    border-radius: 4px;"
                "    padding: 4px;"
                "}"
                "QMenu::item {"
                "    padding: 6px 24px 6px 12px;"
                "    border-radius: 3px;"
                "}"
                "QMenu::item:selected {"
                "    background-color: #4a9eff;"
                "    color: white;"
                "}"
                );

    /*Add mode options*/
    QAction* askAction = menu.addAction("Ask");
    QAction* reviewAction = menu.addAction("Review");
    QAction* replaceAction = menu.addAction("Replace");
    QAction* explainAction = menu.addAction("Explain");
    QAction* optimizeAction = menu.addAction("Optimize");

    /*Mark current selection*/
    QString currentMode = m_qcmbx_AiMode->currentText();
    if(currentMode == "Ask")
    {
        askAction->setCheckable(true), askAction->setChecked(true);
    }
    else if(currentMode == "Review")
    {
        reviewAction->setCheckable(true), reviewAction->setChecked(true);
    }
    else if(currentMode == "Replace")
    {
        replaceAction->setCheckable(true), replaceAction->setChecked(true);
    }
    else if(currentMode == "Explain")
    {
        explainAction->setCheckable(true), explainAction->setChecked(true);
    }
    else if(currentMode == "Optimize")
    {
        optimizeAction->setCheckable(true), optimizeAction->setChecked(true);
    }

    QAction* selected = menu.exec(m_btnAgent->mapToGlobal(QPoint(0, m_btnAgent->height())));

    if(selected)
    {
        QString mode = selected->text();
        m_qcmbx_AiMode->setCurrentText(mode);
        m_btnAgent->setText(mode);
    }
}

void DevAssistant_MainAI::onAutoMenuRequested()
{
    QMenu menu(this);
    menu.setStyleSheet(
                "QMenu {"
                "    background-color: #404142;"
                "    color: #ffffff;"
                "    border: 1px solid #555555;"
                "    border-radius: 4px;"
                "    padding: 4px;"
                "}"
                "QMenu::item {"
                "    padding: 6px 24px 6px 12px;"
                "    border-radius: 3px;"
                "}"
                "QMenu::item:selected {"
                "    background-color: #4a9eff;"
                "    color: white;"
                "}"
                );

    /*Add available models*/
    QString currentModel = m_cmbxModelComboBox->currentText();

    for(int i = 0; i < m_cmbxModelComboBox->count(); ++i)
    {
        QString modelName = m_cmbxModelComboBox->itemText(i);
        QAction* action = menu.addAction(modelName);

        if(modelName == currentModel)
        {
            action->setCheckable(true);
            action->setChecked(true);
        }
    }

    QAction* selected = menu.exec(m_btnAuto->mapToGlobal(QPoint(0, m_btnAuto->height())));

    if(selected)
    {
        QString modelName = selected->text();
        m_cmbxModelComboBox->setCurrentText(modelName);

        /*Update button text (show short name)*/
        if(modelName.length() > 15)
        {
            m_btnAuto->setText(modelName.left(12) + "...");
        }
        else
        {
            m_btnAuto->setText(modelName);
        }
    }
}

void DevAssistant_MainAI::onSettingsClicked()
{
    QMenu menu(this);
    menu.setStyleSheet(
                "QMenu {"
                "    background-color: #404142;"
                "    color: #ffffff;"
                "    border: 1px solid #555555;"
                "    border-radius: 4px;"
                "    padding: 4px;"
                "}"
                "QMenu::item {"
                "    padding: 6px 24px 6px 12px;"
                "    border-radius: 3px;"
                "}"
                "QMenu::item:selected {"
                "    background-color: #4a9eff;"
                "    color: white;"
                "}"
                );

    menu.addAction("Preferences...");
    menu.addAction("Model Settings...");
    menu.addSeparator();
    menu.addAction("Clear Chat");
    menu.addSeparator();
    menu.addAction("About");

    QAction* selected = menu.exec(m_btnSettings->mapToGlobal(QPoint(0, m_btnSettings->height())));

    if(selected && selected->text() == "Clear Chat")
    {
        /*Clear all messages*/
        while(qvbl_chatLayout->count() > 0)
        {
            QLayoutItem* item = qvbl_chatLayout->takeAt(0);
            if(item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    }
}

/* ============================================================
 * USER → AI REQUEST (ORIGINAL LOGIC PRESERVED)
 * ============================================================ */
void DevAssistant_MainAI::SLOT_UpdatesUserRequestToAi()
{
    if(m_bAiActive)
    {
        return;
    }

    QString qsPrompt;
    QString qsMode = m_qcmbx_AiMode->currentText();

    if(qsMode == "Ask")
    {
        qsPrompt = m_qtxtedt_UserPrompt->toPlainText().trimmed();
        if(qsPrompt.isEmpty())
        {
            qvbl_chatLayout->addWidget(
                        new AITextWidget("System: Please enter a prompt."));
            return;
        }

        qvbl_chatLayout->addWidget(
                    new UserMessageWidget(qsPrompt.toHtmlEscaped()));
        m_qtxtedt_UserPrompt->clear();
        m_bCodeReplacement = false;
    }
    else
    {
        QString qsSelectedCode = DevAssistant_EditorUtils::getSelectedTextQt();

        /*To get the Selected Code for Diff Preview*/
        m_qsSelectedCodeFromQtEditor = qsSelectedCode;

        if(qsSelectedCode.isEmpty())
        {
            qvbl_chatLayout->addWidget(
                        new AITextWidget("System: Select code first."));
            return;
        }

        qvbl_chatLayout->addWidget(
                    new AICodeBlockWidget(qsSelectedCode));

        if(qsMode == "Review")
        {
            qsPrompt = DevAssistant_RvwPrptBdr::buildPromptReviewCodeGiveComments(
                        qsSelectedCode, DevAssistant_CdStdLder::loadOfficeCodingStandardsFromFile());

            qvbl_chatLayout->addWidget(
                        new AICodeBlockWidget(qsPrompt));

            m_bCodeReplacement = false;
        }

        if(qsMode == "Replace")
        {
            qsPrompt = DevAssistant_RvwPrptBdr::buildPromptReplaceCode(
                        qsSelectedCode,
                        DevAssistant_CdStdLder::loadOfficeCodingStandardsFromFile());
            qvbl_chatLayout->addWidget(
                        new AICodeBlockWidget(qsPrompt));
            m_bCodeReplacement = true;
        }
        else if(qsMode == "Explain")
        {
            qsPrompt = DevAssistant_RvwPrptBdr::buildPromptExplainCode(qsSelectedCode);
            qvbl_chatLayout->addWidget(
                        new AICodeBlockWidget(qsPrompt));
            m_bCodeReplacement = false;
        }
        else if(qsMode == "Optimize")
        {
            qsPrompt = DevAssistant_RvwPrptBdr::buildPromptOptimizeCode(qsSelectedCode);
            qvbl_chatLayout->addWidget(
                        new AICodeBlockWidget(qsPrompt));
            m_bCodeReplacement = true;
        }
    }

    m_obj_devassist_client->m_selectedModel = m_cmbxModelComboBox->currentText();
    m_obj_devassist_client->SendPromptoAi(qsPrompt);

    m_qpshbtn_SendToAI->setEnabled(false);
    m_btnSettings->setEnabled(false);
    scrollToBottom();
}

/* ============================================================
 * AI RESPONSE START (TEMP BUBBLE) - ORIGINAL LOGIC
 * ============================================================ */
void DevAssistant_MainAI::SLOT_onAIResponseStarted()
{
    m_bAiActive = true;
    m_qs_aiFullResponse.clear();

    m_AIloadingAnimationWidget = new AITextWidget();

    if(m_AIloadingAnimationWidget)
    {
        m_AIloadingAnimationWidget->setMovie();

        qvbl_chatLayout->addWidget(m_AIloadingAnimationWidget);
        scrollToBottom();
    }
}

/* ============================================================
 * STREAMING CHUNKS (DATA ONLY) - ORIGINAL LOGIC
 * ============================================================ */
void DevAssistant_MainAI::SLOT_ReceivePartialAiOutput(const QString &qsChunk)
{
    if(!m_bAiActive)
    {
        return;
    }

    /*Remove temporary bubble - only once*/
    if(m_AIloadingAnimationWidget)
    {
        m_AIloadingAnimationWidget->removeMovie();
        qvbl_chatLayout->removeWidget(m_AIloadingAnimationWidget);
        m_AIloadingAnimationWidget->deleteLater();
        m_AIloadingAnimationWidget = nullptr;
    }

    /*Add chunk to buffer (IMPORTANT: handle partial fence markers)*/
    m_streamBuffer += qsChunk;

    /*Process complete fence markers only*/
    while(true)
    {
        if(!m_bInsideCodeBlockFlag)
        {
            /*Looking for opening ```*/
            int fenceIdx = m_streamBuffer.indexOf("```");
            if(fenceIdx < 0)
            {
                /*No fence found - check if we might have partial fence at end*/
                if(m_streamBuffer.endsWith("`") || m_streamBuffer.endsWith("``"))
                {
                    /*Might be partial fence - wait for next chunk
                      But output everything except the trailing backticks*/
                    int lastSafeIdx = m_streamBuffer.length();
                    if(m_streamBuffer.endsWith("``"))
                    {
                        lastSafeIdx -= 2;
                    }
                    else if(m_streamBuffer.endsWith("`"))
                    {
                        lastSafeIdx -= 1;
                    }

                    QString safeText = m_streamBuffer.left(lastSafeIdx);
                    if(!safeText.isEmpty())
                    {
                        appendTextInTextWidget(safeText);
                        m_streamBuffer.remove(0, lastSafeIdx);
                    }
                }
                else
                {
                    /*Safe to output everything*/
                    appendTextInTextWidget(m_streamBuffer);
                    m_streamBuffer.clear();
                }
                break;
            }

            /*Text before fence*/
            if(fenceIdx > 0)
            {
                appendTextInTextWidget(m_streamBuffer.left(fenceIdx));
            }

            /*Enter code mode*/
            m_streamBuffer.remove(0, fenceIdx + 3);
            m_bInsideCodeBlockFlag = true;
            m_bWaitingForLanguage = true;
            finishTextBlockWidget();
        }
        else
        {
            /*If we're still waiting for the language identifier*/
            if(m_bWaitingForLanguage)
            {
                int newlineIdx = m_streamBuffer.indexOf('\n');
                if(newlineIdx < 0)
                {
                    /*No newline yet - wait for more data*/
                    /*But don't wait forever - if buffer gets too long, assume no language*/
                    if(m_streamBuffer.length() > 30)
                    {
                        /*No language specified, treat buffer as code*/
                        m_currentCodeLanguage = "Code";
                        m_bWaitingForLanguage = false;
                        /*Don't break - continue processing the code(=*/
                    }
                    else
                    {
                        /*Wait for more chunks*/
                        break;
                    }
                }
                else
                {
                    /*Found newline - extract language*/
                    QString language = m_streamBuffer.left(newlineIdx).trimmed();
                    m_streamBuffer.remove(0, newlineIdx + 1);

                    /*Map language identifiers*/
                    if(language.isEmpty())
                        m_currentCodeLanguage = "Code";
                    else if(language == "cpp" || language == "c++")
                        m_currentCodeLanguage = "C++";
                    else if(language == "py" || language == "python")
                        m_currentCodeLanguage = "Python";
                    else if(language == "js" || language == "javascript")
                        m_currentCodeLanguage = "JavaScript";
                    else if(language == "ts" || language == "typescript")
                        m_currentCodeLanguage = "TypeScript";
                    else if(language == "java")
                        m_currentCodeLanguage = "Java";
                    else if(language == "cs" || language == "csharp")
                        m_currentCodeLanguage = "C#";
                    else if(language == "html")
                        m_currentCodeLanguage = "HTML";
                    else if(language == "css")
                        m_currentCodeLanguage = "CSS";
                    else if(language == "sql")
                        m_currentCodeLanguage = "SQL";
                    else if(language == "bash" || language == "sh")
                        m_currentCodeLanguage = "Bash";
                    else if(language == "json")
                        m_currentCodeLanguage = "JSON";
                    else if(language == "xml")
                        m_currentCodeLanguage = "XML";
                    else if(language == "md" || language == "markdown")
                        m_currentCodeLanguage = "Markdown";
                    else if(language == "qml")
                        m_currentCodeLanguage = "QML";
                    else
                        m_currentCodeLanguage = language.toUpper(); // Use as-is, uppercase

                    m_bWaitingForLanguage = false;
                    /*Continue to process remaining code*/
                }
            }

            /*Looking for closing ```*/
            int fenceIdx = m_streamBuffer.indexOf("```");
            if(fenceIdx < 0)
            {
                /*No closing fence yet - check for partial*/
                if(m_streamBuffer.endsWith("`") || m_streamBuffer.endsWith("``"))
                {
                    /*Might be partial fence - wait for next chunk*/
                    int lastSafeIdx = m_streamBuffer.length();
                    if(m_streamBuffer.endsWith("``"))
                    {
                        lastSafeIdx -= 2;
                    }
                    else if(m_streamBuffer.endsWith("`"))
                    {
                        lastSafeIdx -= 1;
                    }

                    QString safeCode = m_streamBuffer.left(lastSafeIdx);
                    if(!safeCode.isEmpty())
                    {
                        appendCodeInCodeBlockWidget(safeCode);
                        m_streamBuffer.remove(0, lastSafeIdx);
                    }
                }
                else
                {
                    /*Safe to output all code*/
                    appendCodeInCodeBlockWidget(m_streamBuffer);
                    m_streamBuffer.clear();
                }
                break;
            }

            /*Code before closing fence*/
            if(fenceIdx > 0)
            {
                appendCodeInCodeBlockWidget(m_streamBuffer.left(fenceIdx));
            }

            /*Exit code mode*/
            m_streamBuffer.remove(0, fenceIdx + 3);
            m_bInsideCodeBlockFlag = false;
            m_bWaitingForLanguage = false;
            finishCodeBlockWidget();
            m_currentCodeLanguage = "Code"; //Reset
        }
    }

    scrollToBottom();
}

void DevAssistant_MainAI::finishTextBlockWidget()
{
    m_CurrentStreamingTextWidget = nullptr;
}

void DevAssistant_MainAI::appendCodeInCodeBlockWidget(const QString& code)
{
    if(code.isEmpty())
    {
        return;
    }

     /*To get the Final Ai Generated Code for Diff Preview*/
    m_qsFinalAiGeneratedCode += code;

    if(!m_CurrentStreamingCodeWidget)
    {
        m_CurrentStreamingCodeWidget = new AICodeBlockWidget("", m_currentCodeLanguage);
        qvbl_chatLayout->addWidget(m_CurrentStreamingCodeWidget);
    }

    m_CurrentStreamingCodeWidget->appendCodeInTextEdit(code);
    scrollToBottom();
}

void DevAssistant_MainAI::finishCodeBlockWidget()
{
    m_CurrentStreamingCodeWidget = nullptr;
}

void DevAssistant_MainAI::appendTextInTextWidget(const QString& text)
{
    if(text.isEmpty())
    {
        return;
    }

    if(!m_CurrentStreamingTextWidget)
    {
        m_CurrentStreamingTextWidget = new AITextWidget("");
        qvbl_chatLayout->addWidget(m_CurrentStreamingTextWidget);
    }

    m_CurrentStreamingTextWidget->appendText(text);
    scrollToBottom();
}

/* ============================================================
 * AI RESPONSE FINISHED (FINAL RENDER) - ORIGINAL LOGIC
 * ============================================================ */
void DevAssistant_MainAI::SLOT_onAIStreamingResponseFinished()
{
    m_bAiActive = false;
    m_qpshbtn_SendToAI->setEnabled(true);
    m_btnSettings->setEnabled(true);

    /*Flush any remaining buffer*/
    if(!m_streamBuffer.isEmpty())
    {
        if(m_bInsideCodeBlockFlag)
        {
            appendCodeInCodeBlockWidget(m_streamBuffer);
        }
        else
        {
            appendTextInTextWidget(m_streamBuffer);
        }
        m_streamBuffer.clear();
    }

    finishCodeBlockWidget();
    finishTextBlockWidget();

    /*Reset flag*/
    m_bInsideCodeBlockFlag = false;

    if(m_bCodeReplacement)
    {
        DevAssistant_DiffViewer *diffViewer  =
            new DevAssistant_DiffViewer(
                m_qsSelectedCodeFromQtEditor,
                m_qsFinalAiGeneratedCode,
                this);

        diffViewer->setAttribute(Qt::WA_DeleteOnClose);

        connect(diffViewer ,
                &DevAssistant_DiffViewer::SGNL_replaceConfirmed,
                this,
                [this](const QString &qsNewCode)
        {
            DevAssistant_EditorUtils::replaceSelectedTextwithNewTextQt(qsNewCode);
        });

        diffViewer ->exec();

        m_qsFinalAiGeneratedCode.clear();
    }

    m_bCodeReplacement = false;

    scrollToBottom();
}

void DevAssistant_MainAI::SLOT_AiThrowedError(const QString &error)
{
    m_bAiActive = false;
    m_qpshbtn_SendToAI->setEnabled(true);
    m_btnSettings->setEnabled(true);

    /*Remove temporary bubble*/
    if(m_AIloadingAnimationWidget)
    {
        m_AIloadingAnimationWidget->removeMovie();

        qvbl_chatLayout->removeWidget(m_AIloadingAnimationWidget);
        m_AIloadingAnimationWidget->deleteLater();
        m_AIloadingAnimationWidget = nullptr;
    }

    if(error.isEmpty())
    {
        m_currentAITextWidget = new AITextWidget("System: AI returned empty response.");
    }
    else
    {
        m_currentAITextWidget = new AITextWidget(QString("System: %1").arg(error));
    }
    qvbl_chatLayout->addWidget(m_currentAITextWidget);
    return;
}

/* ============================================================
 * MODELS - ORIGINAL LOGIC
 * ============================================================ */
void DevAssistant_MainAI::onModelsFetched(const QStringList &qslModels)
{
    m_cmbxModelComboBox->clear();
    if(qslModels.isEmpty())
    {
        m_cmbxModelComboBox->addItem("No models available");
        m_btnAuto->setText("Auto");
    }
    else
    {
        m_cmbxModelComboBox->addItems(qslModels);
        m_cmbxModelComboBox->setEnabled(true);

        /*Update Auto button with first model name*/
        if(!qslModels.isEmpty())
        {
            QString firstModel = qslModels.first();
            if(firstModel.length() > 15)
            {
                m_btnAuto->setText(firstModel.left(12) + "...");
            }
            else
            {
                m_btnAuto->setText(firstModel);
            }
        }
    }
}

/* ============================================================
 * HELPERS - ORIGINAL LOGIC
 * ============================================================ */
void DevAssistant_MainAI::scrollToBottom()
{
    QTimer::singleShot(0, qsa_chatScrollArea, [this]() {
        qsa_chatScrollArea->verticalScrollBar()->setValue(
                    qsa_chatScrollArea->verticalScrollBar()->maximum());
    });
}

/* ============================================================
 * PUBLIC METHOD TO UPDATE CURRENT FILE DISPLAY
 * ============================================================ */
void DevAssistant_MainAI::setCurrentFile(const QString& filename)
{
    if(m_lblCurrentFile)
    {
        m_lblCurrentFile->setText(filename);
    }
}

#include <stdio.h>

int main() {
    int iNum1 = 0;
    int iNum2 = 0;
    int iSum = 0;

    printf("Enter the first number: ");
    if (scanf("%d", &iNum1) != 1 || scanf("%d", &iNum2) != 1) {
        return 1;
    }

    iSum = iNum1 + iNum2;

    printf("The sum of %d and %d is %d\n", iNum1, iNum2, iSum);

    return 0;
}
