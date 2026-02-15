#ifndef DEVASSISTANT_MAINAI_H
#define DEVASSISTANT_MAINAI_H

#include "devassistant_global.h"

#include "devassistant_client.h"
#include "devassistant_editorutils.h"
#include "devassistant_cdstdlder.h"
#include "devassistant_rvwprptbdr.h"
#include "devassistant_ollamaservice.h"
#include "devassistant_diffviewer.h"

/* ============================================================
 * USER MESSAGE WIDGET
 * ============================================================ */
class UserMessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserMessageWidget(const QString& qsText, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* aLayout = new QHBoxLayout(this);
        aLayout->setContentsMargins(0, 0, 0, 0);
        aLayout->setSpacing(0);

        //Left spacer pushes bubble to the right
        aLayout->addStretch();

        QLabel* qlabel = new QLabel(this);
        qlabel->setText(qsText);
        qlabel->setWordWrap(true);
        qlabel->setTextFormat(Qt::PlainText);

        /*Enable text selection*/
        qlabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

        qlabel->setStyleSheet(
                    "background:#dddddd;"
                    "color:black;"
                    "padding:8px;"
                    "border-radius:6px;"
                    );

        //IMPORTANT: size control
        qlabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        qlabel->setMaximumWidth(420);   // chat bubble width limit
        qlabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        aLayout->addWidget(qlabel);
    }
};

/* ============================================================
 * AI TEXT MESSAGE WIDGET (CORRECT)
 * ============================================================ */
class AITextWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AITextWidget(const QString& initialText = "",
                          QWidget* parent = nullptr)
        : QWidget(parent),
          m_qtxtbView(new QTextBrowser(this)),
          m_lblLoading(new QLabel(this))
    {
        movie = new QMovie(":/images/d-letter-squares-gray.gif");

        auto* aLayout = new QHBoxLayout(this);
        aLayout->setContentsMargins(0, 0, 0, 0);
        aLayout->setSpacing(0);

        m_qtxtbView->setReadOnly(true);
        m_qtxtbView->setFrameShape(QFrame::NoFrame);
        m_qtxtbView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_qtxtbView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        //IMPORTANT: allow shrinking
        m_qtxtbView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_qtxtbView->setStyleSheet(
                    "QTextBrowser {"
                    "  background:transparent;"
                    "  color:#000000;"
                    "  padding:8px;"
                    "  border-radius:6px;"
                    "}"
                    );

        aLayout->addWidget(m_qtxtbView);
        setText(initialText);

        m_lblLoading->setFrameShape(QFrame::NoFrame);

        //IMPORTANT: allow shrinking
        m_lblLoading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_lblLoading->setStyleSheet(
                    "QLabel {"
                    "  background:transparent;"
                    "  color:#000000;"
                    "  padding:8px;"
                    "  border-radius:6px;"
                    "}"
                    );

        aLayout->addWidget(m_lblLoading);
    }

    void setMovie()
    {
        if(movie)
        {
            m_lblLoading->show();
            m_qtxtbView->hide();

            movie->setSpeed(500);
            movie->setScaledSize(QSize(23, 23));
            movie->start();

            m_lblLoading->setMovie(movie);
        }
    }

    void removeMovie()
    {
        if(movie)
        {
            movie->stop();
        }
    }

    void setText(const QString& qsText)
    {
        m_qtxtbView->show();
        m_lblLoading->hide();

        m_qtxtbView->setPlainText(qsText);
        adjustHeight();
    }

    void appendText(const QString& qsText)
    {
        m_qtxtbView->show();
        m_lblLoading->hide();

        m_qtxtbView->moveCursor(QTextCursor::End);
        m_qtxtbView->insertPlainText(qsText);
        adjustHeight();
    }

    QMovie *movie;

protected:
    //Ensure height recalculates after layout
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        adjustHeight();
    }

private:
    void adjustHeight()
    {
        if(!m_qtxtbView->document())
        {
            return;
        }

        const int iPadding = 12;

        qreal qrlDocHeight =
                m_qtxtbView->document()->documentLayout()->documentSize().height();

        int iFinalHeight = qMax(30, int(qrlDocHeight) + iPadding);

        m_qtxtbView->setMinimumHeight(iFinalHeight);
        m_qtxtbView->setMaximumHeight(iFinalHeight);
    }

    QTextBrowser* m_qtxtbView;
    QLabel* m_lblLoading;
};

