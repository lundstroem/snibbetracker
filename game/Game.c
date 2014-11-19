//
//  ExampleGame.c
//  iOS_GL_Frontend
//
//  Created by Harry Lundstrom on 22/09/14.
//  Copyright (c) 2014 harrylundstrom. All rights reserved.
//

#include "Game.h"
#include "CEngine.h"
#include "CAnimation.h"
#include "CAnimationFrame.h"
#include "CListQueue.h"
#include "CTimer.h"
#include "CLabel.h"
#include <stdlib.h>

/* This is game logic separated from the engine. Uses convenience functions for default tasks, but can be
 extended as necessary */

struct CEntity *player = NULL;
struct CAnimationFrame *down = NULL;
struct CAnimationFrame *left = NULL;
struct CAnimationFrame *right = NULL;
struct CAnimationFrame *straight_left = NULL;
struct CAnimationFrame *straight_right = NULL;
struct CAnimationFrame *stopping = NULL;

//static struct CAnimationFrame* cEntityPlayerGetFrame(struct CEntity *e, double dt);
static void cEntityPlayerInit(struct CEntity *e);
static void cEntityPlayerUpdateControls(struct CEntity* e, struct CButton **buttons, double dt);

static void initButtons();
static void initLevel();
static void levelContinue();
static void levelCompleted();
static void initGroundCanvas(struct CEngineContext *e);
static void updateGroundCanvas(int speed, unsigned int **raster);
static void gameOver();
static void resetPlayer();

struct CLabel *scrolling_menu_label;

unsigned int **raster_ground = NULL;
int distance_counter = 0;
int flag_color_toggle = 0;
int distance_counter_limit = 100;


int current_level = 0;
int level_retries = 0;

int flag_level_start = 0;
int flag_position = 0;
int flag_position_count = 0;
int flag_position_max = 20;
int level_flag_limit = 6;

int new_level_paused = 0;
int game_over = 0;
int game_over_started = 0;
int menu = 1;


#define TILE_SIZE 16

int flag_positions[20][2] = {
    {30, 100},
    {30, 100},
    {30, 100},
    {30, 100},
    {30, 100},
    {100, 40},
    {100, 40},
    {100, 40},
    {100, 40},
    {100, 40},
    {30, 100},
    {30, 100},
    {30, 100},
    {30, 100},
    {30, 100},
    {100, 100},
    {100, 100},
    {100, 100},
    {100, 100},
    {100, 100}
};

#define cengine_PLAYER 0
#define cengine_FLAG 1

#define cengine_RED_FLAG_X 12
#define cengine_BLUE_FLAG_X 13

/*
 
 done:
 - miss flag/game over, restart.
 - more generous space for side collision.
 - snow tile graphics
 - trace ski's in snow (render to separate screen from ski color/position and scroll up)
 - generate positions for all flags so that it's the same for all players on all devices.
 - decide control scheme: one button/switch or push different side?
 - stats for every level (retry amount, level number)
 - re-implement game over
 - (touch up inside buttons)
 unfinished:
 
 - Label buttons, how to do.
 labels are specified after x/y char pos. Buttons are not.
 buttons are rendered automatically, labels are not.
 - set bgcolor color for normal/pushed. (triggered by touchupinside)
 - active(is this for toggle or for hidden/not not rendered etc)
 - set larger toucharea.
 
 ----------
 - Shorten iteration time (10 flags for example, or random) and a button to continue to the next stage.
 - Make every stage progressively harder.
 - The score (handicap) is based on nr of tries/ current stage, like desert golfing.
 - don't use random, generate static data for the game so that every players experience is the same.
 - make it so that the game is potentially never ending.
 - add more elements like rocks, birds, bears etc after a while to make it harder.
 
 example solution
 requirements:
 - need to be able to get data from nr of tries/stage to detect and tweak difficulty
 - need to be able to tell the game to start from a certain level for debugging.
 - generate randomized static json data for 10000 stages.
 - increase flag difficulty and other obstacles progressively.
 
 - GA tracking. stage_nr_completed with nr_of_tries
 stage_nr_failed
 ----------
 
 - leveldata for if flag is left/right
 - tutorial (show where to go and controls)
 - label (scrolling text)
 - buttons gfx tiles with label
 - tweak collision for when game over is detected
 
 
 
 */

