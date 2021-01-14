#define RAPIDJSON_HAS_STDSTRING 1 // Enable rapidjson's support for std::string

#include "main.hpp"
#include "SettingsViewController.hpp"
using namespace PauseConfirm;

#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/PauseAnimationController.hpp"
using namespace GlobalNamespace;

#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "GlobalNamespace/OVRPlayerController.hpp"
using namespace UnityEngine::XR;

#include "TMPro/TextMeshProUGUI.hpp"
using namespace TMPro;

#include "questui/shared/QuestUI.hpp"
#include "custom-types/shared/register.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

static std::unordered_map<int, std::string> buttonNames = {
    {OVRInput::Button::PrimaryIndexTrigger, "Trigger"},
    {OVRInput::Button::PrimaryThumbstick, "Thumbstick Click"},
    {OVRInput::Button::Start, "Menu Button"},
    {OVRInput::Button::Four, "Y Button"},
    {OVRInput::Button::Three, "X Button"},
};

std::unordered_map<int, std::string> getButtonNames() {
    return buttonNames;
}

// Creates a default config file if one does not already exist
void createDefaultConfig() {
    ConfigDocument& config = getConfig().config;
    if(!config.HasMember("continue")) {
        config.AddMember("continue", true, config.GetAllocator());
        config.AddMember("restart", true, config.GetAllocator());
        config.AddMember("menu", true, config.GetAllocator());
    }

    // Second HasMember block since these options are new
    if(!config.HasMember("overridePauseButtons")) {
        config.AddMember("overridePauseButtons", false, config.GetAllocator());
    }

    rapidjson::Value pauseButtonRequirements;
    if(config.HasMember("pauseButtons")) {
        pauseButtonRequirements = config["pauseButtons"];
    }   else    {
        pauseButtonRequirements.SetObject();
    }

    // Add an option for each button name that we support
    for(auto& button : buttonNames) {
        rapidjson::Value buttonName;
        buttonName.SetString(button.second, config.GetAllocator());

        // Add the button if it doesn't exist
        if(!pauseButtonRequirements.HasMember(buttonName)) {
            pauseButtonRequirements.AddMember(buttonName, false, config.GetAllocator());
        }
    }
    config.RemoveMember("pauseButtons");
    config.AddMember("pauseButtons", pauseButtonRequirements, config.GetAllocator());

    getConfig().Write(); // Save the updated config.

    if(!config.HasMember("pauseWithRightController")) {
        config.AddMember("pauseWithRightController", false, config.GetAllocator());
    }
}

std::unordered_map<UnityEngine::UI::Button*, std::string> previousButtonText;
bool checkConfirmation(UnityEngine::UI::Button* button) {
    getLogger().info("Checking button confirmation status . . .");
    if(previousButtonText.find(button) == previousButtonText.end()) {
        getLogger().info("Setting text to confirm message - button was not confirmed!");
        TextMeshProUGUI* textMesh = button->GetComponentInChildren<TextMeshProUGUI*>(); // Find the text object in the button

        // Store the old text so it can be restored if the pause menu is re-opened
        Il2CppString* oldText = textMesh->get_text();
        previousButtonText[button] = to_utf8(csstrtostr(oldText));

        // Make the user confirm
        textMesh->set_text(il2cpp_utils::createcsstr("Are you sure?"));
        return false;
    }   else    {
        getLogger().info("Confirm message already displayed, performing button action.");
        return true;
    }
}

// Changes all the buttons back to their default text if it was changed
void restoreButtonText() {
    getLogger().info("Restoring changed button text . . .");
    for(auto& entry : previousButtonText) {
        // Change the text back to what it was
        TextMeshProUGUI* textMesh = entry.first->GetComponentInChildren<TextMeshProUGUI*>();
        textMesh->set_text(il2cpp_utils::createcsstr(entry.second));
    }
    previousButtonText.clear(); // Empty the map - no buttons are overridden
}

// Make all three buttons in the pause menu require confirmation (if enabled)

MAKE_HOOK_OFFSETLESS(PauseMenuManager_ContinueButtonPressed, void, PauseMenuManager* self) {
    // Perform the confirmation if we need to
    bool isConfirmationEnabled = getConfig().config["continue"].GetBool();
    if(isConfirmationEnabled && !checkConfirmation(self->continueButton)) {return;} // Return if unconfirmed
    
    PauseMenuManager_ContinueButtonPressed(self);
}

