#pragma once
#include "..\..\..\inc\enums.h"
#include "..\..\..\inc\types.h"
#include <vector>

namespace Controls {
    void SetControlADZ(eControl control, float value, float adz);
}

namespace Util {
    bool PlayerAvailable(Player player, Ped playerPed);
    bool VehicleAvailable(Vehicle vehicle, Ped playerPed);
    bool IsPedOnSeat(Vehicle vehicle, Ped ped, int seat);
    std::vector<Vector3> GetWheelCoords(Vehicle handle);
}