/* Create entities with id's, level etc */
void cEngineInitHook(struct CEngineContext *e) {
    printf("cEngineInitHook\n");
    player = cEngineAddEntity(cengine_PLAYER, 4, 1);
    cEntityPlayerInit(player);
    
    for(int i = 0; i < 20; i++) {
        struct CEntity *obj = cEngineAddEntity(cengine_FLAG, 6, 1);
        obj->current_animation_frame = cAnimationFrameNew();
        obj->current_animation_frame->x = 10;
        obj->current_animation_frame->y = 10;
        obj->gravity_active = 0;
        obj->vec_y = -2;
        obj->active = 0;
    }
    
    scrolling_menu_label = cLabelNew("Hello and welcome to slalomy, a one button game. Press the screen in intervals to turn left and right and get on the correct side of the flags. Have fun! Made by Harry Lundstrom 2014");
    cLabelActivateScrolling(scrolling_menu_label, 144);
    scrolling_menu_label->y = 190;
    
    initButtons();
    initLevel();
    initGroundCanvas(e);
    
}

void updateLevel(double dt) {
    
    if(game_over == 1) {
        return;
    }
    
    distance_counter++;
    
    struct CListQueue* entity_list = cEngineGetEntityList();
    
    //printf("distance_counter:%i distance_counter_limit:%i \n", distance_counter, distance_counter_limit);
    if(distance_counter == distance_counter_limit) {
        
        printf("flag_position_count:%i level_flag_limit:%i \n", flag_position_count, level_flag_limit);
        
        //detect end of level and set paused.
        if(flag_position_count == level_flag_limit) {
            printf("setting level completed");
            if(new_level_paused == 0) {
                levelCompleted();
                new_level_paused = 1;
            }
            return;
        }
        
        if(flag_position_count+1 == level_flag_limit) {
            printf("last flag");
            //last flag.
            distance_counter_limit = 200;
            distance_counter = 0;
            flag_position_count++;
            return;
        }
        
        //spawn
        struct CLinearNode *node = entity_list->first;
        while(node != NULL) {
            struct CEntity *e = node->data;
            if(e->active == 0 && e->type == cengine_FLAG && flag_position < flag_position_max
               && flag_position_count+1 < level_flag_limit) {
                e->active = 1;
                e->x = flag_positions[flag_position][0];
                e->y = 256;
                
                distance_counter_limit = flag_positions[flag_position][1];
                distance_counter = 0;
                
                flag_position++;
                flag_position_count++;
                
                if(flag_color_toggle == 0) {
                    cAnimationFrameUpdate(e->current_animation_frame, 12, 1);
                    flag_color_toggle = 1;
                } else {
                    cAnimationFrameUpdate(e->current_animation_frame, 13, 1);
                    flag_color_toggle = 0;
                }
                
                break;
            }
            
            node = node->next;
        }
        
        if(flag_position >= flag_position_max) {
            printf("flag position is equal or larger than max.");
        }
    }
    
    //deactivate flags that have gone out of screen and if player is on wrong side of flag (game over).
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        if(e->active == 1 && e->y < -(TILE_SIZE*2) && e->type == cengine_FLAG) {
            e->active = 0;
            //cEngineLog("recycled");
        }
        
        int run_game_over = 0;
        if(e->active && e->current_animation_frame->x == cengine_BLUE_FLAG_X
           && e->y < player->y && e->y > player->y-TILE_SIZE
           && e->x < player->x) {
            run_game_over = 1;
        } else if(e->active && e->current_animation_frame->x == cengine_RED_FLAG_X
                  && e->y < player->y && e->y > player->y-TILE_SIZE
                  && e->x > player->x) {
            run_game_over = 1;
        }
        
        if(run_game_over == 1) {
            if(game_over_started == 0) {
                game_over_started = 1;
                gameOver();
            }
        }
        
        node = node->next;
    }
}

int white_color = 0xEEEEEEFF;
int gray_color = 0xE3E3E3FF;
int ground_width = 0;
int ground_height = 0;

void initGroundCanvas(struct CEngineContext *e) {
    ground_width = e->width;
    ground_height = e->height;
    
    raster_ground = malloc(ground_width * sizeof(unsigned int *));
    if(raster_ground == NULL) {
        fprintf(stderr, "out of memory\n");
    }
    for(int i = 0; i < ground_width; i++) {
        if(raster_ground != NULL) {
            raster_ground[i] = malloc(ground_height * sizeof(unsigned int));
            if(raster_ground[i] == NULL) {
                fprintf(stderr, "out of memory\n");
            }
        }
    }
    
    for (int s_x = 0; s_x < ground_width; s_x++) {
        for (int s_y = 0; s_y < ground_height; s_y++) {
            if(raster_ground != NULL && raster_ground[s_x] != NULL) {
                raster_ground[s_x][s_y] = white_color;
            }
        }
    }
}


