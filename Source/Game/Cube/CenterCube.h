#pragma once

#include "Cube/BaseCube.h"

class CenterCube : public BaseCube
{
public:
    CenterCube() = default;
    ~CenterCube() = default;

    void Update(_In_ FLOAT deltaTime) override;
};