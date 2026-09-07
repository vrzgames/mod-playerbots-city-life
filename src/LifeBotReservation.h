/*
 * Shared bot reservations for Playerbots Life modules
 * Copyright (C) 2026 iCore
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LIFE_BOT_RESERVATION_H
#define LIFE_BOT_RESERVATION_H

#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace PlayerbotsLife
{
    enum class ReservationOwner : std::uint8_t
    {
        CityLife,
        PvPLife
    };

    struct ReservationRegistry
    {
        std::mutex Mutex;
        std::unordered_map<std::uint32_t, ReservationOwner> Owners;
    };

    inline ReservationRegistry& GetReservationRegistry()
    {
        static ReservationRegistry registry;
        return registry;
    }

    inline bool IsReserved(std::uint32_t guidLow)
    {
        ReservationRegistry& registry = GetReservationRegistry();
        std::lock_guard<std::mutex> lock(registry.Mutex);
        return registry.Owners.find(guidLow) != registry.Owners.end();
    }

    inline bool TryReserve(std::uint32_t guidLow, ReservationOwner owner)
    {
        ReservationRegistry& registry = GetReservationRegistry();
        std::lock_guard<std::mutex> lock(registry.Mutex);
        auto const [entry, inserted] = registry.Owners.emplace(guidLow, owner);
        return inserted || entry->second == owner;
    }

    inline void Release(std::uint32_t guidLow, ReservationOwner owner)
    {
        ReservationRegistry& registry = GetReservationRegistry();
        std::lock_guard<std::mutex> lock(registry.Mutex);
        auto const entry = registry.Owners.find(guidLow);
        if (entry != registry.Owners.end() && entry->second == owner)
            registry.Owners.erase(entry);
    }
}

#endif
