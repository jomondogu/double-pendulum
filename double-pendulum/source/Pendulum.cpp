#include "Pendulum.hpp"
#include "Paths.hpp"
#include "LayoutLocations.glsl"

#include <atlas/utils/Mesh.hpp>
#include <atlas/core/STB.hpp>
#include <atlas/utils/GUI.hpp>
#include <atlas/math/Coordinates.hpp>
#include <math.h>

namespace doublependulum
{
    Pendulum::Pendulum(std::string const& textureFile) :
        mVertexBuffer(GL_ARRAY_BUFFER),
        mIndexBuffer(GL_ELEMENT_ARRAY_BUFFER),
        mTexture(GL_TEXTURE_2D)
    {
        using atlas::utils::Mesh;
        namespace gl = atlas::gl;
        namespace math = atlas::math;

        Mesh sphere;
        std::string path{ DataDirectory };
        path = path + "sphere.obj";
        Mesh::fromFile(path, sphere);

        mIndexCount = static_cast<GLsizei>(sphere.indices().size());

        std::vector<float> data;
        for (std::size_t i = 0; i < sphere.vertices().size(); ++i)
        {
            data.push_back(sphere.vertices()[i].x);
            data.push_back(sphere.vertices()[i].y);
            data.push_back(sphere.vertices()[i].z);

            data.push_back(sphere.normals()[i].x);
            data.push_back(sphere.normals()[i].y);
            data.push_back(sphere.normals()[i].z);

            data.push_back(sphere.texCoords()[i].x);
            data.push_back(sphere.texCoords()[i].y);
        }

        mVao.bindVertexArray();
        mVertexBuffer.bindBuffer();
        mVertexBuffer.bufferData(gl::size<float>(data.size()), data.data(),
            GL_STATIC_DRAW);
        mVertexBuffer.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 3, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(0));
        mVertexBuffer.vertexAttribPointer(NORMALS_LAYOUT_LOCATION, 3, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(3));
        mVertexBuffer.vertexAttribPointer(TEXTURES_LAYOUT_LOCATION, 2, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(6));

        mVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);
        mVao.enableVertexAttribArray(NORMALS_LAYOUT_LOCATION);
        mVao.enableVertexAttribArray(TEXTURES_LAYOUT_LOCATION);

        mIndexBuffer.bindBuffer();
        mIndexBuffer.bufferData(gl::size<GLuint>(sphere.indices().size()),
            sphere.indices().data(), GL_STATIC_DRAW);

        mIndexBuffer.unBindBuffer();
        mVertexBuffer.unBindBuffer();
        mVao.unBindVertexArray();

        int width, height, nrChannels;
        std::string imagePath = std::string(DataDirectory) + textureFile;
        unsigned char* imageData = stbi_load(imagePath.c_str(), &width, &height,
            &nrChannels, 0);

        mTexture.bindTexture();
        mTexture.texImage2D(0, GL_RGB, width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, imageData);
        mTexture.texParameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
        mTexture.texParameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
        mTexture.texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        mTexture.texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(imageData);

        std::vector<gl::ShaderUnit> shaders
        {
            {std::string(ShaderDirectory) + "Pendulum.vs.glsl", GL_VERTEX_SHADER},
            {std::string(ShaderDirectory) + "Pendulum.fs.glsl", GL_FRAGMENT_SHADER}
        };

        mShaders.emplace_back(shaders);
        mShaders[0].setShaderIncludeDir(ShaderDirectory);
        mShaders[0].compileShaders();
        mShaders[0].linkShaders();

        auto var = mShaders[0].getUniformVariable("model");
        mUniforms.insert(UniformKey("model", var));
        var = mShaders[0].getUniformVariable("projection");
        mUniforms.insert(UniformKey("projection", var));
        var = mShaders[0].getUniformVariable("view");
        mUniforms.insert(UniformKey("view", var));

        mShaders[0].disableShaders();
        mModel = math::Matrix4(1.0f);

        mVelocity1 = atlas::math::Vector(0.0f,0.0f,0.0f);
        mVelocity2 = atlas::math::Vector(0.0f,0.0f,0.0f);
        mMass1 = 10.0f;
        mMass2 = 10.0f;
    }

    void Pendulum::setPosition(atlas::math::Point const& pivot, atlas::math::Point const& pos1, atlas::math::Point const& pos2)
    {
        using atlas::math::Matrix4;
        using atlas::math::cartesianToPolar;

        mPivotPosition.xy = cartesianToPolar(pivot.xy);
        mPosition1.xy = cartesianToPolar(pos1.xy);
        mPosition2.xy = cartesianToPolar(pos2.xy);
        mRadius1 = mag(pos1-pivot);
        mRadius2 = mag(pos2-pos1);
    }

    void Pendulum::updateGeometry(atlas::core::Time<> const& t)
    {
        float r1 = mRadius1;
        float r2 = mRadius2;
        float theta1 = mPosition1.y;
        float theta2 = mPosition2.y;
        float dtheta1 = mVelocity1.y;
        float dtheta2 = mVelocity2.y;

        float mu = 1 + mMass1/mMass2;
        float d2theta1 = (mG*(sin(theta2)*cos(theta1-theta2)-mu*sin(theta1))-(r2*dtheta2*dtheta2+r1*dtheta1*dtheta1*cos(theta1-theta2))*sin(theta1-theta2))/(r1*(mu-cos(theta1-theta2)*cos(theta1-theta2)));
        float d2theta2 = (mu*mG*(sin(theta1)*cos(theta1-theta2)-sin(theta2))+(mu*r1*dtheta1*dtheta1+r2*dtheta2*dtheta2*cos(theta1-theta2))*sin(theta1-theta2))/(r2*(mu-cos(theta1-theta2)*cos(theta1-theta2)));
        mVelocity1.y += d2theta1*t.deltaTime;
        mVelocity2.y += d2theta2*t.deltaTime;
        mPosition1.y += dtheta1*t.deltaTime;
        mPosition2.y += dtheta2*t.deltaTime;

    }

    void Pendulum::renderGeometry(atlas::math::Matrix4 const& projection,
        atlas::math::Matrix4 const& view)
    {
        namespace math = atlas::math;
        using atlas::math::polarToCartesian;

        mShaders[0].hotReloadShaders();
        if (!mShaders[0].shaderProgramValid())
        {
            return;
        }

        mShaders[0].enableShaders();

        mTexture.bindTexture();
        mVao.bindVertexArray();
        mIndexBuffer.bindBuffer();

        atlas::math::Vector pos1;
        atlas::math::Vector pos2;
        pos1.x = mRadius1*sin(mPosition1.y);
        pos1.y = mRadius1*cos(mPosition1.y);
        pos2.x = mRadius1*sin(mPosition1.y) + mRadius2*sin(mPosition2.y);
        pos2.y = mRadius1*cos(mPosition1.y) + mRadius2*cos(mPosition2.y);

        auto mModel1 = glm::translate(mModel, pos1) * glm::scale(atlas::math::Matrix4(1.0f), atlas::math::Vector(0.25f));
        auto mModel2 = glm::translate(mModel, pos2) * glm::scale(atlas::math::Matrix4(1.0f), atlas::math::Vector(0.25f));

        //render bob 1
        glUniformMatrix4fv(mUniforms["model"], 1, GL_FALSE, &mModel1[0][0]);
        glUniformMatrix4fv(mUniforms["projection"], 1, GL_FALSE,
            &projection[0][0]);
        glUniformMatrix4fv(mUniforms["view"], 1, GL_FALSE, &view[0][0]);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

        //render bob 2
        glUniformMatrix4fv(mUniforms["model"], 1, GL_FALSE, &mModel2[0][0]);
        glUniformMatrix4fv(mUniforms["projection"], 1, GL_FALSE,
            &projection[0][0]);
        glUniformMatrix4fv(mUniforms["view"], 1, GL_FALSE, &view[0][0]);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

        mIndexBuffer.unBindBuffer();
        mVao.unBindVertexArray();
        mTexture.unBindTexture();

        mShaders[0].disableShaders();
    }

    void Pendulum::drawGui(){
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Pendulum Controls");
        ImGui::InputFloat("Mass 1", &mMass1, 10.0f, 5.0f, 1);
        ImGui::InputFloat("Radius 1", &mRadius1, 0.5f, 5.0f, 1);
        ImGui::InputFloat("Mass 2", &mMass2, 10.0f, 5.0f, 1);
        ImGui::InputFloat("Radius 2", &mRadius2, 0.5f, 5.0f, 1);
        ImGui::InputFloat("Gravity", &mG, 0.2f, 5.0f, 1);
        ImGui::End();
    }

    void Pendulum::resetGeometry()
    {
        mVelocity1 = atlas::math::Vector(0.0f,0.0f,0.0f);
        mVelocity2 = atlas::math::Vector(0.0f,0.0f,0.0f);
    }

    float Pendulum::mag(atlas::math::Vector v)
    {
        return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    }
}
