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

#ifndef GAME_H
#define GAME_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    GAME_EVT_TICK,
    GAME_EVT_BTN_PLAYER1,
    GAME_EVT_BTN_PLAYER2
} game_evt_type_t;

typedef struct {
    game_evt_type_t type;
} game_evt_t;

typedef struct {
    int32_t paddle_start;
    int32_t paddle_end;
} paddle_pos_t;

typedef struct {
    int32_t min_boundrary;
    int32_t max_boundrary;
    paddle_pos_t p1_paddle;
    paddle_pos_t p2_paddle;
    int32_t ball_pos_after_score;

    int32_t p1_score;
    int32_t p2_score;
    int32_t ball_pos;
    int ball_direction;
} game_state_t;

extern struct k_msgq game_evt_q;

extern struct k_timer game_tick_timer;

void advance_game(game_state_t* game_state);

#endif
