#include "devassistant_editorutils.h"

/*Returns the currently selected text from the active editor.*/
QString DevAssistant_EditorUtils::getSelectedTextQt()
{
    QString qsSelectedText = "";
    auto aEditor = Core::EditorManager::currentEditor(); /*Accesses the currently active editor in Qt Creator*/
    auto aTextEditor = qobject_cast<TextEditor::BaseTextEditor *>(aEditor); /*Safely checks whether the editor is a text editor*/

    /*No editor is open*/
    if(!aTextEditor)
    {
        return QString();
    }

    auto acurrentCursor = aTextEditor->editorWidget()->textCursor(); /*Gets the current QTextCursor*/
    qsSelectedText = acurrentCursor.selectedText(); /*Extracts only the selected text*/
    qsSelectedText.replace(QChar(0x2029), '\n');

    return qsSelectedText;
}

/*Replaces the currently selected text in the active editor with new code (AI output).*/
void DevAssistant_EditorUtils::replaceSelectedTextwithNewTextQt(const QString &qs_newCode)
{
    auto aEditor = Core::EditorManager::currentEditor();
    auto aTextEditor = qobject_cast<TextEditor::BaseTextEditor *>(aEditor);

    if(!aTextEditor)
    {
        return;
    }

    auto atextWidget = aTextEditor->editorWidget();

    /*Retrieves the current cursor*/
    QTextCursor currentCursor = atextWidget->textCursor();

    if(!currentCursor.hasSelection())
    {
        return;
    }

    /*Groups all changes into one undo step*/
    currentCursor.beginEditBlock();

    /*Deletes selected code*/
    currentCursor.removeSelectedText();

    /*Inserts AI-generated code in the same place*/
    currentCursor.insertText(qs_newCode);

    /*Ends the undo operation*/
    currentCursor.endEditBlock();

    atextWidget->setTextCursor(currentCursor);
}
