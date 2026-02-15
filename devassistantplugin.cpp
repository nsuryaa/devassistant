#include "devassistantplugin.h"
#include "devassistantconstants.h"

namespace DevAssistant {
namespace Internal {

DevAssistantPlugin::DevAssistantPlugin()
{
    //Create your members
}

DevAssistantPlugin::~DevAssistantPlugin()
{
    //Unregister objects from the plugin manager's object pool
    //Delete members
}

class DevAssistantFactory : public Core::INavigationWidgetFactory
{
public:
    DevAssistantFactory() {
        setDisplayName(tr(ASSISTANT_DISPLAY_NAME));
        setId(Constants::DEVASSISTANT_ID);
        setPriority(100);
    }

    Core::NavigationView createWidget() override
    {
        Core::NavigationView view;
        view.widget = new DevAssistant_MainAI;
        return view;
    }
};

DevAssistantFactory *devAssistant = nullptr;
DevAssistant_ModeAI *aiMode = nullptr;

bool DevAssistantPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    //Register objects in the plugin manager's object pool
    //Load settings
    //Add actions to menus
    //Connect to other plugins' signals
    //In the initialize function, a plugin can be sure that the plugins it
    //depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Core::ActionContainer *actionContainer = nullptr;
    Core::Command *sendCmd = nullptr;

    QAction *aiAction = nullptr;

    devAssistant = new DevAssistantFactory;
    aiMode = new DevAssistant_ModeAI;

    //Register our new sidebar factory to Core IDE
    ExtensionSystem::PluginManager::addObject(devAssistant);

    //Register our new mode to Core IDE
    ExtensionSystem::PluginManager::addObject(aiMode);

    //1. Create the Action
    aiAction = new QAction(tr("AI Assist"), this);

    //2. Register the Action with a Unique ID in CPP Editor
    //Use CPPEDITOR_ID to register action in CPP Editor
    sendCmd = Core::ActionManager::registerAction(aiAction,
                                                  "DevAssistant.SendToAi",
                                                  Core::Context(CppEditor::Constants::CPPEDITOR_ID));

    //3. Add it to the Cpp Editor's Context Menu
    actionContainer = Core::ActionManager::actionContainer(CppEditor::Constants::M_CONTEXT);
    if(actionContainer) {
        actionContainer->addAction(sendCmd);
    }

    //4. THE MISSING LINK: Logic to open the sidebar
    connect(aiAction, &QAction::triggered, this, []() {
        //This ensures the sidebar expands and shows your widget ID

        //1. Get the IDE command for your sidebar
        QString commandId = QString("QtCreator.Sidebar.%1").arg(Constants::DEVASSISTANT_ID);
        Core::Command *cmd = Core::ActionManager::command(commandId.toUtf8().data());

        if(cmd && cmd->action()) {
            //2. Triggering this standard IDE action handles the "Focus" logic
            //If it's already open, the IDE just focuses it.
            cmd->action()->trigger();
        }
    });

    return true;
}

void DevAssistantPlugin::extensionsInitialized()
{
    //Retrieve objects from the plugin manager's object pool
    //In the extensionsInitialized function, a plugin can be sure that all
    //plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag DevAssistantPlugin::aboutToShutdown()
{
    //Save settings
    //Disconnect from signals that are not needed during shutdown
    //Hide UI (if you add UI that is not in the main window directly)

    if(devAssistant) {
        ExtensionSystem::PluginManager::removeObject(devAssistant);
        delete devAssistant;
        devAssistant = nullptr;
    }

    if(aiMode) {
        ExtensionSystem::PluginManager::removeObject(aiMode);
        delete aiMode;
        aiMode = nullptr;
    }

    return SynchronousShutdown;
}

} //namespace Internal
} //namespace DevAssistant
