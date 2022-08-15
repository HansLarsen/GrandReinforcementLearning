#include "TCPManager.h"

#include "Util/Logger.hpp"

#include <cstddef>
#include <bitset>
#include <mutex>
#include <sstream>

void float2Bytes(float val, std::byte* bytes_array) {
    // Create union of shared memory space
    union {
        float float_variable;
        std::byte temp_array[4];
    } u;
    // Overite bytes of union with float variable
    u.float_variable = val;
    // Assign bytes to input array
    memcpy(bytes_array, u.temp_array, 4);
}

HWND g_HWND = NULL;
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);

    TCHAR buff[MAX_PATH];
    GetWindowTextA(hwnd, buff, MAX_PATH);
    //logger.Write(LogLevel::INFO, "Window Name Try: %s", buff);

    if (lpdwProcessId == lParam)// && _tcscmp(&buff[0], &letterG) )
    {
        g_HWND = hwnd;
        return FALSE;
    }
    return TRUE;
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        logger.Write(LogLevel::ERRORs, "Its fucked in getting data about a bit map");

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

    if (cClrBits < 24)
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * (1 << cClrBits));

    // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

    else
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
        * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
}

TCPManager::TCPManager()
{
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        //printf("WSAStartup failed: %d\n", iResult);
        logger.Write(LogLevel::ERRORs, "WSAStartup failed: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    while (attempts_counter < 10)
    {
        // Resolve the local address and port to be used by the server
        std::ostringstream oss;
        oss << DEFAULT_PORT + attempts_counter;
        iResult = getaddrinfo(NULL, oss.str().c_str(), &hints, &result);
        if (iResult != 0)
        {
            //printf("getaddrinfo failed: %d\n", iResult);
            logger.Write(LogLevel::ERRORs, "getaddrinfo failed: %d\n", iResult);
            WSACleanup();
            return;
        }

        ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ListenSocket == INVALID_SOCKET)
        {
            //printf("Error at socket(): %ld\n", WSAGetLastError());
            logger.Write(LogLevel::ERRORs, "Error at socket(): %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
        }

        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            //printf("bind failed with error: %d\n", WSAGetLastError());
            logger.Write(LogLevel::ERRORs, "bind failed with error: %d\n", WSAGetLastError());

            if (attempts_counter < 10) {
                attempts_counter += 1;
                continue;
            }

            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }
        logger.Write(LogLevel::INFO, "Started on port: %d\n", DEFAULT_PORT + attempts_counter);
        break;
    }

    freeaddrinfo(result);

    logger.Write(LogLevel::INFO, "TCP server started");
    running_variable = true;
	tcp_thread = std::thread(&TCPManager::tcp_run_thread, this);
    get_window_info();
}

TCPManager::~TCPManager()
{
    g_HWND = NULL;
    // after the recording is done, release the desktop context you got..
    ReleaseDC(NULL, hdc);

    // ..delete the bitmap you were using to capture frames..
    DeleteObject(hbDesktop);

    // ..and delete the context you created
    DeleteDC(hDest);

    logger.Write(LogLevel::INFO, "Ending Threads");

	running_variable = false;
	tcp_thread.join();
    tcp_data_thread.join();
    logger.Write(LogLevel::INFO, "Threads ended");
}

void TCPManager::setSpeedDistance(float speed, float dist)
{
    lock_for_speeds.lock();
    meta_data.speed = speed;
    meta_data.distance = dist;
    lock_for_speeds.unlock();
}

float TCPManager::getThrottle()
{
    throttle_lock.lock();
    float temp_throttle = throttle;
    throttle_lock.unlock();
    return temp_throttle;
}

float TCPManager::getSteeringAngle()
{
    throttle_lock.lock();
    float temp_throttle = sterring_angle;
    throttle_lock.unlock();
    return temp_throttle;
}

int TCPManager::getCommand()
{
    command_lock.lock();
    int return_value = command_buf;
    command_lock.unlock();
    command_buf = 0;
    return return_value;
}

