#pragma once

namespace bowtie
{
namespace uniform
{

enum Type
{
    Float, Vec2, Vec3, Vec4, Mat3, Mat4, Texture1, Texture2, Texture3, NumUniformTypes
};

enum AutomaticValue
{
    None, ModelViewProjectionMatrix, ModelViewMatrix, ModelMatrix, Time, ViewResolution, ViewResolutionRatio, Resolution, NumAutomaticValues
};

}
}