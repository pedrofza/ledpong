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

#include "game.h"
#include "render.h"
#include <zephyr/input/input.h>

#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#define PADDLE_SIZE (8)
#define GAME_TIMER_DELAY_MS (15)

K_MSGQ_DEFINE(game_evt_q, sizeof(game_evt_t), 20, 1);

static void do_game2render_post(game_state_t* game_state, game_state_t* render_state, struct k_mutex* render_state_lock, struct k_event* render_group_event, uint32_t event)
{
	k_mutex_lock(render_state_lock, K_FOREVER);
	memcpy(render_state, game_state, sizeof(*game_state));
	k_mutex_unlock(render_state_lock);
	k_event_post(render_group_event, event);
}

void advance_game(game_state_t* game_state) {
	int32_t candidate_ball_pos = game_state->ball_pos + (game_state->ball_direction < 0 ? -1 : 1);
	
	if (candidate_ball_pos < game_state->min_boundrary) {
		game_state->p2_score++;
		game_state->ball_pos = game_state->ball_pos_after_score;
		game_state->ball_direction = 1;
	} else if (candidate_ball_pos > game_state->max_boundrary) {
		game_state->p1_score++;
		game_state->ball_pos = game_state->ball_pos_after_score;
		game_state->ball_direction = -1;
	} else {
		game_state->ball_pos = candidate_ball_pos;
	}
	do_game2render_post(game_state, &render_state, &render_state_lock, &render_group_event, RENDER_EVT_ANY);
}

bool is_ball_in_paddle(paddle_pos_t paddle, int32_t ball)
{
	return (paddle.paddle_start <= ball) && (ball <= paddle.paddle_end);
}

static void game_tick_cb(struct k_timer *timer_id)
{
	game_evt_t game_evt = {.type = GAME_EVT_TICK};
	k_msgq_put(&game_evt_q, &game_evt, K_NO_WAIT);
}
K_TIMER_DEFINE(game_tick_timer, game_tick_cb, NULL);

static void game_thread_entrypoint(void*, void*, void*)
{
	game_state_t game_state;
	game_state.ball_pos = STRIP_NUM_PIXELS/2;
	game_state.ball_pos_after_score = STRIP_NUM_PIXELS/2;
	game_state.ball_direction = 1;
	game_state.min_boundrary = 0;
	game_state.max_boundrary = STRIP_NUM_PIXELS - 1;
	game_state.p1_score = 0;
	game_state.p2_score = 0;
	int32_t paddle_size = 8;
	game_state.p1_paddle.paddle_start = 0;
	game_state.p1_paddle.paddle_end = paddle_size;
	game_state.p2_paddle.paddle_start = (STRIP_NUM_PIXELS - 1) - paddle_size;
	game_state.p2_paddle.paddle_end = STRIP_NUM_PIXELS - 1;

	k_timer_start(&game_tick_timer, K_MSEC(GAME_TIMER_DELAY_MS), K_MSEC(GAME_TIMER_DELAY_MS));
	while(1) {
		game_evt_t evt;
		k_msgq_get(&game_evt_q, &evt, K_FOREVER);
		switch(evt.type) {
			case GAME_EVT_BTN_PLAYER1: {
				if (is_ball_in_paddle(game_state.p1_paddle, game_state.ball_pos)) {
					game_state.ball_direction = 1;
				}
			}
			break;
			case GAME_EVT_BTN_PLAYER2: {
				if (is_ball_in_paddle(game_state.p2_paddle, game_state.ball_pos)) {
					game_state.ball_direction = -1;
				}
			}
			break;
			case GAME_EVT_TICK:
				advance_game(&game_state);
			break;
			default:
				break;
		}
	}
}

K_THREAD_DEFINE(game_thread_id,
                4096,
                game_thread_entrypoint,
                NULL, NULL, NULL,
                2,
                0,
                0);


static void paddle_btns_cb(struct input_event* evt, void *user_data)
{
	if (evt->sync == 0) {
		return;
	}
	game_evt_type_t evt_type;
	switch (evt->code) {
		case INPUT_KEY_0:
			if (!evt->value) {
				return;
			}
            evt_type = GAME_EVT_BTN_PLAYER1;
		break;
		case INPUT_KEY_1:
			if (!evt->value) {
				return;
			}
			evt_type = GAME_EVT_BTN_PLAYER2;
		break;
		default:
			return;
		break;
	}
	game_evt_t game_evt = { .type = evt_type};
	k_msgq_put(&game_evt_q, &game_evt, K_NO_WAIT);
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(player_keys)), paddle_btns_cb, NULL);
