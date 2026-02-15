#include "devassistant_suggestor.h"
#include "devassistant_global.h"

using namespace DevAssistant::Internal;

DevAssistantSuggestor::DevAssistantSuggestor(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    /*Header*/
    auto headerLayout = new QHBoxLayout;
    auto titleLabel = new QLabel(tr("AI Code Suggestions"), this);
    QFont font = titleLabel->font();
    font.setBold(true);
    titleLabel->setFont(font);
    headerLayout->addWidget(titleLabel);

    m_cmbxModelComboBox = new QComboBox(this);
    m_cmbxModelComboBox->setMinimumWidth(150);

    /*Start with a loading message*/
    m_cmbxModelComboBox->addItem("Loading models...");
    m_cmbxModelComboBox->setEnabled(false);
    headerLayout->addWidget(m_cmbxModelComboBox, 2);

    m_qpshbtnRefreshButton = new QPushButton(tr("Refresh"), this);
    m_qpshbtnRefreshButton->setMaximumWidth(80);
    headerLayout->addWidget(m_qpshbtnRefreshButton);

    mainLayout->addLayout(headerLayout);

    /*Status label*/
    m_lblStatusLabel = new QLabel(this);
    m_lblStatusLabel->setWordWrap(true);
    m_lblStatusLabel->setStyleSheet("QLabel { color: #666; font-size: 11px; padding: 3px; }");
    m_lblStatusLabel->setText(tr("Ready"));
    mainLayout->addWidget(m_lblStatusLabel);

    /*Separator line*/
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    /*Splitter for suggestions list and preview*/
    auto splitter = new QSplitter(Qt::Vertical, this);

    /*Suggestions list with label*/
    QWidget *listWidget = new QWidget(this);
    auto listLayout = new QVBoxLayout(listWidget);
    listLayout->setContentsMargins(0, 0, 0, 0);

    auto suggestionsLabel = new QLabel(tr("Suggestions:"), listWidget);
    listLayout->addWidget(suggestionsLabel);

    m_qlstwdgtSuggestionsList = new QListWidget(listWidget);
    m_qlstwdgtSuggestionsList->setAlternatingRowColors(true);
    listLayout->addWidget(m_qlstwdgtSuggestionsList);

    splitter->addWidget(listWidget);

    /*Preview section*/
    QWidget *previewWidget = new QWidget(this);
    auto previewLayout = new QVBoxLayout(previewWidget);
    previewLayout->setContentsMargins(0, 0, 0, 0);

    auto previewLabel = new QLabel(tr("Preview:"), previewWidget);
    previewLayout->addWidget(previewLabel);

    m_qtxtedtPreviewText = new QTextEdit(previewWidget);
    m_qtxtedtPreviewText->setReadOnly(true);
    QFont monoFont("Courier");
    monoFont.setStyleHint(QFont::Monospace);
    m_qtxtedtPreviewText->setFont(monoFont);
    previewLayout->addWidget(m_qtxtedtPreviewText);

    m_qpshbtnApplyButton = new QPushButton(tr("Apply to Editor"), previewWidget);
    m_qpshbtnApplyButton->setEnabled(false);
    previewLayout->addWidget(m_qpshbtnApplyButton);

    splitter->addWidget(previewWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    /*Connections*/
    connect(m_qlstwdgtSuggestionsList, &QListWidget::itemClicked,
            this, &DevAssistantSuggestor::onSuggestionSelected);
    connect(m_qlstwdgtSuggestionsList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *) { onApplySuggestion(); });
    connect(m_qpshbtnApplyButton, &QPushButton::clicked,
            this, &DevAssistantSuggestor::onApplySuggestion);
    connect(m_qpshbtnRefreshButton, &QPushButton::clicked,
            this, &DevAssistantSuggestor::SGNL_refreshRequested);
    connect(m_cmbxModelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DevAssistantSuggestor::SLOT_modelChanged);

    /*Initial status*/
    setStatus(tr("Initializing..."));

#if 0
    /*Add some sample suggestions for testing*/
    QStringList samples = {
        "//TODO: Add error handling here",
        "if(ptr == nullptr) {\n    return false;\n}",
        "//Consider using smart pointers"
    };
    setSuggestions(samples);
#endif
}

DevAssistantSuggestor::~DevAssistantSuggestor()
{
}

void DevAssistantSuggestor::addSuggestion(const QString &qsSuggestion)
{
    if(!qsSuggestion.isEmpty())
    {
        QString qsDisplayText = qsSuggestion;
        if(qsDisplayText.length() > 60)
        {
            qsDisplayText = qsDisplayText.left(60) + "...";
        }

        auto aItem = new QListWidgetItem(qsDisplayText, m_qlstwdgtSuggestionsList);
        aItem->setData(Qt::UserRole, qsSuggestion);
        aItem->setToolTip(qsSuggestion);
    }
}

