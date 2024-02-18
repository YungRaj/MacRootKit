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

#include "SEP.hpp"

namespace SEP {

    void initialize() {
        mrk::MacRootKit* rootkit = mac_rootkit_get_rootkit();

        plugin = new mrk::Plugin("SEP", 1, 0, NULL, 0, NULL, 0, NULL, 0);

        appleA7IOP = rootkit->getKextByIdentifier("com.apple.driver.AppleA7IOP");
        appleSEPManager = rootkit->getKextByIdentifier("com.apple.driver.AppleSEPManager");
        appleKeyStore = rootkit->getKextByIdentifier("com.apple.driver.AppleSEPKeyStore");
        appleCredentialManager =
            rootkit->getKextByIdentifier("com.apple.driver.AppleCredentialManager");

        plugin->addTarget(appleA7IOP);
        plugin->addTarget(appleSEPManager);
        plugin->addTarget(appleKeyStore);
        plugin->addTarget(appleCredentialManager);
    }

    void installAppleA7IOPHooks() {}

    void installAppleSEPManagerHooks() {}

    void installAppleKeyStoreHooks() {}

    void installAppleCredentialManagerHooks() {}

}; // namespace SEP