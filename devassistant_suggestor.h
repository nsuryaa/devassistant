#ifndef DEVASSISTANT_SUGGESTOR_H
#define DEVASSISTANT_SUGGESTOR_H

#include "devassistant_global.h"

namespace DevAssistant {
namespace Internal {

class DevAssistantSuggestor : public QWidget
{
    Q_OBJECT

public:
    explicit DevAssistantSuggestor(QWidget *parent = nullptr);
    ~DevAssistantSuggestor();

    void addSuggestion(const QString &qsSuggestion);
    void setSuggestions(const QStringList &qslSuggestions);
    void clearSuggestions();

    QString currentModel() const;
    void setAvailableModels(const QStringList &qslModels);

    void setStatus(const QString &qsStatus, bool bIsError = false);
    void showLoading(bool bLoading);

signals:
    void SGNL_suggestionApplied(const QString &qsSuggestion);
    void SGNL_refreshRequested();
    void SGNL_modelChanged(const QString &qsModel);

private slots:
    void onSuggestionSelected(QListWidgetItem *qlwdgtItem);
    void onApplySuggestion();
    void SLOT_modelChanged(int index);

private:
    QListWidget *m_qlstwdgtSuggestionsList;
    QTextEdit *m_qtxtedtPreviewText;
    QPushButton *m_qpshbtnApplyButton;
    QPushButton *m_qpshbtnRefreshButton;
    QString m_qsCurrentSuggestion;
    QComboBox *m_cmbxModelComboBox;
    QLabel *m_lblStatusLabel;
};

} // namespace Internal
} // namespace DevAssistant

#endif // DEVASSISTANT_SUGGESTOR_H
