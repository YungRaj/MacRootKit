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

#include "Task.hpp"

namespace dyld {
class Library {
public:
    Library(xnu::Task* task, dyld::Dyld* dyld, struct dyld_image_info* image_info)
        : task(task), dyld(dyld), image_info(image_info) {}

    ~Library() {}

    dyld::Dyld* getDyld() {
        return dyld;
    }

    xnu::Task* getTask() {
        return task;
    }

private:
    xnu::Task* task;

    dyld::Dyld* dyld;

    struct dyld_image_info* image_info;
};
} // namespace dyld
