#pragma once

#include <atlas/utils/Geometry.hpp>
#include <atlas/gl/Buffer.hpp>
#include <atlas/gl/VertexArrayObject.hpp>
#include <atlas/gl/Texture.hpp>

namespace doublependulum
{
    class Pendulum : public atlas::utils::Geometry
    {
    public:
        Pendulum(std::string const& textureFile);

        void setPosition(atlas::math::Point const& pivot, atlas::math::Point const& pos1, atlas::math::Point const& pos2);

        void updateGeometry(atlas::core::Time<> const& t) override;
        void renderGeometry(atlas::math::Matrix4 const& projection,
            atlas::math::Matrix4 const& view) override;
        void drawGui() override;

        void resetGeometry() override;

    private:
        float mag(atlas::math::Vector v);

        atlas::gl::Buffer mVertexBuffer;
        atlas::gl::Buffer mIndexBuffer;
        atlas::gl::VertexArrayObject mVao;
        atlas::gl::Texture mTexture;

        atlas::math::Point mPivotPosition;

        atlas::math::Point mPosition1;
        atlas::math::Vector mVelocity1;
        float mMass1;
        float mRadius1;
        atlas::math::Point mPosition2;
        atlas::math::Vector mVelocity2;
        float mMass2;
        float mRadius2;

        GLsizei mIndexCount;
        float mG = -9.8f;
    };
}
