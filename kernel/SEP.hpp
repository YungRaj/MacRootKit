/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Types.h>

#include "vector.hpp"

#include "MacRootKit.hpp"

namespace mrk {
    class MacRootKit;
};

extern mrk::MacRootKit* mac_rootkit_get_rootkit();

namespace SEP {
    static mrk::Plugin* plugin;

    static Kext* appleA7IOP;
    static Kext* appleSEPManager;
    static Kext* appleKeyStore;
    static Kext* appleCredentialManager;

    void initialize();

    void installAppleA7IOPHooks();
    void installAppleSEPManagerHooks();
    void installAppleKeyStoreHooks();
    void installAppleCredentialManagerHooks();
}; // namespace SEP