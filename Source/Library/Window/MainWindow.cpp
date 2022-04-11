#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        return initialize(hInstance, nCmdShow, pszWindowName, WS_OVERLAPPEDWINDOW);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    PCWSTR MainWindow::GetWindowClassName() const
    {
        return m_pszWindowName;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(_In_ UINT uMessage, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        RAWINPUTDEVICE Rid[1];
        Rid[0].usUsagePage = (USHORT)0x01;
        Rid[0].usUsage = (USHORT)0x02;
        Rid[0].dwFlags = RIDEV_INPUTSINK;
        Rid[0].hwndTarget = m_hWnd;
        RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

        switch (uMessage)
        {
        case WM_INPUT:
        {
            UINT dwSize = sizeof(RAWINPUT);
            static BYTE lpb[sizeof(RAWINPUT)];

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                int xPosRelative = raw->data.mouse.lLastX;
                int yPosRelative = raw->data.mouse.lLastY;
            }
            break;
        }
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case 0x57: // W
                m_directions.bFront = true;
                break;
            case 0x41: // A
                m_directions.bLeft = true;
                break;
            case 0x53: // S
                m_directions.bBack = true;
                break;
            case 0x44: // D 
                m_directions.bRight = true;
                break;
            case VK_SPACE: // Space
                m_directions.bUp = true;
                break;
            case VK_SHIFT: // Shift
                m_directions.bDown = true;
                break;
            }
            break;
        }
        case WM_KEYUP:
        {
            switch (wParam)
            {
            case 0x57:
                m_directions.bFront = false;
                break;
            case 0x41:
                m_directions.bLeft = false;
                break;
            case 0x53:
                m_directions.bBack = false;
                break;
            case 0x44:
                m_directions.bRight = false;
                break;
            case VK_SPACE:
                m_directions.bUp = false;
                break;
            case VK_SHIFT:
                m_directions.bDown = false;
                break;
            }
            break;
        }
        case WM_PAINT:
        {
            hdc = BeginPaint(m_hWnd, &ps);
            EndPaint(m_hWnd, &ps);
            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(m_hWnd);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(m_hWnd, uMessage, wParam, lParam);
        }

        return 0;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections
      Summary:  Returns the keyboard direction input
      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement
      Summary:  Returns the mouse relative movement
      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement
      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement = 
        { 
            .X = 0l,
            .Y = 0l
        };
    }
}