/* ============================================================
 * AI CODE BLOCK WIDGET
 * ============================================================ */
class AICodeBlockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AICodeBlockWidget(const QString& code, const QString& language = "Code", QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* aLayout = new QVBoxLayout(this);

        /*Track last active editor*/
        connect(qApp, &QApplication::focusChanged,
                this, [this](QWidget*, QWidget* now) {
            if(auto* ed = qobject_cast<QPlainTextEdit*>(now))
            {
                m_qplntxtLastEditor = ed;
            }
        });

        /*Header*/
        auto* aHeader = new QHBoxLayout;

        /*Language label*/
        m_qlblLanguage = new QLabel(language, this);
        m_qlblLanguage->setStyleSheet(
                    "QLabel {"
                    "  color: #666666;"
                    "  font-weight: bold;"
                    "  font-size: 12px;"
                    "  padding: 2px 4px;"
                    "}"
                    );

        QPushButton* qpshbtnInsertBtn = new QPushButton("Insert At Cursor", this);
        QPushButton* qpshbtnCopyBtn   = new QPushButton("⧉ Copy", this);

        QString qsBtnStyle =
                "QPushButton { background:#dddddd; color:#000000; padding: 4px 8px; border-radius: 4px; }"
                "QPushButton:hover { background: #cccccc; }";

        qpshbtnInsertBtn->setStyleSheet(qsBtnStyle);
        qpshbtnCopyBtn->setStyleSheet(qsBtnStyle);

        connect(qpshbtnCopyBtn, &QPushButton::clicked, this, [this]()
        {
            QGuiApplication::clipboard()->setText(m_editor->toPlainText());
        });

        connect(qpshbtnInsertBtn, &QPushButton::clicked, this, [this]()
        {
            if(!m_qplntxtLastEditor)
            {
                return;
            }

            QTextCursor qtxtCursor = m_qplntxtLastEditor->textCursor();
            qtxtCursor.insertText(m_editor->toPlainText());
            m_qplntxtLastEditor->setTextCursor(qtxtCursor);
            m_qplntxtLastEditor->setFocus();
        });

        aHeader->addWidget(m_qlblLanguage);
        aHeader->addStretch();
        aHeader->addWidget(qpshbtnInsertBtn);
        aHeader->addWidget(qpshbtnCopyBtn);

        /*Code viewer*/
        m_editor = new QPlainTextEdit(this);
        m_editor->setPlainText(code);
        m_editor->setReadOnly(true);
        m_editor->setStyleSheet(
                    "QPlainTextEdit {"
                    "  background:#2d2d2d;"
                    "  color:#ffffff;"
                    "  border:1px solid #444;"
                    "  border-radius:6px;"
                    "  padding: 8px;"
                    "  font-family: 'Courier New', monospace;"
                    "  font-size: 13px;"
                    "}"
                    );

        aLayout->addLayout(aHeader);
        aLayout->addWidget(m_editor);
    }

    void appendCodeInTextEdit(const QString& code)
    {
        if(!m_editor)
            return;

        m_editor->insertPlainText(code);
        m_editor->moveCursor(QTextCursor::End);
    }

    /*Method to update language label*/
    void setLanguage(const QString& language)
    {
        if(m_qlblLanguage)
        {
            m_qlblLanguage->setText(language);
        }
    }
private:
    QPlainTextEdit* m_qplntxtLastEditor = nullptr;
    QPlainTextEdit* m_editor = nullptr;
    QLabel* m_qlblLanguage = nullptr;
};

/* ============================================================
 * CUSTOM TOOLBAR BUTTON (Flat style with dropdown indicator)
 * ============================================================ */
class ToolbarButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ToolbarButton(const QString& text, bool hasDropdown = false, QWidget* parent = nullptr)
        : QPushButton(text, parent), m_hasDropdown(hasDropdown)
    {
        setFlat(true);
        setCursor(Qt::PointingHandCursor);
        updateStyleSheet();
    }

    void setHasDropdown(bool has)
    {
        m_hasDropdown = has;
        updateStyleSheet();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPushButton::paintEvent(event);

        if(m_hasDropdown)
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            //Draw small dropdown arrow
            int arrowSize = 4;
            int rightMargin = 6;
            int centerY = height() / 2;
            int arrowX = width() - rightMargin - arrowSize;

            QPen pen(QColor("#888888"));
            pen.setWidth(1);
            painter.setPen(pen);

            //Draw down arrow
            painter.drawLine(arrowX, centerY - 1, arrowX + arrowSize, centerY - 1);
            painter.drawLine(arrowX, centerY - 1, arrowX + arrowSize/2, centerY + 2);
            painter.drawLine(arrowX + arrowSize, centerY - 1, arrowX + arrowSize/2, centerY + 2);
        }
    }

