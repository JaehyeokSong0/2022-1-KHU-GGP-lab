#include "Cube/MyCube.h"

void MyCube::Update(_In_ FLOAT deltaTime)
{
	XMMATRIX mSpin = XMMatrixRotationZ(-deltaTime);
	XMMATRIX mOrbit = XMMatrixRotationY(-deltaTime * 0.5f);
	XMMATRIX mTranslate = XMMatrixTranslation(6.0f, 1.0f, 2.0f);
	XMMATRIX mScale = XMMatrixScaling(0.6f, 0.6f, 0.6f);

	m_world = mScale * mSpin * mTranslate * mOrbit;
}