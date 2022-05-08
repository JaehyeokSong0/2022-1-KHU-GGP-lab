#pragma once

#include "Common.h"
#include "Cube/BaseCube.h"

class CustomCube : public BaseCube
{
public:
	CustomCube(const XMFLOAT4& outputColor);
	CustomCube(const CustomCube& other) = delete;
	CustomCube(CustomCube&& other) = delete;
	CustomCube& operator=(const CustomCube& other) = delete;
	CustomCube& operator=(CustomCube&& other) = delete;
	~CustomCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};