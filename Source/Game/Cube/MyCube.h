#pragma once

#include "Cube/BaseCube.h"

class MyCube : public BaseCube
{
public:
    MyCube() = default;
    ~MyCube() = default;

    void Update(_In_ FLOAT deltaTime) override;
};