#include "Cube/CenterCube.h"

void CenterCube::Update(_In_ FLOAT deltaTime)
{
	m_world = XMMatrixRotationY(deltaTime);
}