#include "TextGame.h"

#include "Modules/ModuleManager.h"

class FTextGameModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();

        UE_LOG(
            LogTemp,
            Display,
            TEXT("TextGame C++ smoke test: module loaded successfully.")
        );
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FTextGameModule, TextGame, "TextGame");
