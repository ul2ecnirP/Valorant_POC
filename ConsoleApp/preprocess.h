#pragma once

#define PROCESS_NAME //REDACTED

#include <TlHelp32.h>
#include <stdio.h>
#include <utility>

struct Vector4{
    float x,y,z,w;
};




class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	float x;
	float y;
	float z;

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		float x = this->x - v.x;
		float y = this->y - v.y;
		float z = this->z - v.z;

		return sqrtf((x * x) + (y * y) + (z * z)) * 0.03048f;
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(float number) const {
		return Vector3(x * number, y * number, z * number);
	}

	__forceinline float Magnitude() const {
		return sqrtf(x * x + y * y + z * z);
	}

	inline float Length()
	{
		return sqrtf((x * x) + (y * y) + (z * z));
	}

	__forceinline Vector3 Normalize() {
		Vector3 vector;
		float length = this->Magnitude();

		if (length != 0) {
			vector.x = x / length;
			vector.y = y / length;
			vector.z = z / length;
		}
		else {
			vector.x = vector.y = 0.0f;
			vector.z = 1.0f;
		}
		return vector;
	}

	__forceinline Vector3& operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
};
struct FMinimalViewInfo {
	Vector3 Location; // 0x00(0x0c)
	Vector3 Rotation; // 0x0c(0x0c)
	float FOV; // 0x18(0x04)
};
struct BoneAtriculation{
    Vector3 RThigh1; //cuisse droit 1
    Vector3 RThigh2; //cuisse droit 2
    Vector3 RFoot; //pied droit
    Vector3 LThigh1; //cuisse gauche 1
    Vector3 LThigh2; //cuisse gauche 2
    Vector3 LFoot; //pied gauche
    Vector3 Chest; //torse
    Vector3 Stomach; //estomac
    Vector3 Pelvis; //pelvis
    Vector3 LUpperarm; //haut du bras gauche
    Vector3 LForearm1; //avant bras gauche 1
    Vector3 LForearm2; // avant bras gauche 2
    Vector3 RUpperarm; //haut du bras droit
    Vector3 RForearm1; //avant bras droit 1 
    Vector3 RForearm2; //avant bras droit 2 
    Vector3 Neck; //nuque
    Vector3 Head; //tête
};

struct Player{
	float vie;
	Vector3 pos;
    bool is_in_game = 0;
    BoneAtriculation bones;
};



uintptr_t GetModuleBaseAdress(int procID, const char* modname) {
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modentry;
		modentry.dwSize = sizeof(modentry);
		if (Module32First(hSnap, &modentry)) {
			do
			{
				if (!strcmp((char*)modentry.szModule, modname)) {
					modBaseAddr = (uintptr_t)modentry.modBaseAddr;
					printf("BASEADDR found\n");
					return modBaseAddr;

				}
			} while (Module32Next(hSnap, &modentry));

		}
	}
	return 1;
}


int GetProcId(const char* name) {
	int procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procentry;
		procentry.dwSize = sizeof(procentry);
		if (Process32First(hSnap, &procentry)) {
			do
			{
				if (!strcmp((char*)procentry.szExeFile, name))
				{
					procId = procentry.th32ProcessID;
					return procId;
				}
			} while (Process32Next(hSnap, &procentry));

		}
	}
	return -1;
}
int ScreenX, ScreenY;
void GetSWindowsSize(HWND window){
    RECT rect;
    if(GetWindowRect(window, &rect))
    {
    ScreenX = (rect.right - rect.left)-16;
    ScreenY = (rect.bottom - rect.top)-39;
    }
}
auto get_process_wnd(uint32_t pid) -> HWND
{
	std::pair<HWND, uint32_t> params = { 0, pid };
	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
		auto pParams = (std::pair<HWND, uint32_t>*)(lParam);
		uint32_t processId = 0;

		if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processId)) && processId == pParams->second) {
			SetLastError((uint32_t)-1);
			pParams->first = hwnd;
			return FALSE;
		}

		return TRUE;

		}, (LPARAM)&params);

	if (!bResult && GetLastError() == -1 && params.first)
		return params.first;

	return NULL;
}


DWORD pid = GetProcId(PROCESS_NAME);
uintptr_t uworld_global;
HDC hdc_global;
//--------------------------
auto driver = new driver_manager("\\\\.\\1337Driver", pid);
//--------------------------
int BaseAddress = 0;