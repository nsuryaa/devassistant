#ifndef DEVASSISTANT_EDITORUTILS_H
#define DEVASSISTANT_EDITORUTILS_H

#include "devassistant_global.h"

class DevAssistant_EditorUtils
{
public:
    static QString getSelectedTextQt();
    static void replaceSelectedTextwithNewTextQt(const QString &qs_newCode);
};

#endif // DEVASSISTANT_EDITORUTILS_H
