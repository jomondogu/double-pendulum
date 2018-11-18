#include "PendulumScene.hpp"

#include <atlas/utils/GUI.hpp>
#include <atlas/gl/GL.hpp>

namespace doublependulum
{
    PendulumScene::PendulumScene() :
        mPlay(false),
        mAnimCounter(60.0f),
        mPendulum("sun.jpg")
    {
        mPendulum.setPosition({0.0f, 0.0f, 0.0f},{ 0.0f, 5.0f, 0.0f },{ 0.0f, 10.0f, 0.0f });
    }

    void PendulumScene::updateScene(double time)
    {
        using atlas::core::Time;

        ModellingScene::updateScene(time);
        if (mPlay && mAnimCounter.isFPS(mTime))
        {
            mPendulum.updateGeometry(mTime);
        }
    }

    void PendulumScene::renderScene()
    {
        using atlas::utils::Gui;

        Gui::getInstance().newFrame();
        const float grey = 92.0f / 255.0f;
        glClearColor(grey, grey, grey, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mProjection = glm::perspective(
            glm::radians(mCamera.getCameraFOV()),
            (float)mWidth / mHeight, 1.0f, 100000000.0f);
        mView = mCamera.getCameraMatrix();

        mGrid.renderGeometry(mProjection, mView);
        mPendulum.renderGeometry(mProjection, mView);

        // Global HUD
        ImGui::SetNextWindowSize(ImVec2(350, 140), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Global HUD");
        if (ImGui::Button("Reset Camera"))
        {
            mCamera.resetCamera();
        }

        if (mPlay)
        {
            if (ImGui::Button("Pause"))
            {
                mPlay = !mPlay;
            }
        }
        else
        {
            if (ImGui::Button("Play"))
            {
                mPlay = !mPlay;
            }
        }

        if (ImGui::Button("Reset"))
        {
            mPendulum.setPosition({0.0f, 0.0f, 0.0f},{ 0.0f, 5.0f, 0.0f },{ 0.0f, 10.0f, 0.0f });
            mPendulum.resetGeometry();
            mPlay = false;
            mTime.currentTime = 0.0f;
            mTime.totalTime = 0.0f;
        }

        ImGui::Text("Application average %.3f ms/frame (%.1FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        mPendulum.drawGui();

        ImGui::Render();
    }
}