private:
    void updateStyleSheet()
    {
        QString style =
                "ToolbarButton {"
                "    background: transparent;"
                "    color: #ffffff;"
                "    border: none;"
                "    padding: 4px 8px;"
                "    font-size: 13px;"
                "}"
                "ToolbarButton:hover {"
                "    background: #404142;"
                "    color: #ffffff;"
                "}";

        if(m_hasDropdown)
        {
            style += "ToolbarButton { padding-right: 20px; }";
        }

        setStyleSheet(style);
    }

    bool m_hasDropdown;
};

/* ============================================================
 * MAIN DEV ASSISTANT UI
 * ============================================================ */
class DevAssistant_MainAI : public QWidget
{
    Q_OBJECT

public:
    explicit DevAssistant_MainAI(QWidget *parent = nullptr);
    ~DevAssistant_MainAI();

    //Public method to update current file display
    void setCurrentFile(const QString& filename);

    void finishTextBlockWidget();
    void appendCodeInCodeBlockWidget(const QString& code);
    void finishCodeBlockWidget();
    void appendTextInTextWidget(const QString& text);

public slots:
    void SLOT_UpdatesUserRequestToAi();

private slots:
    void SLOT_onAIResponseStarted();
    void SLOT_ReceivePartialAiOutput(const QString &qsAiResponseChunks);
    void SLOT_onAIStreamingResponseFinished();
    void SLOT_AiThrowedError(const QString &error);

    void onModelsFetched(const QStringList &models);
    void scrollToBottom();
    void onAgentMenuRequested();
    void onAutoMenuRequested();
    void onSettingsClicked();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void setupUI();
    void setupToolbar();
    void setupInputArea();
    void applyDarkTheme();
    void adjustInputHeight();

    /* ================= STATE ================= */

    bool m_bAiActive {false};
    bool m_bCodeReplacement {false};
    bool m_bInsideCodeBlockFlag {false};

    /* ================= BACKEND ================= */

    DevAssistant_Client *m_obj_devassist_client {nullptr};

    /* ================= CHAT UI ================= */

    QScrollArea *qsa_chatScrollArea {nullptr};
    QWidget     *qW_chatContainer   {nullptr};
    QVBoxLayout *qvbl_chatLayout    {nullptr};

    /* ================= INPUT UI (NEW DESIGN) ================= */

    QWidget     *m_toolbar           {nullptr};
    QWidget     *m_inputContainer    {nullptr};
    QTextEdit   *m_qtxtedt_UserPrompt {nullptr};
    QPushButton *m_qpshbtn_SendToAI   {nullptr};
    QPushButton *m_btnSettings       {nullptr};
    QLabel      *m_lblCurrentFile    {nullptr};

    ToolbarButton *m_btnAgent        {nullptr};
    ToolbarButton *m_btnAuto         {nullptr};

    //Hidden combo boxes (for internal logic)
    QComboBox   *m_qcmbx_AiMode       {nullptr};
    QComboBox   *m_cmbxModelComboBox  {nullptr};

    AITextWidget *m_currentAITextWidget {nullptr};
    AITextWidget *m_AIloadingAnimationWidget {nullptr};

    /* ================= STREAM BUFFER ================= */

    QString m_qs_aiFullResponse;

    AITextWidget*     m_CurrentStreamingTextWidget = nullptr;
    AICodeBlockWidget* m_CurrentStreamingCodeWidget = nullptr;

    QString m_streamBuffer;  //Buffer for handling partial chunks
    QString m_currentCodeLanguage {"Code"};

    bool m_bWaitingForLanguage {false};  // NEW: flag to wait for language extraction

    QString m_qsSelectedCodeFromQtEditor;
    QString m_qsFinalAiGeneratedCode;
};

#endif // DEVASSISTANT_MAINAI_H
