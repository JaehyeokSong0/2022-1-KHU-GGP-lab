/*+===================================================================
  File:      BASEWINDOW.H

  Summary:   BaseWindow header file contains declarations of the
			 base class of all windows used in the library.

  Classes: BaseWindow<DerivedType>

  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

namespace library
{
	/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
	  Class:    BaseWindow

	  Summary:  An abstract base class from specific window

	  Methods:  WindowProc
				  The window procedure of the window
				Initialize
					Purely virtual function that initializes window
				GetWindowClassName
					Purely virtual function that returns the name of
					the window class
				HandleMessage
					Purely virtual function that that handles the
					messages
				GetWindow
					Getter for the handle to the window
				BaseWindow
					Constructor.
				~BaseWindow
					Destructor.
	C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
	template <class DerivedType>
	class BaseWindow
	{
	public:
		static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

		BaseWindow();
		BaseWindow(const BaseWindow& rhs) = delete;
		BaseWindow(BaseWindow&& rhs) = delete;
		BaseWindow& operator=(const BaseWindow& rhs) = delete;
		BaseWindow& operator=(BaseWindow&& rhs) = delete;
		virtual ~BaseWindow() = default;

		virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
		virtual PCWSTR GetWindowClassName() const = 0;
		virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

		HWND GetWindow() const;

	protected:
		HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x = CW_USEDEFAULT,
			_In_opt_ INT y = CW_USEDEFAULT,
			_In_opt_ INT nWidth = CW_USEDEFAULT,
			_In_opt_ INT nHeight = CW_USEDEFAULT,
			_In_opt_ HWND hWndParent = nullptr,
			_In_opt_ HMENU hMenu = nullptr
		);

		HINSTANCE m_hInstance;
		HWND m_hWnd;
		LPCWSTR m_pszWindowName;
	};


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		Method:   BaseWindow<DerivedType>::WindowProc

		Summary:  Defines the behavior of the window—its appearance, how
				it interacts with the user, and so forth

		Args:     HWND hWnd
					Handle to the window
				UINT uMessage
					Message code
				WPARAM wParam
					Additional data the pertains to the message
				LPARAM lParam
					Additional data the pertains to the message

		Modifies: [m_hWnd].

		Returns:  LRESULT
					Integer value that your program returns to Windows
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	template <class DerivedType>
	LRESULT BaseWindow<DerivedType>::WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
		DerivedType* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DerivedType*)pCreate->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
			pThis->m_hWnd = hWnd;
		}
		else
		{
			pThis = (DerivedType*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		Method:   BaseWindow<DerivedType>::BaseWindow

		Summary:  Constructor

		Modifies: [m_hInstance, m_hWnd, m_pszWindowName].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	template<class DerivedType>
	BaseWindow<DerivedType>::BaseWindow() :
		m_hInstance(),
		m_hWnd(),
		m_pszWindowName()
	{}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		Method:   BaseWindow<DerivedType>::GetWindow()

		Summary:  Returns the handle to the window

		Returns:  HWND
					The handle to the window
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	template<class DerivedType>
	HWND BaseWindow<DerivedType>::GetWindow() const
	{
		return m_hWnd;
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   BaseWindow<DerivedType>::initialize

	  Summary:  Registers the window class and creates a window

	  Args:     HINSTANCE hInstance
				  Handle to the instance
				INT nCmdShow
				  Is a flag that says whether the main application window
				  will be minimized, maximized, or shown normally
				PCWSTR pszWindowName
				  The window name
				DWORD dwStyle
				  The style of the window being created
				INT x
				  The initial horizontal position of the window
				INT y
				  The initial vertical position of the window
				INT nWidth
				  The width, in device units, of the window
				INT nHeight
				  The height, in device units, of the window
				HWND hWndParent
				  A handle to the parent or owner window of the window
				  being created
				HMENU hMenu
				  A handle to a menu, or specifies a child-window
				  identifier depending on the window style

	  Modifies: [m_hInstance, m_pszWindowName, m_hWnd].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	template<class DerivedType>
	HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu)
	{
		m_hInstance = hInstance;
		m_pszWindowName = pszWindowName;

		// Registers the window class
		WNDCLASS wc = {
			.lpfnWndProc = DerivedType::WindowProc,
			.hInstance = hInstance,
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = GetWindowClassName()
		};

		RegisterClass(&wc);

		// Creates a window
		m_hWnd = CreateWindowEx(
			0,									// Optional window styles.
			GetWindowClassName(),			// Window class
			pszWindowName,						// Window text
			dwStyle,							// Window style

			// Size and position
			x, y, nWidth, nHeight,

			hWndParent,		// Parent window    
			hMenu,			// Menu
			hInstance,		// Instance handle
			this			// Additional application data
		);

		if (m_hWnd == nullptr)
		{
			DWORD dwError = GetLastError();
			return HRESULT_FROM_WIN32(dwError);
		}

		// Shows the window
		ShowWindow(m_hWnd, nCmdShow);

		// Returns a result code of HRESULT type
		return S_OK;
	}
}