#ifndef DEVASSISTANT_H
#define DEVASSISTANT_H

#include "devassistant_global.h"

#include "devassistant_mainai.h"
#include "devassistant_modeai.h"

namespace DevAssistant {
namespace Internal {

class DevAssistantPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(
            IID "org.qt-project.Qt.QtCreatorPlugin"
            FILE "DevAssistant.json"
            )

public:
    DevAssistantPlugin();
    ~DevAssistantPlugin() override;

    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;
};

} // namespace Internal
} // namespace DevAssistant

/*
 *
 * Lifecycle of qt-plugin
Plugin loaded
   ↓
Constructor
   ↓
initialize()
   ↓
extensionsInitialized()
   ↓
Plugin running
   ↓
aboutToShutdown()
   ↓
Destructor
*/

#endif // DEVASSISTANT_H
