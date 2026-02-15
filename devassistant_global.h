#ifndef DEVASSISTANT_GLOBAL_H
#define DEVASSISTANT_GLOBAL_H

#include <QtGlobal>

#include <coreplugin/icore.h>
#include <coreplugin/imode.h>
#include <coreplugin/icontext.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/sidebar.h>
#include <coreplugin/inavigationwidgetfactory.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>

#include <coreplugin/modemanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <cpptools/cpptoolsconstants.h>
#include <cppeditor/cppeditorconstants.h>

#include "devassistantconstants.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <QString>
#include <QScrollBar>
#include <QComboBox>
#include <QObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QWidget>
#include <QListWidget>
#include <QTextDocument>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QMap>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QStyle>
#include <QTextBrowser>
#include <QToolButton>
#include <QMovie>
#include <QPainter>
#include <QKeyEvent>

#if defined(DEVASSISTANT_LIBRARY)
#  define DEVASSISTANTSHARED_EXPORT Q_DECL_EXPORT /*Building the DLL / SO*/
#else
#  define DEVASSISTANTSHARED_EXPORT Q_DECL_IMPORT
#endif

#define ASSISTANT_DISPLAY_NAME "AI Assistant"

#define ASSISTANT_INPUT_OBJECT_NAME "aiChatInput"

#endif // DEVASSISTANT_GLOBAL_H
