#include "script.h"
#include "Memory/MemoryPatcher.hpp"
#include "Memory/VehicleExtensions.hpp"
#include "..\..\inc\natives.h"
#include "Transforms.h"

using VExt = VehicleExtensions;
TCPManager *tcp_server;

//Vector3 startingLocation = Vector3(-1068.189f, -783.214f, 19.0f);
Vector3 startingLocation = Vector3(38.831593f, -1539.847534f, 29.342484f);
float startingHeading = 318.750641f;
Vehicle old_car = NULL;
bool camera_init = false;
bool inThridPerson = false;
Cam customCam;
int changed = 0;

void reset_car() 
{
	STREAMING::REQUEST_MODEL(2891838741u);

	while (!STREAMING::HAS_MODEL_LOADED(2891838741u)) {
		WAIT(0);
	}

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	/*Vector3 location = ENTITY::GET_ENTITY_COORDS(playerPed, false);
	Vector3 out;
	float heading;
	PATHFIND::GET_CLOSEST_VEHICLE_NODE_WITH_HEADING(location.x, location.y, location.z, &out, &heading, 0, 100.0, 2.5);*/

	logger.Write(LogLevel::DEBUG, "Spawning New Car");

	if (old_car != NULL) {
		VEHICLE::DELETE_VEHICLE(&old_car);
	}

	WAIT(30);

	TIME::SET_CLOCK_TIME(12, 0, 0);
	Vehicle new_car = VEHICLE::CREATE_VEHICLE(2891838741u, startingLocation.x, startingLocation.y, startingLocation.z, startingHeading, false, false);
	old_car = new_car;
	VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(new_car, false);
	VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(new_car);
	char text[] = "HAXs";
	VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(new_car, text);
	VEHICLE::SET_VEHICLE_DOORS_LOCKED(new_car, 4);
	PED::SET_PED_INTO_VEHICLE(playerPed, new_car, -1);
	PED::SET_PED_CAN_BE_DRAGGED_OUT(playerPed, false);
	PED::SET_PED_CAN_BE_SHOT_IN_VEHICLE(playerPed, false);
	PED::SET_PED_CAN_RAGDOLL(playerPed, false);

	PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
	PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);

	PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(playerPed, true);
	ENTITY::SET_ENTITY_INVINCIBLE(playerPed, true);
	AUDIO::SET_VEHICLE_RADIO_ENABLED(new_car, false);
}

void reset_car_road()
{
	STREAMING::REQUEST_MODEL(2891838741u);

	while (!STREAMING::HAS_MODEL_LOADED(2891838741u)) {
		WAIT(0);
	}

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	Vector3 location = ENTITY::GET_ENTITY_COORDS(playerPed, false);
	Vector3 out;
	float heading;
	PATHFIND::GET_CLOSEST_VEHICLE_NODE_WITH_HEADING(location.x, location.y, location.z, &out, &heading, 2, 0.0, 0.0);

	logger.Write(LogLevel::DEBUG, "Spawning New Car");

	if (old_car != NULL) {
		VEHICLE::DELETE_VEHICLE(&old_car);
	}

	WAIT(30);

	TIME::SET_CLOCK_TIME(12, 0, 0);
	Vehicle new_car = VEHICLE::CREATE_VEHICLE(2891838741u, location.x, location.y, location.z, heading, false, false);
	old_car = new_car;
	VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(new_car, false);
	VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(new_car);
	char text[] = "HAXs";
	VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(new_car, text);
	VEHICLE::SET_VEHICLE_DOORS_LOCKED(new_car, 4);
	PED::SET_PED_INTO_VEHICLE(playerPed, new_car, -1);
	PED::SET_PED_CAN_BE_DRAGGED_OUT(playerPed, false);
	PED::SET_PED_CAN_BE_SHOT_IN_VEHICLE(playerPed, false);
	PED::SET_PED_CAN_RAGDOLL(playerPed, false);

	PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
	PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);

	PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(playerPed, true);
	ENTITY::SET_ENTITY_INVINCIBLE(playerPed, true);
	AUDIO::SET_VEHICLE_RADIO_ENABLED(new_car, false);
}

