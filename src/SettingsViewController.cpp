#include "SettingsViewController.hpp"
using namespace PauseConfirm;

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
using namespace QuestUI;

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/RectOffset.hpp"
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;

DEFINE_CLASS(SettingsViewController);

// Create 3 methods for when each of our settings changes
void onContinueSettingChange(bool newValue) {
    getConfig().config["continue"] = newValue;
}

void onRestartSettingChange(bool newValue) {
    getConfig().config["restart"] = newValue;
}

void onMenuSettingChange(bool newValue) {
    getConfig().config["menu"] = newValue;
}

void onPauseButtonsOverrideSettingChange(SettingsViewController* self, bool newValue) {
    getConfig().config["overridePauseButtons"] = newValue;
    getLogger().info("Override status change.");
    self->UpdateButtonsLayoutVisibility(); // Hide/show the layout if necessary
}

void onUseRightControllerSettingChange(bool newValue) {
    getConfig().config["pauseWithRightController"] = newValue;
    getLogger().info("Right controller setting change.");
}

void onButtonRequiredToPauseSettingChange(std::string buttonName, bool newValue) {
    getLogger().info("Button with name " + buttonName + " change.");
    getConfig().config["pauseButtons"][buttonName] = newValue;
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!(addedToHierarchy && firstActivation)) {return;} // Only create the UI once

    getLogger().info("Creating settings UI . . .");
    // Align the three settings to the top centre with a VerticalLayoutGroup
    VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(get_rectTransform());
    layout->set_spacing(1.0);
    layout->set_childControlHeight(true);
    layout->set_childForceExpandHeight(false);

    VerticalLayoutGroup* confirmationLayout = BeatSaberUI::CreateVerticalLayoutGroup(layout->get_rectTransform());
    confirmationLayout->set_childControlHeight(true);
    confirmationLayout->set_childForceExpandHeight(false);
    confirmationLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    confirmationLayout->get_gameObject()->AddComponent<Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
    confirmationLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));

    BeatSaberUI::CreateText(confirmationLayout->get_rectTransform(), "Pause menu confirmation");
    // Create 3 toggles for each button in the pause menu, making sure to set the current value in the config to each toggle.
    BeatSaberUI::CreateToggle(confirmationLayout->get_rectTransform(), "Enable confirmation on continue button", getConfig().config["continue"].GetBool(),
             onContinueSettingChange);
    BeatSaberUI::CreateToggle(confirmationLayout->get_rectTransform(), "Enable confirmation on restart button", getConfig().config["restart"].GetBool(),
             onRestartSettingChange);
    BeatSaberUI::CreateToggle(confirmationLayout->get_rectTransform(), "Enable confirmation on menu button", getConfig().config["menu"].GetBool(),
             onMenuSettingChange);

    VerticalLayoutGroup* buttonSectionLayout = BeatSaberUI::CreateVerticalLayoutGroup(layout->get_rectTransform());
    buttonSectionLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    buttonSectionLayout->set_childControlHeight(true);
    buttonSectionLayout->set_childForceExpandHeight(false);
    buttonSectionLayout->get_gameObject()->AddComponent<Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
    buttonSectionLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));

    getLogger().info("Creating buttons section layouts . . .");
    // Create a toggle for enabling/disabling the pause buttons override
    BeatSaberUI::CreateToggle(buttonSectionLayout->get_rectTransform(), "Override Pause Buttons", getConfig().config["overridePauseButtons"].GetBool(),
        [this](bool newValue) {
            onPauseButtonsOverrideSettingChange(this, newValue);
        }
    );

    // Create toggle for choosing to pause with your right controller
    BeatSaberUI::CreateToggle(buttonSectionLayout->get_rectTransform(), "Pause with Right Controller",
            getConfig().config["pauseWithRightController"].GetBool(),
            onUseRightControllerSettingChange
    );

    VerticalLayoutGroup* buttonsLayout = BeatSaberUI::CreateVerticalLayoutGroup(layout->get_rectTransform());
    buttonsLayout->get_gameObject()->AddComponent<Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
    buttonsLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    buttonsLayout->set_childControlHeight(true);
    buttonsLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
    buttonsLayout->set_childForceExpandHeight(false);
    // Create toggle for each button that can be used to pause the game.
    for(auto& pair : getButtonNames()) {
        getLogger().info("Adding button to list . . .");
        // Make the PauseOverrideClickData to send the correct info to the delegate
        std::string buttonName = pair.second;

        BeatSaberUI::CreateToggle(buttonsLayout->get_rectTransform(), buttonName,
                getConfig().config["pauseButtons"][buttonName].GetBool(),
                [buttonName](bool newValue) {onButtonRequiredToPauseSettingChange(buttonName, newValue);}
        );
    }

    this->buttonsObject = buttonsLayout->get_gameObject();
    UpdateButtonsLayoutVisibility(); // Hide the buttons layout if overridding pause buttons is not enabled
}

void SettingsViewController::UpdateButtonsLayoutVisibility() {
    bool isVisible = getConfig().config["overridePauseButtons"].GetBool();

    buttonsObject->SetActive(isVisible);
}

// Save the config upon leaving the settings menu
void SettingsViewController::DidDeactivate(bool removedFromHierarchy, bool systemScreenDisabling) {
    getLogger().info("Saving config on settings menu exit.");
    getConfig().Write();
}