#include "player_manager.h"

#include "../mcc/mcc_manager.h"

using namespace libmcc;

#include <cstring>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/xchar.h>

int c_player_manager::initialize() {
	// Initialize XUID
	initialize_xuid();

	// Initialize profiles
	m_profiles = new s_player_profiles[k_local_player_count];

	if (load_profile()) {
		initialize_profile();
	}

	return 0;
}

int c_player_manager::shutdown() {
	save_profile();

	delete[] m_profiles;

	return 0;
}

int c_player_manager::initialize_xuid() {
	union {
		GUID guid;
		XUID xuid;
		struct { uint64_t part1, part2; };
	} id;

	auto hr = CoCreateGuid(&id.guid);

	ASSERT_HR(hr, "Failed to create guid!");

	id.xuid = id.part1 ^ id.part2;

	for (int i = 0; i < k_local_player_count; i++) {
		m_xuids[i] = id.xuid + i;
		m_input_devices[i] = _player_input_device_km;
	}

	m_local_player_count = 0;

	return 0;
}

int c_player_manager::find_slot(e_player_input_device device) {
	for (int i = 0; i < m_local_player_count; i++) {
		if (m_input_devices[i] == device) {
			return i;
		}
	}

	return -1;
}

int c_player_manager::join(e_player_input_device device) {
	c_critical_section cs(_critical_section_player);

	if (find_slot(device) != k_player_input_device_none) {
		return -1;
	}

	if (device == _player_input_device_km) {
		if (m_local_player_count != 0) {
			return -1;
		}

		m_input_devices[0] = _player_input_device_km;
		m_local_player_count = 1;

		return 0;
	}

	if (m_local_player_count >= libmcc::k_local_player_count) {
		return -1;
	}

	int slot = m_local_player_count;
	m_input_devices[slot] = device;
	m_local_player_count++;

	return slot;
}

void c_player_manager::leave(int slot) {
	c_critical_section cs(_critical_section_player);

	if (slot < 0 || slot >= m_local_player_count) {
		return;
	}

	for (int i = slot; i < m_local_player_count - 1; i++) {
		m_input_devices[i] = m_input_devices[i + 1];
	}

	m_local_player_count--;

	if (m_local_player_count == 0) {
		m_input_devices[0] = _player_input_device_km;
	}
}

int c_player_manager::initialize_profile() {
	memset(m_profiles, 0, k_player_profiles_size);

	for (int i = 0; i < k_local_player_count; i++) {
		auto profile = m_profiles + i;

		auto name = fmt::format(L"Player {}", i + 1);

		auto tag = fmt::format(L"P{}", i + 1);

		memcpy(profile->name, name.c_str(), name.size() * sizeof(wchar_t));

		for (int j = 0; j < k_game_count; j++) {
			auto game_profile = profile->game_profile + j;

			memcpy(game_profile->profile.service_tag, tag.c_str(), tag.size() * sizeof(wchar_t));
		}
	}

	return 0;
}

int c_player_manager::load_profile() {
	auto profile_file = mcc_manager()->read_resource(PROFILE_FILE_NAME);

	if (profile_file == nullptr || profile_file->size() != k_player_profiles_size) {
		return -1;
	}

	memcpy(m_profiles, profile_file->data(), k_player_profiles_size);

	return 0;
}

int c_player_manager::save_profile() {
	return mcc_manager()->write_resource(
		PROFILE_FILE_NAME,
		reinterpret_cast<const char*>(m_profiles),
		k_player_profiles_size);
}