void print_position() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	Vector3 position = ENTITY::GET_ENTITY_COORDS(playerPed, false);
	float heading = ENTITY::GET_ENTITY_HEADING(playerPed);

	logger.Write(LogLevel::INFO, "X: %f, Y: %f, Z: %f Heading: %f", position.x, position.y, position.z, heading);
}

void update()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
		return;

	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
		return;

	// check if player is in a vehicle and vehicle name isn't being displayed as well as player's phone
	const int HUD_VEHICLE_NAME = 6;
	if (UI::IS_HUD_COMPONENT_ACTIVE(HUD_VEHICLE_NAME) || PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed))
	{
		return;
	}

	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE)) {
		/*if (MemoryPatcher::NumGearboxPatched != MemoryPatcher::NumGearboxPatches) {
			MemoryPatcher::ApplyGearboxPatches();
		}*/
		if (!MemoryPatcher::BrakePatcher.Patched()) {
			MemoryPatcher::PatchBrake();
		}
		if (!MemoryPatcher::ThrottlePatcher.Patched()) {
			MemoryPatcher::PatchThrottle();
		}
		if (!MemoryPatcher::SteeringAssistPatcher.Patched()) {
			MemoryPatcher::PatchSteeringAssist();
		}
		if (!MemoryPatcher::SteeringControlPatcher.Patched()) {
			MemoryPatcher::PatchSteeringControl();
		}
		if (!MemoryPatcher::ThrottleControlPatcher.Patched()) {
			MemoryPatcher::ThrottleControlPatcher.Patch();
		}

		Vehicle car = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

		Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(car);
		Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(car);
		Vector3 position = ENTITY::GET_ENTITY_COORDS(playerPed, false);

		//Generate reward values, speed, distance, etc...
		double length_vec = sqrt(pow(forward.x, 2) + pow(forward.y, 2));
		//Vector3 forward_vector = Vector3();
		//forward_vector.x = forward.x / length_vec;
		//forward_vector.y = forward.y / length_vec;
		float speed = ((float)forward.x * (float)velocity.x + (float)forward.y * (float)velocity.y) / length_vec; // / (forward_vector.x * forward_vector.x + forward_vector.y * forward_vector.y); //Probably an easier way

		Vector3 outposition;
		float heading;
		PATHFIND::GET_CLOSEST_VEHICLE_NODE_WITH_HEADING(position.x, position.y, position.z, &outposition, &heading, 0, 0.0, 0);

		//float distance = sqrt(pow(outposition.x - position.x, 2) + pow(outposition.y - position.y, 2)); // + pow(outposition.z - position.z, 2)
		float distance = abs(heading - ENTITY::GET_ENTITY_HEADING(car));
		tcp_server->setSpeedDistance(speed, distance);

		//Update the steering of the car.
		float angle = tcp_server->getSteeringAngle();
		float correct_angle = ((angle / 100.0f) - 0.5f)*2.0f;

		VExt::SetSteeringAngle(car, correct_angle);
		VExt::SetSteeringInputAngle(car, correct_angle);

		float throttle = tcp_server->getThrottle();
		float correct_throttle = ((throttle / 100.0f) - 0.5f) * 2.0f;

		if (abs(correct_throttle) > 0.01) {
			VExt::SetThrottle(car, correct_throttle);
			VExt::SetThrottleP(car, correct_throttle);
			//VExt::SetCurrentRPM(car, abs(correct_throttle));
		}
		else
		{
			VExt::SetThrottle(car, 0);
			VExt::SetThrottleP(car, 0);
			//VExt::SetCurrentRPM(car, 0);
		}
		

		if (abs(speed) > 0.01 && (correct_throttle < 0) != (speed < 0)) {
			VExt::SetBrakeP(car, abs(correct_throttle));
		}
		else
		{
			VExt::SetBrakeP(car, 0.0f);
		}
		
		VExt::SetHandbrake(car, false);

		if (camera_init) {
			Vector3 position = ENTITY::GET_ENTITY_COORDS(car, false);
			Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(car, 2);
			//Vector3 forward_vec = ENTITY::GET_ENTITY_FORWARD_VECTOR(car);

			transformMatrix car_pos_rot = createMatrix(position, rotation);
			if (tcp_server->readyCameraChange() && changed == -1) {
				changed = 0;
				inThridPerson = !inThridPerson;
			}
			
			if (inThridPerson) {
				Vector3 offset = Vector3();
				offset.x = 0.0f;
				offset.y = -5.5f;
				offset.z = 3.5f;

				Vector3 offset_rot = Vector3();
				offset_rot.x = 0.0f;
				offset_rot.y = 0.0f;
				offset_rot.z = 0.0f;

				CAM::ATTACH_CAM_TO_ENTITY(customCam, car, offset.x, offset.y, offset.z, true);

				/*transformMatrix offset_matrix = createMatrix(offset, offset_rot);

				transformMatrix absoluteMatrix = transformTransform(car_pos_rot, offset_matrix);

				Vector3 absolutePosition = getPosition(absoluteMatrix);
				Vector3 absoluteRotation = getRotation(absoluteMatrix);

				CAM::SET_CAM_COORD(customCam, absolutePosition.x, absolutePosition.y, absolutePosition.z);*/
				CAM::SET_CAM_ROT(customCam, rotation.x, rotation.y, rotation.z, 2);
			}
			else {
				Vector3 offset = Vector3();

				offset.x = 0.0f;
				offset.y = 1.5f;
				offset.z = 0.3f;

				CAM::ATTACH_CAM_TO_ENTITY(customCam, car, offset.x, offset.y, offset.z, true);

				/*
				offset = transformVector(car_pos_rot, offset);

				CAM::SET_CAM_COORD(customCam, offset.x, offset.y, offset.z);*/
				CAM::SET_CAM_ROT(customCam, rotation.x, rotation.y, rotation.z, 2);
			}
			WAIT(0);
			if (changed == 1) {
				tcp_server->camera_changed(inThridPerson);
				changed = -1;
			}
			else if (changed > -1)
			{
				changed += 1;
			}
		}
	}
	else
	{
		reset_car();
	}

	if (!camera_init) {
		camera_init = true;
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 position = ENTITY::GET_ENTITY_COORDS(playerPed, false);
		Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(playerPed, 2);

		customCam = CAM::CREATE_CAM_WITH_PARAMS((char*)"DEFAULT_SCRIPTED_CAMERA", position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, 77.5, true, 2);
		CAM::SET_CAM_ACTIVE(customCam, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);
		CAM::SET_FOLLOW_VEHICLE_CAM_VIEW_MODE(2);

		tcp_server->camera_changed(false);
	}

	int cmd = tcp_server->getCommand();
	/*char text[256];
	sprintf_s(text, "Angle: %f Throttle %f Command %d", tcp_server->getSteeringAngle(), tcp_server->getThrottle(), cmd);
	UI::SET_TEXT_SCALE(0.5, 0.5);
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY((char*)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_TEXT(0.0f, 0.0f);
	// box
	GRAPHICS::DRAW_RECT(0.027f, 0.043f, 0.058f, 0.056f, 75, 75, 75, 75);*/

	if (cmd == 1) {
		reset_car();
	}
	else if (cmd == 2) {
		print_position();
	}
	else if (cmd == 3) {
		tcp_server->get_window_info();
	}
	else if (cmd == 4) {
		reset_car_road();
	}
}

int main()
{
	tcp_server = new TCPManager();

	startingLocation.x = 38.831593f;
	startingLocation.y = -1539.847534f;
	startingLocation.z = 29.342484f;

	reset_car();

	while (script_running_bool)
	{
		update();
		WAIT(0);
	}
	return 0;
}

void ScriptMain()
{
	main();
}

void ScriptExit() 
{
	script_running_bool = false;
	delete tcp_server;
}