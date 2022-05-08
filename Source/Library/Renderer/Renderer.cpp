#include "Renderer/Renderer.h"

namespace library {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				 m_immediateContext, m_immediateContext1, m_swapChain,
				 m_swapChain1, m_renderTargetView, m_depthStencil,
				 m_depthStencilView, m_cbChangeOnResize, m_camera,
				 m_projection, m_renderables, m_vertexShaders,
				 m_pixelShaders].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Renderer::Renderer() :
		m_driverType(D3D_DRIVER_TYPE_HARDWARE),
		m_featureLevel(D3D_FEATURE_LEVEL_11_1),
		m_d3dDevice(),
		m_d3dDevice1(),
		m_immediateContext(),
		m_immediateContext1(),
		m_swapChain(),
		m_swapChain1(),
		m_renderTargetView(),
		m_depthStencil(),
		m_depthStencilView(),
		m_cbChangeOnResize(),
		m_cbLights(),
		m_camera(XMVectorSet(0.0f, 8.0f, -12.0f, 0.0f)),
		m_projection(),
		m_renderables(),
		m_aPointLights(),
		m_vertexShaders(),
		m_pixelShaders()
	{}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize

	  Summary:  Creates Direct3D device and swap chain

	  Args:     HWND hWnd
				  Handle to the window

	  Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				 m_d3dDevice1, m_immediateContext1, m_swapChain1,
				 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
				 m_projection, m_cbLights, m_camera, m_vertexShaders,
				 m_pixelShaders, m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
		HRESULT hr;

#pragma region GetInterfaceForDeviceAndContext
		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		const D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = D3D11CreateDevice(
			nullptr,								// Specify nullptr to use the default adapter.
			m_driverType,							// Create a device using the hardware graphics driver.
			nullptr,								// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags,							// Set debug and Direct2D compatibility flags.
			featureLevels,							// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),				// Size of the list above.
			D3D11_SDK_VERSION,						// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&m_d3dDevice,							// Returns the Direct3D device created.
			&m_featureLevel,						// Returns feature level of device created.
			&m_immediateContext						// Returns the device immediate context.
		);
		if (FAILED(hr)) return hr;

		m_d3dDevice.As(&m_d3dDevice1);
		m_immediateContext.As(&m_immediateContext1);
#pragma endregion

#pragma region CreateSwapChain
		DXGI_SWAP_CHAIN_DESC desc = {
			.BufferDesc = {
				.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			},
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = hWnd,
			.Windowed = TRUE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		};

		// Create the DXGI device object to use in other factories, such as Direct2D.
		ComPtr<IDXGIDevice3> pDxgiDevice;
		m_d3dDevice.As(&pDxgiDevice);

		// Create swap chain.
		ComPtr<IDXGIAdapter> pAdapter;
		ComPtr<IDXGIFactory> pFactory;

		hr = pDxgiDevice->GetAdapter(&pAdapter);
		if (FAILED(hr)) return hr;

		hr = pAdapter->GetParent(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) return hr;

		hr = pFactory->CreateSwapChain(
			m_d3dDevice.Get(),
			&desc,
			&m_swapChain
		);
		if (FAILED(hr)) return hr;

		m_swapChain.As(&m_swapChain1);
#pragma endregion

#pragma region CreateBackDepthStencilBuffersAndViews
		ComPtr<ID3D11Texture2D> pBackBuffer;
		D3D11_TEXTURE2D_DESC bbDesc;

		hr = m_swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			&pBackBuffer);
		if (FAILED(hr)) return hr;

		hr = m_d3dDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			&m_renderTargetView
		);
		if (FAILED(hr)) return hr;

		pBackBuffer->GetDesc(&bbDesc);

		// Depth Stencil
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			bbDesc.Width,
			bbDesc.Height,
			1, // Number of textures.
			1, // Single mipmap level. Use 1 for multisampled texture.
			D3D11_BIND_DEPTH_STENCIL
		);

		hr = m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&m_depthStencil
		);
		if (FAILED(hr)) return hr;

		// Depth Stencil View
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
			D3D11_DSV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			0
		);

		hr = m_d3dDevice->CreateDepthStencilView(
			m_depthStencil.Get(),
			&depthStencilViewDesc,
			&m_depthStencilView
		);
		if (FAILED(hr)) return hr;

		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