void TCPManager::tcp_run_thread()
{
    logger.Write(LogLevel::INFO, "TCP server 2 started");
	while (running_variable)
	{
        try {
            //logger.Write(LogLevel::INFO, "Listening Connection");
            if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
            {
                //printf("Listen failed with error: %ld\n", WSAGetLastError());
                logger.Write(LogLevel::ERRORs, "Listen failed with error: %ld\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                running_variable = false;
                return;
            }

            //logger.Write(LogLevel::INFO, "Awaiting Connection");
            ClientSocket = accept(ListenSocket, NULL, NULL);
            if (ClientSocket == INVALID_SOCKET)
            {
                //printf("accept failed: %d\n", WSAGetLastError());
                logger.Write(LogLevel::ERRORs, "Accept failed: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                running_variable = false;
                client_connect = false;
                return;
            }
            //logger.Write(LogLevel::INFO, "Accepted Incoming Connection");
            client_connect = true;
                
            tcp_data_thread = std::thread(&TCPManager::tcp_data_thread_fun, this);
            tcp_data_reciv_thread = std::thread(&TCPManager::tcp_reciv_thread_fun, this);
            tcp_data_thread.join();
            tcp_data_reciv_thread.join();
            //logger.Write(LogLevel::INFO, "New Thread Create");

		    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //logger.Write(LogLevel::INFO, "Updating");
        }
        catch (const std::exception& e) {
            logger.Write(LogLevel::INFO, e.what());
        }
    }
}

void TCPManager::tcp_data_thread_fun()
{
    logger.Write(LogLevel::INFO, "Incoming connection thread");
    //std::vector<uint8_t> buffer;
    while (client_connect && running_variable) {
        try {
            sync_lock.lock();
            bool ready_go = ready_for_capture;
            sync_lock.unlock();
            if (ready_go){
                //std::this_thread::sleep_for(std::chrono::milliseconds(100));
                capture_gamewindow();

                sync_lock.lock();
                ready_for_capture = false;
                ready_camera_change = true;
                sync_lock.unlock();

                //logger.Write(LogLevel::INFO, "%d", image.length());

                /*lock_for_speeds.lock();
                std::string mystring = "{ \"Speed\":\"" + std::to_string(speed) + "\", \"Distance\":\"" + std::to_string(distance) + "\", \"Width\":\"" + std::to_string(width) + "\", \"Height\":\"" + std::to_string(height) + "\", \"Image\":\"" + image + "\" }";
                lock_for_speeds.unlock();*/

                //std::copy(mystring.begin(), mystring.end(), buffer);

                PBITMAPINFO myinfo = CreateBitmapInfoStruct(g_HWND, hbDesktop);
                LPBYTE lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, myinfo->bmiHeader.biSizeImage);

                if (!lpBits)
                    logger.Write(LogLevel::ERRORs, "Global Alloc failed");

                // Retrieve the color table (RGBQUAD array) and the bits  
                // (array of palette indices) from the DIB.  
                if (!GetDIBits(hdc, hbDesktop, 0, (WORD)myinfo->bmiHeader.biHeight, lpBits, myinfo,
                    DIB_RGB_COLORS))
                {
                    logger.Write(LogLevel::ERRORs, "GetDIBits");
                }

                //logger.Write(LogLevel::INFO, "%d biWidth: %d biHeight %d", myinfo->bmiHeader.biSizeImage, myinfo->bmiHeader.biSize, myinfo->bmiHeader.biHeight);
                lock_for_speeds.lock();
                meta_data.nWidth = width - 6; // width - 6;
                meta_data.nHeight = height - 31;
                meta_data.image_size = myinfo->bmiHeader.biSizeImage;
                logger.Write(LogLevel::INFO, "%d", sizeof(meta_data));
                int new_bytes = send(ClientSocket, (const char*)&meta_data, sizeof(meta_data), NULL);
                lock_for_speeds.unlock();

                if (new_bytes == -1) {
                    client_connect = false;
                    closesocket(ClientSocket);
                    logger.Write(LogLevel::INFO, "Failed to send header: Connection Closed");
                    return;
                }

                new_bytes = send(ClientSocket, (const char*)lpBits, meta_data.image_size, NULL);

                if (new_bytes == -1) {
                    logger.Write(LogLevel::INFO, "Failed to send image");
                }

                GlobalFree((HGLOBAL)lpBits);
            }
        }
        catch (const std::exception& e) {
            client_connect = false;
            closesocket(ClientSocket);
            logger.Write(LogLevel::INFO, "Connection Closed ", e.what());
            return;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

}
void TCPManager::tcp_reciv_thread_fun()
{
    char outputbuffer[12];
    while (client_connect && running_variable) {
        try {
            int status = recv(ClientSocket, outputbuffer, 12, NULL);
            int temp_steering, temp_throttle, temp_command;
            std::memcpy(&temp_command, &outputbuffer[8], sizeof temp_command);
            std::memcpy(&temp_steering, &outputbuffer[4], sizeof temp_steering);
            std::memcpy(&temp_throttle, &outputbuffer[0], sizeof temp_throttle);

            if (temp_steering > 100) {
                temp_steering = 100;
            }
            else if (temp_steering < 0) {
                temp_steering = 0;
            }

            if (temp_throttle > 100) {
                temp_throttle = 100;
            }
            else if (temp_throttle < 0) {
                temp_throttle = 0;
            }

            throttle_lock.lock();
            sterring_angle = (float)temp_steering;
            throttle = (float)temp_throttle;
            throttle_lock.unlock();

            command_lock.lock();
            command_buf = temp_command;
            command_lock.unlock();
        }
        catch (const std::exception& e) {
            client_connect = false;
            closesocket(ClientSocket);
            logger.Write(LogLevel::INFO, "Connection Closed ", e.what());
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void TCPManager::get_window_info()
{
    /*if (EnumWindows(EnumWindowsProcMy, GetCurrentProcessId()) == 0) {
        logger.Write(LogLevel::INFO, "Failed to get program window");
        window_works = false;
        return;
    }*/
    g_HWND = FindWindowA(NULL, "Grand Theft Auto V");

    hdc = GetWindowDC(g_HWND); // get the desktop device context
    hDest = CreateCompatibleDC(hdc); // create a device context to use yourself

    windowRect = RECT();
    GetWindowRect(g_HWND, &windowRect);

    // get the height and width of the screen
    height = windowRect.bottom - windowRect.top;
    width = windowRect.right - windowRect.left;

    // create a bitmap
    hbDesktop = CreateCompatibleBitmap(hdc, width - 6, height - 31);

    // use the previously created device context with the bitmap
    SelectObject(hDest, hbDesktop);

    TCHAR buff[MAX_PATH];
    GetWindowTextA(g_HWND, buff, MAX_PATH);
    logger.Write(LogLevel::INFO, "Window Name: %s", buff);
}

void TCPManager::camera_changed(bool thirdPerson)
{
    lock_for_speeds.lock();
    meta_data.thirdPersonCamera = thirdPerson;
    lock_for_speeds.unlock();

    sync_lock.lock();
    ready_for_capture = true;
    ready_camera_change = false;
    sync_lock.unlock();
}

bool TCPManager::readyCameraChange()
{
    sync_lock.lock();
    bool temp_return = ready_camera_change;
    sync_lock.unlock();
    return temp_return;
}

void TCPManager::capture_gamewindow()
{
    // copy from the desktop device context to the bitmap device context
    // call this once per 'frame'
    BitBlt(hDest, 0, 0, width - 3, height - 31, hdc, 3, 29, SRCCOPY);
    
    //CImage myimage;
    //myimage.Attach(hbDesktop);

    return;// as_bytes(myimage);
    //std::vector<uint8_t> image_compressed = as_bytes(myimage);
    //return to_base64(image_compreseed);
}