int isInBounds(int x, int y, int width, int height) {
    if(x > -1 && y > -1 && x < width && y < height) {
        return 1;
    } else {
        return 0;
    }
}
void updateGroundCanvas(int speed, unsigned int**raster) {
    for (int s_x = 0; s_x < ground_width; s_x++) {
        for (int s_y = 0; s_y < ground_height; s_y++) {
            if(s_y-speed < ground_height && s_y-speed > -1) {
                raster_ground[s_x][s_y-speed] = raster_ground[s_x][s_y];
            }
            
            if(s_y == ground_height-1 || s_y == ground_height-2) {
                raster_ground[s_x][s_y] = white_color;
                if(rand()%2 == 0) {
                    raster_ground[s_x][s_y] = gray_color;
                }
            }
            raster[s_x][s_y] = raster_ground[s_x][s_y];
        }
    }
    
    int trace_color = 0xF1F1F1FF;
    
    if(player->current_animation_frame == down) {
        
        int p_x = (int)player->x+6;
        int p_y = (int)player->y;
        if(isInBounds(p_x, p_y, ground_width, ground_height)) {
            raster_ground[p_x][p_y] = trace_color;
        }
        
        if(isInBounds(p_x, p_y-1, ground_width, ground_height)) {
            raster_ground[p_x][p_y-1] = trace_color;
        }
        
        if(isInBounds(p_x+3, p_y, ground_width, ground_height)) {
            raster_ground[p_x+3][p_y] = trace_color;
        }
        
        if(isInBounds(p_x+3, p_y-1, ground_width, ground_height)) {
            raster_ground[p_x+3][p_y-1] = trace_color;
        }
    }
    
    if(player->current_animation_frame == left) {
        
        int p_x = (int)player->x+10;
        int p_y = (int)player->y+3;
        if(isInBounds(p_x, p_y, ground_width, ground_height)) {
            raster_ground[p_x][p_y] = trace_color;
        }
        
        if(isInBounds(p_x, p_y-1, ground_width, ground_height)) {
            raster_ground[p_x][p_y-1] = trace_color;
        }
        
        if(isInBounds(p_x+4, p_y+1, ground_width, ground_height)) {
            raster_ground[p_x+4][p_y+1] = trace_color;
        }
        
        if(isInBounds(p_x+4, p_y, ground_width, ground_height)) {
            raster_ground[p_x+4][p_y] = trace_color;
        }
    }
    
    if(player->current_animation_frame == right) {
        
        int p_x = (int)player->x+1;
        int p_y = (int)player->y+3;
        if(isInBounds(p_x, p_y, ground_width, ground_height)) {
            raster_ground[p_x][p_y] = trace_color;
        }
        
        if(isInBounds(p_x, p_y+1, ground_width, ground_height)) {
            raster_ground[p_x][p_y+1] = trace_color;
        }
        
        if(isInBounds(p_x+4, p_y, ground_width, ground_height)) {
            raster_ground[p_x+4][p_y] = trace_color;
        }
        
        if(isInBounds(p_x+4, p_y-1, ground_width, ground_height)) {
            raster_ground[p_x+4][p_y-1] = trace_color;
        }
    }
    
}

void initLevel() {
    // init level
    struct CAnimation ***level = cEngineGetLevel();
    for (int s_x = 0; s_x < 64; s_x++) {
        for (int s_y = 0; s_y < 64; s_y++) {
            struct CAnimation *animation = cAnimationNew(1, 0, 0);
            animation->default_frame.x = 3;
            animation->default_frame.y = 0;
            level[s_x][s_y] = animation;
        }
    }
}


void gameOver() {
    distance_counter_limit = 200;
    distance_counter = 0;
    cEngineGetButton(1)->active = 1;
    game_over = 1;
    
    struct CListQueue* entity_list = cEngineGetEntityList();
    // stop tiles.
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        if(e->type == cengine_FLAG && e->active == 1) {
            // e->active = 0;
            e->vec_y = 0;
        }
        node = node->next;
    }
}

void levelCompleted() {
    distance_counter_limit = 200;
    distance_counter = 0;
    cEngineGetButton(2)->active = 1;
}

void levelContinue(int retry) {
    
    resetPlayer();
    
    current_level++;
    cEngineGetButton(2)->active = 0;
    
    if(retry == 1) {
        flag_position = flag_level_start;
        game_over_started = 0;
        game_over = 0;
        cEngineGetButton(1)->active = 0;
        level_retries++;
    } else {
        level_retries = 0;
        flag_level_start = flag_position;
    }
    
    flag_position_count = 0;
    new_level_paused = 0;
    distance_counter = 0;
    
    printf("starting at flag_position:%i \n", flag_position);
    printf("flag_position_count:%i level_flag_limit:%i \n", flag_position_count, level_flag_limit);
    
    
    struct CListQueue* entity_list = cEngineGetEntityList();
    // restart flags.
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        if(e->type == cengine_FLAG) {
            e->active = 0;
            e->vec_y = -2;
        }
        node = node->next;
    }
}


