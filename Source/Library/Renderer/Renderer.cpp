#include "Renderer/Renderer.h"

namespace library
{

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
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_HARDWARE)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_1)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_cbLights()
        , m_pszMainSceneName(L"")
        , m_padding()
        , m_camera(XMVectorSet(0.0f, 8.0f, -12.0f, 0.0f))
        , m_projection()

        , m_renderables()
        , m_models() // added at lab08
        , m_aPointLights()
        , m_vertexShaders()
        , m_pixelShaders()
        , m_scenes()
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
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);

        POINT p1 =
        {
            .x = rc.left,
            .y = rc.top
        };
        POINT p2 =
        {
            .x = rc.right,
            .y = rc.bottom
        };

        ClientToScreen(hWnd, &p1);
        ClientToScreen(hWnd, &p2);

        rc.left = p1.x;
        rc.top = p1.y;
        rc.right = p2.x;
        rc.bottom = p2.y;

        ClipCursor(&rc);

        UINT createDeviceFlags = 0;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);

            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);

        if (dxgiFactory2)
        {
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
                hr = m_immediateContext.As(&m_immediateContext1);

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd,
                nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
                hr = m_swapChain1.As(&m_swapChain);
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {.Numerator = 60, .Denominator = 1 },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM
                },
                .SampleDesc = {.Count = 1, .Quality = 1 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd,
                m_swapChain.GetAddressOf());
        }

        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr< ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);

        // Create the constant buffer
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
            return hr;

        CBChangeOnResize cbChangeOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };

        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangeOnResize, 0, 0);

        // Create the constant buffer of light
        D3D11_BUFFER_DESC bdLight =
        {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };

        hr = m_d3dDevice->CreateBuffer(&bdLight, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
            return hr;

        CBLights cbLights = {};

        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

        // Initialize the shaders 
        for (auto pixelShadersElem : m_pixelShaders)
            pixelShadersElem.second->Initialize(m_d3dDevice.Get());

        for (auto vertexShadersElem : m_vertexShaders)
            vertexShadersElem.second->Initialize(m_d3dDevice.Get());

        // Initialize the renderables
        for (auto renderablesElem : m_renderables)
            renderablesElem.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        // Initialize the voxels
        for (auto voxelElem : m_scenes)
        {
            for (int i = 0; i < voxelElem.second->GetVoxels().size(); ++i)
                voxelElem.second->GetVoxels()[i]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        // Initialize the models
        for (auto modelElem : m_models)
            modelElem.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddModel

      Summary:  Add a model object

      Args:     PCWSTR pszModelName
                  Key of the model object
                const std::shared_ptr<Model>& pModel
                  Shared pointer to the model object

      Modifies: [m_models].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel)
    {
        if (m_models.count(pszModelName) > 0)
            return E_FAIL;

        m_models[pszModelName] = pModel;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables and models each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        for (auto& renderable : m_renderables)
            renderable.second->Update(deltaTime);

        for (auto& light : m_aPointLights)
            light->Update(deltaTime);

        for (auto& model : m_models)
            model.second->Update(deltaTime);

        m_camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear the depth buffer to 1.0 (max depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Create camera constant buffer and update 
        m_camera.Initialize(m_d3dDevice.Get());
        CBChangeOnCameraMovement cbChangeOnCameraMovement =
        {
            .View = XMMatrixTranspose(m_camera.GetView())
        };
        XMStoreFloat4(&cbChangeOnCameraMovement.CameraPosition, m_camera.GetEye());
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
        };

        m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());

        // Set the constant buffer to each shaders accordingly
        CBLights cbLights = { };
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cbLights, 0, 0);

        for (auto renderablesElem = m_renderables.begin(); renderablesElem != m_renderables.end(); ++renderablesElem)
        {
            auto renderable = renderablesElem->second;

            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0;
            m_immediateContext->IASetVertexBuffers(0, 1, renderable->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

            // Create renderable constant buffer and update
            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(renderable->GetWorldMatrix()),
                .OutputColor = renderable->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(renderable->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0, 0);

            // Set shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            if (renderable->HasTexture())
            {
                for (UINT i = 0u; i < renderable->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = renderable->GetMesh(i).uMaterialIndex;
                    m_immediateContext->PSSetShaderResources(0u, 1u, renderable->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0u, 1u, renderable->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());

                    // Draw
                    m_immediateContext->DrawIndexed(renderable->GetMesh(i).uNumIndices,
                        renderable->GetMesh(i).uBaseIndex, renderable->GetMesh(i).uBaseVertex);
                }
            }
            else
                m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
        }

        // Voxel
        for (auto voxelsElem = m_scenes.begin(); voxelsElem != m_scenes.end(); ++voxelsElem)
        {
            auto voxel = voxelsElem->second;

            for (UINT i = 0u; i < voxel->GetVoxels().size(); ++i)
            {
                // Set the vertex buffer
                UINT stride = sizeof(SimpleVertex);
                UINT offset = 0u;
                m_immediateContext->IASetVertexBuffers(0u, 1u, voxel->GetVoxels()[i]->GetVertexBuffer().GetAddressOf(), &stride, &offset);

                // Set the index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetVoxels()[i]->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the instance buffer
                stride = sizeof(InstanceData);
                m_immediateContext->IASetVertexBuffers(1u, 1u, voxel->GetVoxels()[i]->GetInstanceBuffer().GetAddressOf(), &stride, &offset);

                // Set primitive topology
                m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                // Set the input layout
                m_immediateContext->IASetInputLayout(voxel->GetVoxels()[i]->GetVertexLayout().Get());

                CBChangesEveryFrame cbChangesEveryFrame =
                {
                    .World = XMMatrixTranspose(voxel->GetVoxels()[i]->GetWorldMatrix()),
                    .OutputColor = voxel->GetVoxels()[i]->GetOutputColor()
                };

                m_immediateContext->UpdateSubresource(voxel->GetVoxels()[i]->GetConstantBuffer().Get(), 0, nullptr, &cbChangesEveryFrame, 0, 0);

                // Set shaders and constant buffers, shader resources, and samplers
                m_immediateContext->VSSetShader(voxel->GetVoxels()[i]->GetVertexShader().Get(), nullptr, 0); // exception
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetVoxels()[i]->GetConstantBuffer().GetAddressOf());

                m_immediateContext->PSSetShader(voxel->GetVoxels()[i]->GetPixelShader().Get(), nullptr, 0);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetVoxels()[i]->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                // Draw
                m_immediateContext->DrawIndexedInstanced(voxel->GetVoxels()[i]->GetNumIndices(), voxel->GetVoxels()[i]->GetNumInstances(), 0, 0, 0);
            }
        }

        // Model
        for (auto modelElem = m_models.begin(); modelElem != m_models.end(); ++modelElem)
        {
            // Set the vertex buffer
            UINT aStrides[2] =
            {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(AnimationData)),
            };
            UINT aOffsets[2] = { 0u, 0u };

            ComPtr<ID3D11Buffer> aBuffers[2]
            {
                modelElem->second->GetVertexBuffer(),
                modelElem->second->GetAnimationBuffer()
            };

            for (UINT i = 0; i < 2; ++i)
                m_immediateContext->IASetVertexBuffers(i, 1, aBuffers[i].GetAddressOf(), &aStrides[i], &aOffsets[i]);

            // Set the index buffer 
            m_immediateContext->IASetIndexBuffer(modelElem->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            m_immediateContext->IASetInputLayout(modelElem->second->GetVertexLayout().Get());

            CBSkinning cbSkinning;
            for (UINT i = 0; i < modelElem->second->GetBoneTransforms().size(); ++i)
                cbSkinning.BoneTransforms[i] = XMMatrixTranspose(modelElem->second->GetBoneTransforms()[i]);
            m_immediateContext->UpdateSubresource(modelElem->second->GetSkinningConstantBuffer().Get(), 0, nullptr, &cbSkinning, 0, 0);

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(modelElem->second->GetWorldMatrix()),
                .OutputColor = modelElem->second->GetOutputColor(),
            };
            m_immediateContext->UpdateSubresource(modelElem->second->GetConstantBuffer().Get(), 0, nullptr, &cbChangesEveryFrame, 0, 0);

            // Set shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(modelElem->second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, modelElem->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(4, 1, modelElem->second->GetSkinningConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(modelElem->second->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, modelElem->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(4, 1, modelElem->second->GetSkinningConstantBuffer().GetAddressOf());

            if (modelElem->second->HasTexture())
            {
                for (UINT i = 0; i < modelElem->second->GetNumMeshes(); ++i)
                {
                    UINT MaterialIndex = modelElem->second->GetMesh(i).uMaterialIndex;
                    m_immediateContext->PSSetShaderResources(0, 1, modelElem->second->GetMaterial(MaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 1, modelElem->second->GetMaterial(MaterialIndex).pDiffuse->GetSamplerState().GetAddressOf());

                    m_immediateContext->DrawIndexed(modelElem->second->GetMesh(i).uNumIndices,
                        modelElem->second->GetMesh(i).uBaseIndex,
                        modelElem->second->GetMesh(i).uBaseVertex);
                }
            }
            else
                m_immediateContext->DrawIndexed(modelElem->second->GetNumIndices(), 0, 0);
        }

        // Present the information rendered to the back buffer to the front buffer
        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_models.count(pszModelName) == 0 || m_vertexShaders.count(pszVertexShaderName) == 0)
            return E_FAIL;

        m_models[pszModelName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_models.count(pszModelName) == 0 || m_pixelShaders.count(pszPixelShaderName) == 0)
            return E_FAIL;

        m_models[pszModelName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }
}