#pragma endregion

#pragma region CreateAndSetViewport
		// Create a viewport
		D3D11_VIEWPORT viewport = {
			.Width = (float)bbDesc.Width,
			.Height = (float)bbDesc.Height,
			.MinDepth = 0,
			.MaxDepth = 1
		};

		m_immediateContext->RSSetViewports(
			1,
			&viewport
		);
#pragma endregion

#pragma region CreateCBChangeOnResizeAndSetB1
		float fovAngleY = XM_PIDIV2;
		float nearZ = 0.01f;
		float farZ = 100.0f;
		m_projection = XMMatrixPerspectiveFovLH(fovAngleY, (float)bbDesc.Width / (float)bbDesc.Height, nearZ, farZ);


		D3D11_BUFFER_DESC cBufferDesc = {
			.ByteWidth = sizeof(CBChangeOnResize),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		CBChangeOnResize cb = {
			.Projection = XMMatrixTranspose(m_projection)
		};

		D3D11_SUBRESOURCE_DATA cData = {
			.pSysMem = &cb,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = m_d3dDevice->CreateBuffer(&cBufferDesc, &cData, &m_cbChangeOnResize);
		if (FAILED(hr)) return hr;

		m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
#pragma endregion

#pragma region CreateCBLightsAndSetB3
		D3D11_BUFFER_DESC cbLightsDesc = {
			.ByteWidth = sizeof(CBLights),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		CBLights cbLights = { };

		D3D11_SUBRESOURCE_DATA cbLightsData = {
			.pSysMem = &cbLights,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = m_d3dDevice->CreateBuffer(&cbLightsDesc, &cbLightsData, &m_cbLights);
		if (FAILED(hr)) return hr;

		m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
#pragma endregion

#pragma region InitializeShadersAndRenderables
		for (auto& vs : m_vertexShaders)
		{
			hr = vs.second->Initialize(m_d3dDevice.Get());
			if (FAILED(hr)) return hr;
		}

		for (auto& ps : m_pixelShaders)
		{
			hr = ps.second->Initialize(m_d3dDevice.Get());
			if (FAILED(hr)) return hr;
		}

		for (auto& renderable : m_renderables)
		{
			hr = renderable.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
			if (FAILED(hr)) return hr;
		}
#pragma endregion

		// Initialize Camera
		hr = m_camera.Initialize(m_d3dDevice.Get());
		if (FAILED(hr)) return hr;

		// Set primitive topology
		m_immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddRenderable

	  Summary:  Add a renderable object

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Shared pointer to the renderable object

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
	{
		if (m_renderables.count(pszRenderableName) > 0) return E_FAIL;
		m_renderables.insert({ pszRenderableName, renderable });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPointLight

	  Summary:  Add a point light

	  Args:     size_t index
				  Index of the point light
				const std::shared_ptr<PointLight>& pointLight
				  Shared pointer to the point light object

	  Modifies: [m_aPointLights].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
	{
		if (index >= NUM_LIGHTS || !pPointLight)
		{
			return E_FAIL;
		}

		m_aPointLights[index] = pPointLight;
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddVertexShader

	  Summary:  Add the vertex shader into the renderer

	  Args:     PCWSTR pszVertexShaderName
				  Key of the vertex shader
				const std::shared_ptr<VertexShader>&
				  Vertex shader to add

	  Modifies: [m_vertexShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		if (m_vertexShaders.count(pszVertexShaderName) > 0) return E_FAIL;
		m_vertexShaders.insert({ pszVertexShaderName, vertexShader });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPixelShader

	  Summary:  Add the pixel shader into the renderer

	  Args:     PCWSTR pszPixelShaderName
				  Key of the pixel shader
				const std::shared_ptr<PixelShader>&
				  Pixel shader to add

	  Modifies: [m_pixelShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		if (m_pixelShaders.count(pszPixelShaderName) > 0) return E_FAIL;
		m_pixelShaders.insert({ pszPixelShaderName, pixelShader });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::HandleInput

	  Summary:  Add the pixel shader into the renderer and initialize it

	  Args:     const DirectionsInput& directions
				  Data structure containing keyboard input data
				const MouseRelativeMovement& mouseRelativeMovement
				  Data structure containing mouse relative input data

	  Modifies: [m_camera].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
	{
		m_camera.HandleInput(
			directions,
			mouseRelativeMovement,
			deltaTime
		);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update

	  Summary:  Update the renderables each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		for (auto& renderable : m_renderables)
		{
			renderable.second->Update(deltaTime);
		}

		for (auto& light : m_aPointLights)
		{
			light->Update(deltaTime);
		}

		m_camera.Update(deltaTime);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render

	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Render()
	{
		// Clear the backbuffer
		const float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);

		// Clear the depth buffer to 1.0 (maximum depth)
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Create camera constant buffer and update
		XMFLOAT4 camPos;
		XMStoreFloat4(&camPos, m_camera.GetEye());
		CBChangeOnCameraMovement cbCamera = {
			.View = XMMatrixTranspose(m_camera.GetView()),
			.CameraPosition = camPos,
		};

		m_immediateContext->UpdateSubresource(
			m_camera.GetConstantBuffer().Get(),
			0u,
			nullptr,
			&cbCamera,
			0u,
			0u
		);
		m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

		// Create light constant buffer and update
		CBLights cbLights = { };

		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			if (!m_aPointLights[i]) continue;
			cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
			cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
		}

		m_immediateContext->UpdateSubresource(
			m_cbLights.Get(),
			0u,
			nullptr,
			&cbLights,
			0u,
			0u
		);

		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

		// For each renderables
		for (auto& pair : m_renderables)
		{
			auto& renderable = pair.second;

			// Set the vertex buffer
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,												// the first input slot for binding
				1,												// the number of buffers in the array
				renderable->GetVertexBuffer().GetAddressOf(),	// the array of vertex buffers
				&stride,										// array of stride values, one for each buffer
				&offset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

			// Create and update renderable constant buffer
			CBChangesEveryFrame cbRenderable = {
				.World = XMMatrixTranspose(renderable->GetWorldMatrix()),
				.OutputColor = renderable->GetOutputColor()
			};

			m_immediateContext->UpdateSubresource(
				renderable->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbRenderable,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);

			// Set renderable constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

			if (renderable->HasTexture())
			{
				for (UINT i = 0u; i < renderable->GetNumMeshes(); ++i)
				{
					const UINT materialIndex = renderable->GetMesh(i).uMaterialIndex;
					if (renderable->GetMaterial(materialIndex).pDiffuse)
					{
						// Set texture resource view of the renderable into the pixel shader
						m_immediateContext->PSSetShaderResources(0, 1, 
							renderable->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());

						// Set sampler state of the renderable into the pixel shader
						m_immediateContext->PSSetSamplers(0, 1, 
							renderable->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
					}
					m_immediateContext->DrawIndexed(
						renderable->GetMesh(i).uNumIndices,
						renderable->GetMesh(i).uBaseIndex,
						renderable->GetMesh(i).uBaseVertex);
				}
			}
			else
				// Render
				m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0u, 0);
		}

		// Present
		m_swapChain->Present(0u, 0u);

		// Set Render Target View again (Present call for DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbinds backbuffer 0)
		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfRenderable

	  Summary:  Sets the vertex shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
	{
		if (m_renderables.count(pszRenderableName) == 0 || m_vertexShaders.count(pszVertexShaderName) == 0)
		{
			return E_INVALIDARG;
		}
		auto& vs = m_vertexShaders[pszVertexShaderName];
		m_renderables[pszRenderableName]->SetVertexShader(vs);
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfRenderable

	  Summary:  Sets the pixel shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
	{
		if (m_renderables.count(pszRenderableName) == 0 || m_pixelShaders.count(pszPixelShaderName) == 0)
		{
			return E_INVALIDARG;
		}
		auto& ps = m_pixelShaders[pszPixelShaderName];
		m_renderables[pszRenderableName]->SetPixelShader(ps);
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetDriverType

	  Summary:  Returns the Direct3D driver type

	  Returns:  D3D_DRIVER_TYPE
				  The Direct3D driver type used
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	D3D_DRIVER_TYPE Renderer::GetDriverType() const
	{
		return m_driverType;
	}
}