int control_player_toggle = 0;
int control_player_button_toggle_set = 0;


void initButtons() {
    
    // init button
    cEngineGetButton(0)->x = 0;
    cEngineGetButton(0)->y = 0;
    cEngineGetButton(0)->width = 0;
    cEngineGetButton(0)->height = 0;
    cEngineGetButton(0)->whole_screen = 1;
    cEngineGetButton(0)->active = 1;
    
    struct CButton *b2 = cEngineGetButton(1);
    b2->x = 20;
    b2->y = 70;
    //b2->active = 1;
    b2->color = 0xFF0000FF;
    b2->color_pressed = 0x00FF00FF;
    b2->color_bg = 0x00FF00FF;
    b2->color_bg_pressed = 0xFF0000FF;
    struct CLabel *label = cLabelNew("restart");
    cButtonSetLabel(b2, label);
    
    b2 = cEngineGetButton(2);
    b2->x = 20;
    b2->y = 60;
    //b2->active = 1;
    b2->color = 0xFF0000FF;
    b2->color_pressed = 0x00FF00FF;
    b2->color_bg = 0x00FF00FF;
    b2->color_bg_pressed = 0xFF0000FF;
    label = cLabelNew("continue");
    cButtonSetLabel(b2, label);
    
    //menu options------------------
    
    b2 = cEngineGetButton(3);
    b2->x = 20;
    b2->y = 100;
    b2->active = 1;
    b2->color = 0xFF0000FF;
    b2->color_pressed = 0x00FF00FF;
    b2->color_bg = 0x00FF00FF;
    b2->color_bg_pressed = 0xFF0000FF;
    label = cLabelNew("start game");
    cButtonSetLabel(b2, label);
    
    b2 = cEngineGetButton(4);
    b2->x = 20;
    b2->y = 120;
    b2->active = 1;
    b2->color = 0xFF0000FF;
    b2->color_pressed = 0x00FF00FF;
    b2->color_bg = 0x00FF00FF;
    b2->color_bg_pressed = 0xFF0000FF;
    label = cLabelNew("music off");
    cButtonSetLabel(b2, label);
    
    b2 = cEngineGetButton(5);
    b2->x = 20;
    b2->y = 140;
    b2->active = 1;
    b2->color = 0xFF0000FF;
    b2->color_pressed = 0x00FF00FF;
    b2->color_bg = 0x00FF00FF;
    b2->color_bg_pressed = 0xFF0000FF;
    label = cLabelNew("sfx off");
    cButtonSetLabel(b2, label);
}

void cEnginePreUpdateHook(struct CButton **buttons, double dt) {
    //player->current_animation_frame = cEntityPlayerGetFrame(player, dt);
    cEntityPlayerUpdateControls(player, buttons, dt);
}


void cEngineUpdateHook(double dt) {
    
    /* center camera on specific entity or coordinate */
    /* handle animations */
    /* handle input */
    
    if(menu == 0) {
        updateLevel(dt);
    }
}

void cEnginePreEntityRenderHook(double dt, unsigned int **raster) {
    if(menu == 0) {
        if(game_over == 1) {
            updateGroundCanvas(0, raster);
        } else {
            updateGroundCanvas(2, raster);
        }
    }
}

void cEngineRenderHook(double dt, unsigned int **raster) {
    /* custom rendering tasks */
    //cEngineRenderLabel(raster, "abcdefghijklmnopqrstuvwxyz 0123456789.,:-'!\"#?", 0, 0, 0x00FF00FF, 0x00FFFFFF);
    
    //if(game_over == 1) {
    //    cEngineRenderLabel(raster, "game over", 0, 1, 0x00FF00FF, 0x00FFFFFF);
    //}
    if(new_level_paused == 1) {
        char t[1000];
        sprintf(t, "current level:%d", current_level);
        cEngineRenderLabelWithParams(raster, t, 0, 12, 0x00FF00FF, 0x00FFFFFF);
        sprintf(t, "retries:%d", level_retries);
        cEngineRenderLabelWithParams(raster, t, 0, 13, 0x00FF00FF, 0x00FFFFFF);
    }
    
    if(menu == 1) {
        for (int s_x = 0; s_x < 9; s_x++) {
            for (int s_y = 0; s_y < 16; s_y++) {
                cEngineRenderSprite(raster, s_x*TILE_SIZE, s_y*TILE_SIZE, s_x, s_y+2, 0, TILE_SIZE, TILE_SIZE, 0, -1, -1);
            }
        }
        
        cEngineRenderLabel(raster, scrolling_menu_label, 0xFF000000, 0xFEFEFEFF);
    }
}

