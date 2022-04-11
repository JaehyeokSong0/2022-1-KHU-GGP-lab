#pragma once

#include "Cube/BaseCube.h"

class RotateCube : public BaseCube
{
public:
    RotateCube() = default;
    ~RotateCube() = default;

    void Update(_In_ FLOAT deltaTime) override;
};