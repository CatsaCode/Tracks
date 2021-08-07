#include "TimeSourceHelper.h"
#include "GlobalNamespace/AudioTimeSyncController.hpp"

using namespace GlobalNamespace;

float TimeSourceHelper::getSongTime(GlobalNamespace::IAudioTimeSource *timeSource) {
    static auto *timeSyncControllerClass = classof(AudioTimeSyncController *);
    auto *timeSourceObject = reinterpret_cast<Il2CppObject *>(timeSource);
    if (timeSourceObject->klass == timeSyncControllerClass) {
        auto *timeSyncController = reinterpret_cast<AudioTimeSyncController *>(timeSource);
        return timeSyncController->songTime;
    } else {
        return timeSource->get_songTime();
    }
}