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

#include "render.h"
#include "game.h"
#include <zephyr/kernel.h>
#include <stdint.h>

K_EVENT_DEFINE(render_group_event);

K_MUTEX_DEFINE(render_state_lock);

game_state_t render_state;

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
const struct device *const led_strip_dev = DEVICE_DT_GET(DT_ALIAS(led_strip));

static void render_thread_entrypoint(void*, void*, void*)
{
	struct led_rgb pixels[STRIP_NUM_PIXELS];
    memset(pixels, 0, sizeof(pixels));

	while (1) {
        uint32_t evt = k_event_wait_safe(&render_group_event, RENDER_EVT_ANY, true, K_FOREVER);
        k_mutex_lock(&render_state_lock, K_FOREVER);
        game_state_t local_render_state = render_state;
        k_mutex_unlock(&render_state_lock);
        
        if (evt & RENDER_EVT_ANY) {
            memset(pixels, 0, sizeof(pixels));
            draw(&local_render_state, pixels, STRIP_NUM_PIXELS);
            led_strip_update_rgb(led_strip_dev, pixels, STRIP_NUM_PIXELS);
        }
	}
}

void draw(game_state_t* render_state, struct led_rgb* pixels, int num_pixels)
{
    const struct led_rgb paddle_color = RGB(75, 65, 20); 
    const struct led_rgb ball_color = RGB(180, 180, 180);
    for (int i = render_state->p1_paddle.paddle_start; i <= render_state->p1_paddle.paddle_end; i++) {
        pixels[i] = paddle_color;
    }
    for (int i = render_state->p2_paddle.paddle_start; i <= render_state->p2_paddle.paddle_end; i++) {
        pixels[i] = paddle_color;
    }
    pixels[render_state->ball_pos] = ball_color;
}

K_THREAD_DEFINE(render_thread_id,
                2048,
                render_thread_entrypoint,
                NULL, NULL, NULL,
                4,
                0,
                0);