void DevAssistantSuggestor::setSuggestions(const QStringList &qslSuggestions)
{
    clearSuggestions();

    if(qslSuggestions.isEmpty())
    {
        setStatus(tr("No suggestions available"), false);
        return;
    }

    for(const QString &qsSuggestion : qslSuggestions)
    {
        addSuggestion(qsSuggestion);
    }

    setStatus(tr("Ready - %1 suggestions").arg(qslSuggestions.count()));
}

void DevAssistantSuggestor::clearSuggestions()
{
    m_qlstwdgtSuggestionsList->clear();
    m_qtxtedtPreviewText->clear();
    m_qpshbtnApplyButton->setEnabled(false);
    m_qsCurrentSuggestion.clear();
}

QString DevAssistantSuggestor::currentModel() const
{
    if((m_cmbxModelComboBox->count() > 0) && (m_cmbxModelComboBox->isEnabled()))
    {
        return m_cmbxModelComboBox->currentText();
    }

    return QString();
}

void DevAssistantSuggestor::setAvailableModels(const QStringList &qslModels)
{
    QString qsCurrentModel = m_cmbxModelComboBox->currentText();

    m_cmbxModelComboBox->clear();
    m_cmbxModelComboBox->setEnabled(true);

    if(qslModels.isEmpty())
    {
        m_cmbxModelComboBox->addItem("No models available");
        m_cmbxModelComboBox->setEnabled(false);
        setStatus(tr("No models found. Is Ollama running?"), true);
        return;
    }

    m_cmbxModelComboBox->addItems(qslModels);

    /*Try to restore previous selection*/
    int iIndex = m_cmbxModelComboBox->findText(qsCurrentModel);
    if(iIndex >= 0)
    {
        m_cmbxModelComboBox->setCurrentIndex(iIndex);
    }
    else
    {
        /*Select first model by default*/
        m_cmbxModelComboBox->setCurrentIndex(0);
    }

    setStatus(tr("Connected - %1 models available").arg(qslModels.count()));
}

void DevAssistantSuggestor::setStatus(const QString &qsStatus, bool bIsError)
{
    if(m_lblStatusLabel)
    {
        m_lblStatusLabel->setText(qsStatus);

        if(bIsError)
        {
            m_lblStatusLabel->setStyleSheet("QLabel { color: #c00; font-size: 11px; padding: 3px; }");
        }
        else
        {
            m_lblStatusLabel->setStyleSheet("QLabel { color: #666; font-size: 11px; padding: 3px; }");
        }
    }
}

void DevAssistantSuggestor::showLoading(bool bLoading)
{
    if(bLoading)
    {
        setStatus(tr("Generating suggestions..."));
        m_qpshbtnRefreshButton->setEnabled(false);
    }
    else
    {
        setStatus(tr("Ready"));
        m_qpshbtnRefreshButton->setEnabled(true);
    }
}

void DevAssistantSuggestor::onSuggestionSelected(QListWidgetItem *qlwdgtItem)
{
    if(!qlwdgtItem)
    {
        return;
    }

    m_qsCurrentSuggestion = qlwdgtItem->data(Qt::UserRole).toString();
    m_qtxtedtPreviewText->setPlainText(m_qsCurrentSuggestion);
    m_qpshbtnApplyButton->setEnabled(true);
}

void DevAssistantSuggestor::onApplySuggestion()
{
    if(m_qsCurrentSuggestion.isEmpty())
    {
        return;
    }

    auto aEditor = Core::EditorManager::currentEditor();
    if(!aEditor)
    {
        setStatus(tr("No active editor"), true);
        return;
    }

    auto aTextEditor = qobject_cast<TextEditor::BaseTextEditor *>(aEditor);
    if(!aTextEditor)
    {
        setStatus(tr("Current editor is not a text editor"), true);
        return;
    }

    auto aEditorWidget = aTextEditor->editorWidget();
    if(aEditorWidget)
    {
        QTextCursor qtxtCursor = aEditorWidget->textCursor();
        qtxtCursor.insertText(m_qsCurrentSuggestion);
        aEditorWidget->setTextCursor(qtxtCursor);
        aEditorWidget->setFocus();

        setStatus(tr("Suggestion applied successfully"));
    }
    else
    {
        setStatus(tr("Could not access editor widget"), true);
    }

    emit SGNL_suggestionApplied(m_qsCurrentSuggestion);
}

void DevAssistantSuggestor::SLOT_modelChanged(int index)
{
    Q_UNUSED(index)

    QString qsModel = m_cmbxModelComboBox->currentText();
    if((!qsModel.isEmpty()) && (qsModel != "Loading models...") && (qsModel != "No models available"))
    {
        emit SGNL_modelChanged(qsModel);
        setStatus(tr("Model changed to: %1").arg(qsModel));
    }
}
