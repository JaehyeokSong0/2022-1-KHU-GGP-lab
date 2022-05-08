#include "Cube/CustomCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Cube

  Summary:  Constructor

  Args:     const XMFLOAT4& outputColor
			  Default color of the cube
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
CustomCube::CustomCube(const XMFLOAT4& outputColor) :
	BaseCube(outputColor)
{}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Update

  Summary:  Updates the cube every frame

  Args:     FLOAT deltaTime
			  Elapsed time

  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void CustomCube::Update(_In_ FLOAT deltaTime)
{
	static FLOAT s_totalTime = 0.0f;
	s_totalTime += deltaTime;

	m_world = XMMatrixScaling(0.3f, 0.3f, 0.3f)
		* XMMatrixRotationY(s_totalTime * 8.0f)
		* XMMatrixTranslation(4.0f, std::abs(XMScalarSin(s_totalTime * 6.0f)) * 1.5f - 1.0f, 0.0f)
		* XMMatrixRotationY(-s_totalTime);
}