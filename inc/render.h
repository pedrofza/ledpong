/*
 * Copyright 2026 Pedro Zawadniak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>

extern struct k_event render_group_event;

#define RENDER_EVT_ANY 1

extern struct k_mutex render_state_lock;
extern game_state_t render_state;

void draw(game_state_t* render_state, struct led_rgb* pixels, int num_pixels);

#endif
