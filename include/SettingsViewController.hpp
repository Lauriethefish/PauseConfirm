#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Toggle.hpp"

DECLARE_CLASS_CODEGEN(PauseConfirm, SettingsViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, DidDeactivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidDeactivate", 2), bool removedFromHierarchy, bool systemScreenDisabling);
    DECLARE_METHOD(void, UpdateButtonsLayout);
    
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, buttonsObject);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, firstButtonSetting);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, secondButtonSetting);

    REGISTER_FUNCTION(SettingsViewController,
        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(DidDeactivate);
        REGISTER_METHOD(UpdateButtonsLayout);

        REGISTER_FIELD(buttonsObject);
        REGISTER_FIELD(firstButtonSetting);
        REGISTER_FIELD(secondButtonSetting);
    )
);