#include <modules/config/config.hpp>
#include <modules/gui/gui.hpp>
#include <modules/gui/components/float-toggle.hpp>
#include <modules/hack/hack.hpp>

#include <Geode/binding/FMODAudioEngine.hpp>

namespace eclipse::hacks::Global {

    void setPitch(float pitch) {
        static FMOD::DSP* pitchShifter = nullptr;

        // si toggle está OFF → fuerza normal
        pitch = config::get<bool>("global.pitchshift.toggle", false) ? pitch : 1.f;

        FMOD::System* system = utils::get<FMODAudioEngine>()->m_system;

        // limpiar DSP anterior
        if (pitchShifter) {
            utils::get<FMODAudioEngine>()->m_backgroundMusicChannel->removeDSP(pitchShifter);
            pitchShifter->release();
            pitchShifter = nullptr;
        }

        // normal = no aplicar efecto
        if (pitch == 1.f)
            return;

        // crear DSP nuevo
        system->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchShifter);

        // calidad del pitch (puedes ajustar 2048 / 4096)
        pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 2048);

        // pitch real
        pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, pitch);

        utils::get<FMODAudioEngine>()->m_backgroundMusicChannel->addDSP(0, pitchShifter);
    }

    class $hack(PitchShift) {
        void init() override {
            auto tab = gui::MenuTab::find("tab.global");

            config::setIfEmpty("global.pitchshift.toggle", false);
            config::setIfEmpty("global.pitchshift", 1.f);

            tab->addFloatToggle(
                    "global.pitchshift",
                    0.05f,   // mínimo (permite 0.4 sin problema)
                    6.f,     // máximo (permite >2 sin problema)
                    "%.2f"
                )
                ->valueCallback(setPitch)
                ->handleKeybinds()
                ->setDescription()
                ->toggleCallback([] {
                    if (config::get<bool>("global.pitchshift.toggle", false))
                        setPitch(config::get<double>("global.pitchshift", 1.f));
                    else
                        setPitch(1.f);
                });
        }

        void lateInit() override {
            if (config::get<bool>("global.pitchshift.toggle", false))
                setPitch(config::get<double>("global.pitchshift", 1.f));
        }

        [[nodiscard]] const char* getId() const override {
            return "Pitch Shift";
        }

        [[nodiscard]] int32_t getPriority() const override {
            return -9;
        }
    };

    REGISTER_HACK(PitchShift)
}
