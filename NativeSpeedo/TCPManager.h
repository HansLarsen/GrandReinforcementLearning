#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <mutex>

// This should be placed before #include <Windows.h> if the windows include is used
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <cstdio>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

#include <atlimage.h>
#include <comdef.h>
#include <Shlwapi.h>
#include <vector>

#include <wincrypt.h>
#include <string>

#pragma comment(lib, "Crypt32.lib")

#include "..\..\inc\natives.h"

struct metadata {
	const unsigned char delimiter[8] = {0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0};
	float speed = 0.0f;
	float distance = 0.0f;
	int nWidth = 800;
	int nHeight = 600;
	int image_size = nWidth * nHeight * 3;
	bool thirdPersonCamera = false;
};

class TCPManager
{
public:
	TCPManager();
	~TCPManager();

	void setSpeedDistance(float speed, float dist);
	float getThrottle();
	float getSteeringAngle();

	int getCommand();
	void get_window_info();
	void camera_changed(bool thirdPerson);
	bool readyCameraChange();

private:
	void tcp_run_thread();
	void tcp_data_thread_fun();
	void tcp_reciv_thread_fun();

	void capture_gamewindow();
	unsigned short DEFAULT_PORT = 3223;
	int height;
	int width;
	HDC hdc;
	HDC hDest;
	HBITMAP hbDesktop;
	RECT windowRect;

	std::thread tcp_thread;
	std::thread tcp_data_thread;
	std::thread tcp_data_reciv_thread;
	bool running_variable = true;

	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int attempts_counter = 0;
	SOCKET ListenSocket;

	bool client_connect = false;
	SOCKET ClientSocket;

	std::mutex lock_for_speeds;
	metadata meta_data = metadata();

	std::mutex throttle_lock;
	float throttle = 0.0f;
	float sterring_angle = 0.0f;

	std::mutex command_lock;
	int command_buf = 0;

	std::mutex sync_lock;
	bool ready_for_capture = false;
	bool ready_camera_change = false;
};