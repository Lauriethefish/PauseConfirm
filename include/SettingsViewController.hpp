#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"

DECLARE_CLASS_CODEGEN(PauseConfirm, SettingsViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, DidDeactivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidDeactivate", 2), bool removedFromHierarchy, bool systemScreenDisabling);
    DECLARE_METHOD(void, UpdateButtonsLayoutVisibility);
    
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, buttonsObject);

    REGISTER_FUNCTION(SettingsViewController,
        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(DidDeactivate);
        REGISTER_METHOD(UpdateButtonsLayoutVisibility);

        REGISTER_FIELD(buttonsObject);
    )
);