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
    Core::ActionContainer *cppContextMenu = nullptr;
    Core::ActionContainer *aiMenu = nullptr;

    Core::Command *sendCmd = nullptr;
    Core::Command *aiCmd = nullptr;

    aiMenu = Core::ActionManager::createMenu("DevAssistant.AIMenu");
    aiMenu->menu()->setTitle(tr("AI Assist"));

    QAction *explainAction = new QAction(tr("Explain"), this);
    QAction *fixAction = new QAction(tr("Fix"), this);
    QAction *reviewAction = new QAction(tr("Review and Comment"), this);
    QAction *genDocsAction = new QAction(tr("Generate Docs"), this);
    QAction *genDoxygenAction = new QAction(tr("Generate Doxygen Comment"), this);
    QAction *createDeclDefAction = new QAction(tr("Create Declaration/Definition"), this);
    QAction *refactorAction = new QAction(tr("Refactor"), this);

    Core::Command *explainCmd =
        Core::ActionManager::registerAction(
            explainAction,
            "DevAssistant.Explain",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *fixCmd =
        Core::ActionManager::registerAction(
            fixAction,
            "DevAssistant.Fix",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *reviewCmd =
        Core::ActionManager::registerAction(
            reviewAction,
            "DevAssistant.Review",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *genDocsCmd =
        Core::ActionManager::registerAction(
            genDocsAction,
            "DevAssistant.GenDocs",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *genDoxyCmd =
        Core::ActionManager::registerAction(
            genDoxygenAction,
            "DevAssistant.GenDoxy",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *createDeclDefCmd =
        Core::ActionManager::registerAction(
            createDeclDefAction,
            "DevAssistant.CreateDeclDef",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    Core::Command *refactorCmd =
        Core::ActionManager::registerAction(
            refactorAction,
            "DevAssistant.Refactor",
            Core::Context(CppEditor::Constants::CPPEDITOR_ID)
        );

    aiMenu->addAction(explainCmd);
    aiMenu->addAction(fixCmd);
    aiMenu->addAction(reviewCmd);
    aiMenu->addAction(genDocsCmd);
    aiMenu->addAction(genDoxyCmd);
    aiMenu->addAction(createDeclDefCmd);
    aiMenu->addAction(refactorCmd);

    cppContextMenu =
        Core::ActionManager::actionContainer(CppEditor::Constants::M_CONTEXT);
    if(cppContextMenu)
    {
        cppContextMenu->addMenu(aiMenu);
    }

    //aiMenu->addAction(explainCmd);

    devAssistant = new DevAssistantFactory;
    aiMode = new DevAssistant_ModeAI;

    //Register our new sidebar factory to Core IDE
    ExtensionSystem::PluginManager::addObject(devAssistant);

    //Register our new mode to Core IDE
    ExtensionSystem::PluginManager::addObject(aiMode);

//    connect(explainAction, &QAction::triggered, this, [](){
//        DevAssistant_RequestRouter::handle(DevAssistant_RequestType::Explain);
//    });

//    void DevAssistant_RequestRouter::handle(DevAssistant_RequestType type)

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