void cEngineEntityCollisionHook(struct CEntity *a, struct CEntity *b) {
    /* collision response */
    //printf("collision");
}

void cEngineEnvironmentCollisionHook() {
    
}

void cEngineCleanupHook() {
    
    free(down);
    down = NULL;
    free(left);
    left = NULL;
    free(right);
    right = NULL;
    free(straight_left);
    straight_left = NULL;
    free(straight_right);
    straight_right = NULL;
    free(stopping);
    stopping = NULL;
}


void resetPlayer() {
    player->x = TILE_SIZE*4;
    player->y = TILE_SIZE*1;
    player->speed = 0;
}

void cEntityPlayerInit(struct CEntity *e)
{
    e->speed = 0;
    
    down = cAnimationFrameNew();
    down->x = 4;
    down->y = 1;
    
    left = cAnimationFrameNew();
    left->x = 5;
    left->y = 1;
    
    right = cAnimationFrameNew();
    right->x = 6;
    right->y = 1;
    
    straight_left = cAnimationFrameNew();
    straight_left->x = 7;
    straight_left->y = 1;
    
    straight_right = cAnimationFrameNew();
    straight_right->x = 8;
    straight_right->y = 1;
    
    stopping = cAnimationFrameNew();
    stopping->x = 14;
    stopping->y = 1;
    
    e->current_animation_frame = down;
}


void cEntityPlayerUpdateControls(struct CEntity* e, struct CButton **buttons, double dt)
{
    if(menu == 1) {
        
        // start game
        if(buttons[3]->touched_up_inside) {
            
        }
        // music toggle
        if(buttons[4]->touched_up_inside) {
            
        }
        // sfx toggle
        if(buttons[5]->touched_up_inside) {
            
        }
        
        
    } else if(menu == 0) {
        double player_speed_inc = 0.05;
        double player_friction_inc = 0.02;
        double speed_limit = 1.3;
        double speed_limit_down = 0.3;
        
        if(buttons[0]->pressed && control_player_toggle == 0) {
            control_player_button_toggle_set = 0;
            
            e->turn_left = 1;
            e->turn_right = 0;
            e->moving = 1;
            e->speed -= player_speed_inc;
            e->current_animation_frame = left;
            
            // less friction when pushing
            player_friction_inc = 0.001;
            
        }
        
        if(buttons[0]->pressed && control_player_toggle == 1) {
            control_player_button_toggle_set = 0;
            
            e->turn_right = 1;
            e->turn_left = 0;
            e->moving = 1;
            e->speed += player_speed_inc;
            e->current_animation_frame = right;
            
            // less friction when pushing
            player_friction_inc = 0.001;
        }
        
        
        if(buttons[0]->pressed == 0 && control_player_button_toggle_set == 0) {
            control_player_button_toggle_set = 1;
            if(control_player_toggle == 0) {
                control_player_toggle = 1;
            } else {
                control_player_toggle = 0;
            }
        }
        
        
        // on game over
        if(buttons[1]->touched_up_inside) {
            levelContinue(1);
        }
        
        // next stage
        if(buttons[2]->touched_up_inside) {
            levelContinue(0);
        }
        
        
        
        // limit speed
        if(e->speed > speed_limit && control_player_toggle == 1) {
            e->speed = speed_limit;
        } else if(e->speed < -speed_limit && control_player_toggle == 0) {
            e->speed = -speed_limit;
        }
        
        // friction
        if(e->speed > 0) {
            e->speed -= player_friction_inc;
        } else if(e->speed < 0) {
            e->speed += player_friction_inc;
        }
        
        // show down animation if slow speed
        if(buttons[0]->pressed == 0) {
            if(e->speed < speed_limit_down && control_player_toggle == 0) {
                //printf("control 0: speed: %f speed_limit_down:%f\n", e->speed, speed_limit_down);
                e->current_animation_frame = down;
            } else if(e->speed > -speed_limit_down && control_player_toggle == 1) {
                //printf("control 1: speed: %f speed_limit_down:%f\n", e->speed, speed_limit_down);
                e->current_animation_frame = down;
            }
        }
        
        e->xa = e->speed;
    }
}





