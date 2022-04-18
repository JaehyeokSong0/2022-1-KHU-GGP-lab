#pragma once

#include "Cube/Cube.h"

class RotateCube : public BaseCube
{
public:
    RotateCube(const std::filesystem::path& textureFilePath);
    RotateCube(const RotateCube& other) = delete;
    RotateCube(RotateCube&& other) = delete;
    RotateCube& operator=(const RotateCube& other) = delete;
    RotateCube& operator=(RotateCube&& other) = delete;
    ~RotateCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};