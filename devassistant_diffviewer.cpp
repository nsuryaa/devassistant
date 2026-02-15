#include "devassistant_diffviewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFont>
#include <QColor>

DevAssistant_DiffViewer::DevAssistant_DiffViewer(
        const QString &qsOriginal,
        const QString &qsModified,
        QWidget *parent)
    : QDialog(parent),
      m_qsModifiedCode(qsModified)
{
    setWindowTitle("AI Code Replacement Preview");
    resize(1100, 650);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *diffLayout = new QHBoxLayout;

    m_qptOriginal = new QPlainTextEdit(this);
    m_qptModified = new QPlainTextEdit(this);
    m_qpbReplace  = new QPushButton("Replace Code", this);

    m_qptOriginal->setReadOnly(true);
    m_qptModified->setReadOnly(true);

    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_qptOriginal->setFont(monoFont);
    m_qptModified->setFont(monoFont);

    diffLayout->addWidget(m_qptOriginal);
    diffLayout->addWidget(m_qptModified);

    mainLayout->addLayout(diffLayout);
    mainLayout->addWidget(m_qpbReplace);

    buildDiff(qsOriginal, qsModified);

    connect(m_qpbReplace,
            &QPushButton::clicked,
            this,
            &DevAssistant_DiffViewer::SLOT_onReplaceClicked);
}

void DevAssistant_DiffViewer::buildDiff(
        const QString &original,
        const QString &modified)
{
    QStringList originalLines = original.split('\n');
    QStringList modifiedLines = modified.split('\n');

    QVector<S_DiffLine> diff =
            computeDiff(originalLines, modifiedLines);

    QStringList leftDisplay;
    QStringList rightDisplay;

    for(const S_DiffLine &line : diff)
    {
        leftDisplay.append(line.qsLeftLine);
        rightDisplay.append(line.qsRightLine);
    }

    m_qptOriginal->setPlainText(leftDisplay.join("\n"));
    m_qptModified->setPlainText(rightDisplay.join("\n"));

    for(int i = 0; i < diff.size(); ++i)
    {
        switch(diff[i].eOperation)
        {
        case DIFF_ADDITION:
            highlightLine(m_qptModified, i, QColor(200,255,200));
            break;

        case DIFF_DELETION:
            highlightLine(m_qptOriginal, i, QColor(255,200,200));
            break;

        case DIFF_UPDATE:
            highlightLine(m_qptOriginal, i, QColor(255,240,180));
            highlightLine(m_qptModified, i, QColor(255,240,180));
            break;

        default:
            break;
        }
    }
}

QVector<DevAssistant_DiffViewer::S_DiffLine>
DevAssistant_DiffViewer::computeDiff(
        const QStringList &leftLines,
        const QStringList &rightLines)
{
    int n = leftLines.size();
    int m = rightLines.size();

    QVector<QVector<int>> lcs(n + 1,
                              QVector<int>(m + 1, 0));

    for(int i = n - 1; i >= 0; --i)
    {
        for(int j = m - 1; j >= 0; --j)
        {
            if(leftLines[i] == rightLines[j])
                lcs[i][j] = 1 + lcs[i + 1][j + 1];
            else
                lcs[i][j] =
                        qMax(lcs[i + 1][j],
                             lcs[i][j + 1]);
        }
    }

    QVector<S_DiffLine> result;

    int i = 0, j = 0;

    while(i < n && j < m)
    {
        if(leftLines[i] == rightLines[j])
        {
            result.append(
                        { leftLines[i],
                          rightLines[j],
                          DIFF_EQUAL });
            ++i; ++j;
        }
        else if(lcs[i + 1][j] >= lcs[i][j + 1])
        {
            result.append(
                        { leftLines[i],
                          "",
                          DIFF_DELETION });
            ++i;
        }
        else
        {
            result.append(
                        { "",
                          rightLines[j],
                          DIFF_ADDITION });
            ++j;
        }
    }

    while(i < n)
    {
        result.append(
                    { leftLines[i++],
                      "",
                      DIFF_DELETION });
    }

    while(j < m)
    {
        result.append(
                    { "",
                      rightLines[j++],
                      DIFF_ADDITION });
    }

    /* Detect UPDATE (delete + add pair) */
    for(int k = 0; k < result.size() - 1; ++k)
    {
        if(result[k].eOperation == DIFF_DELETION &&
           result[k+1].eOperation == DIFF_ADDITION)
        {
            result[k].eOperation = DIFF_UPDATE;
            result[k].qsRightLine =
                    result[k+1].qsRightLine;

            result.removeAt(k+1);
        }
    }

    return result;
}

void DevAssistant_DiffViewer::highlightLine(
        QPlainTextEdit *editor,
        int lineNumber,
        const QColor &color)
{
    QTextBlock block =
            editor->document()
            ->findBlockByLineNumber(lineNumber);

    if(!block.isValid())
        return;

    QTextCursor cursor(block);
    cursor.select(QTextCursor::LineUnderCursor);

    QTextCharFormat format;
    format.setBackground(color);

    cursor.setCharFormat(format);
}

void DevAssistant_DiffViewer::SLOT_onReplaceClicked()
{
    emit SGNL_replaceConfirmed(m_qsModifiedCode);
    accept();
}