MAKE_HOOK_OFFSETLESS(PauseMenuManager_RestartButtonPressed, void, PauseMenuManager* self) {
    // Perform the confirmation if we need to
    bool isConfirmationEnabled = getConfig().config["restart"].GetBool();
    if(isConfirmationEnabled && !checkConfirmation(self->restartButton)) {return;} // Return if unconfirmed
    
    previousButtonText.clear(); // If we restart the song, we don't need to worry about changed button text
    PauseMenuManager_RestartButtonPressed(self);
}

MAKE_HOOK_OFFSETLESS(PauseMenuManager_MenuButtonPressed, void, PauseMenuManager* self) {
    // Perform the confirmation if we need to
    bool isConfirmationEnabled = getConfig().config["menu"].GetBool();
    if(isConfirmationEnabled && !checkConfirmation(self->backButton)) {return;} // Return if unconfirmed
    
    previousButtonText.clear(); // If we go back to the menu, we don't need to worry about changed button text
    PauseMenuManager_MenuButtonPressed(self);
}

static bool hasInducedPause = false;
MAKE_HOOK_OFFSETLESS(GameSongController_LateUpdate, void, OVRPlayerController* self) {
    GameSongController_LateUpdate(self);
    if(!getConfig().config["overridePauseButtons"].GetBool()) {return;} // If overriding the pause buttons is disabled return
    getLogger().info("Checking that all required buttons are down . . .");
    // Check that all buttons required to pause are down
    bool allDown = true;
    for(auto pair : buttonNames) {
        if(!getConfig().config["pauseButtons"][pair.second].GetBool()) {continue;} // Check that this button is required

        // Make sure to use the correct controller
        bool useRightController = getConfig().config["pauseWithRightController"].GetBool();
        bool isDown = OVRInput::Get((OVRInput::Button)pair.first, useRightController ? OVRInput::Controller::RTouch : OVRInput::Controller::LTouch);

        if(!isDown) {
            allDown = false;
            break;
        }
    }

    // Pause the game if all buttons required to pause are down
    if(allDown) {
        getLogger().info("Pausing game as they were all down . . .");
        Array<PauseController*>* controllers = UnityEngine::Resources::FindObjectsOfTypeAll<PauseController*>();
        if(controllers->Length() == 0) {return;} // If we're in the menu, there isn't a PauseController
        hasInducedPause = true;
        reinterpret_cast<PauseController*>(controllers->GetValue(0))->Pause();
    }
}

MAKE_HOOK_OFFSETLESS(PauseController_Pause, void, PauseController* self) {
    // If overriding the pause buttons is disabled, don't do anything
    if(!getConfig().config["overridePauseButtons"].GetBool()) {
        PauseController_Pause(self);
        return;
    }

    // Only allow the pause if the method above has induced it, since otherwise pausing with the menu button would still work
    if(hasInducedPause) {
        hasInducedPause = false;
        PauseController_Pause(self);
    }
}

// Change the confirmation text back to default for the next time the pause menu is opened
// This is done here, instead of in PauseMenuManager_ContinueButtonPressed, since otherwise you see the default text for a split second
MAKE_HOOK_OFFSETLESS(PauseAnimationController_ResumeFromPauseAnimationDidFinish, void, PauseAnimationController* self) {
    restoreButtonText();
    PauseAnimationController_ResumeFromPauseAnimationDidFinish(self);
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    createDefaultConfig();

    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    QuestUI::Init(); // Register QuestUI types/other stuff
    custom_types::Register::RegisterType<SettingsViewController>(); // Register the custom ViewController for our mod settings menu
    custom_types::Register::RegisterType<PauseOverrideClickData>();

    QuestUI::Register::RegisterModSettingsViewController<SettingsViewController*>(modInfo); // Make QuestUI show it as an option in mod settings

    getLogger().info("Installing hooks...");
    // Install our hooks
    INSTALL_HOOK_OFFSETLESS(getLogger(), PauseMenuManager_ContinueButtonPressed,
                il2cpp_utils::FindMethodUnsafe("", "PauseMenuManager", "ContinueButtonPressed", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PauseMenuManager_RestartButtonPressed,
                il2cpp_utils::FindMethodUnsafe("", "PauseMenuManager", "RestartButtonPressed", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PauseMenuManager_MenuButtonPressed, 
                il2cpp_utils::FindMethodUnsafe("", "PauseMenuManager", "MenuButtonPressed", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PauseAnimationController_ResumeFromPauseAnimationDidFinish,
                il2cpp_utils::FindMethodUnsafe("", "PauseAnimationController", "ResumeFromPauseAnimationDidFinish", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), GameSongController_LateUpdate, 
                il2cpp_utils::FindMethodUnsafe("", "GameSongController", "LateUpdate", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PauseController_Pause,
                il2cpp_utils::FindMethodUnsafe("", "PauseController", "Pause", 0));
    getLogger().info("Installed all hooks!");
}