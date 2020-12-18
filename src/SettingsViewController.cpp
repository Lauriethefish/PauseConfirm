#include "SettingsViewController.hpp"
using namespace PauseConfirm;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI;

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;

DEFINE_CLASS(SettingsViewController);

// Create 3 methods for when each of our settings changes
void onContinueSettingChange(SettingsViewController* self, bool newValue) {
    getConfig().config["continue"] = newValue;
}

void onRestartSettingChange(SettingsViewController* self, bool newValue) {
    getConfig().config["restart"] = newValue;
}

void onMenuSettingChange(SettingsViewController* self, bool newValue) {
    getConfig().config["menu"] = newValue;
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!(addedToHierarchy && firstActivation)) {return;} // Only create the UI once

    getLogger().info("Creating settings UI . . .");
    // Align the three settings to the top centre with a VerticalLayoutGroup
    VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(get_rectTransform());
    layout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    layout->set_childControlHeight(true);
    layout->set_childForceExpandHeight(false);
    
    // Create 3 toggles for each button in the pause menu, making sure to set the current value in the config to each toggle.
    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Enable confirmation on continue button", getConfig().config["continue"].GetBool(),
             il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onContinueSettingChange));
    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Enable confirmation on restart button", getConfig().config["restart"].GetBool(),
             il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onRestartSettingChange));
    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Enable confirmation on menu button", getConfig().config["menu"].GetBool(),
             il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onMenuSettingChange));
}

// Save the config upon leaving the settings menu
void SettingsViewController::DidDeactivate(bool removedFromHierarchy, bool systemScreenDisabling) {
    getLogger().info("Saving config on settings menu exit.");
    getConfig().Write();
}