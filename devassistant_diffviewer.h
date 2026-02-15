#ifndef DEVASSISTANT_DIFFVIEWER_H
#define DEVASSISTANT_DIFFVIEWER_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QVector>

class DevAssistant_DiffViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DevAssistant_DiffViewer(
            const QString &qsOriginal,
            const QString &qsModified,
            QWidget *parent = nullptr);

signals:
    void SGNL_replaceConfirmed(const QString &qsNewCode);

private slots:
    void SLOT_onReplaceClicked();

private:

    enum E_DIFF_OPERATION
    {
        DIFF_EQUAL,
        DIFF_ADDITION,
        DIFF_DELETION,
        DIFF_UPDATE
    };

    struct S_DiffLine
    {
        QString qsLeftLine;
        QString qsRightLine;
        E_DIFF_OPERATION eOperation;
    };

    void buildDiff(const QString &original,
                   const QString &modified);

    QVector<S_DiffLine> computeDiff(
            const QStringList &leftLines,
            const QStringList &rightLines);

    void highlightLine(QPlainTextEdit *editor,
                       int lineNumber,
                       const QColor &color);

    QPlainTextEdit *m_qptOriginal;
    QPlainTextEdit *m_qptModified;
    QPushButton    *m_qpbReplace;

    QString m_qsModifiedCode;
};

#endif // DEVASSISTANT_DIFFVIEWER_H
