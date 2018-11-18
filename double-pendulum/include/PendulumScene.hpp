#pragma once

#include "Pendulum.hpp"

#include <atlas/tools/ModellingScene.hpp>
#include <atlas/utils/FPSCounter.hpp>

namespace doublependulum
{
    class PendulumScene : public atlas::tools::ModellingScene
    {
    public:
        PendulumScene();

        void updateScene(double time) override;
        void renderScene() override;

    private:
        bool mPlay;
        atlas::utils::FPSCounter mAnimCounter;
        Pendulum mPendulum;
    };
}
