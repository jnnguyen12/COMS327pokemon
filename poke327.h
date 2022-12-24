#ifndef POKE327_H
#define POKE327_H
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <typeinfo>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "heap.h"
#include "parser.h"

using namespace std;

#define malloc(size) ({          \
  void *_tmp;                    \
  assert((_tmp = malloc(size))); \
  _tmp;                          \
})

class path
{
public:
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
};

typedef enum dir
{
  null,
  north,
  east,
  south,
  west
} dir_t;

typedef enum dim
{
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

#define MAP_X 80
#define MAP_Y 21
#define MIN_TREES 10
#define MIN_BOULDERS 10
#define TREE_PROB 95
#define BOULDER_PROB 95
#define WORLD_SIZE 401
#define MIN_TRAINERS 5
#define ADD_TRAINER_PROB 50

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])
#define ter_cost(x, y, c) move_cost[c][m->map[y][x]]
#define str(x) (const_cast<char *>(x.c_str()))
#define ic(x) (static_cast<int>(x))
#define strint(x) (const_cast<char *>((to_string(x)).c_str()))
  

typedef enum __attribute__((__packed__)) terrain_type
{
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_exit,
  num_terrain_types
} terrain_type_t;

typedef enum __attribute__((__packed__)) movement_type
{
  move_hiker,
  move_rival,
  move_pace,
  move_wander,
  move_sentry,
  move_explore,
  move_pc,
  num_movement_types
} movement_type_t;

typedef enum __attribute__((__packed__)) character_type
{
  char_pc,
  char_hiker,
  char_rival,
  char_other,
  num_character_types
} character_type_t;

string character_name[num_movement_types] = {
  "Hiker",
  "Rival",
  "Pacer",
  "Wanderer",
  "Sentry",
  "Explorer",
  "PC",
};

int32_t move_cost[num_character_types][num_terrain_types] = {
    {INT_MAX, INT_MAX, 10, 10, 10, 20, 10, INT_MAX, INT_MAX, 10},
    {INT_MAX, INT_MAX, 10, 50, 50, 15, 10, 15, 15, INT_MAX},
    {INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX},
    {INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX},
};

class mon_move 
{
  public:
    int id;
    string iden;
    int type;
    int power;
    int accuracy;
    int priority;
    int level;
};

class mon 
{
  public:
    mon() {
      lv = 0;
    }
    int id;
    string iden;
    int type[2];
    mon_move *moves[2];
    vector<mon_move> all_moves;
    int hp;
    int lv;
    int base_stats[6];
    int stats[6];
    string gender;
    bool shiny;
};

class inv
{
 public:
  int balls;
  int potions;
  int revives;
};

class character
{
public:
  pair_t dir;
  pair_t pos;
  char symbol;
  int next_turn;
  vector<mon> pkms;
};

class pc : public character
{
  public:
  inv *i;
  int money;
};

class npc : public character
{
public:
  character_type_t ctype;
  movement_type_t mtype;
  uint8_t fought; // 0 or 1
};

class map
{
public:
  terrain_type_t map[MAP_Y][MAP_X];
  uint8_t height[MAP_Y][MAP_X];
  character *cmap[MAP_Y][MAP_X];
  heap_t turn;
  int8_t n, s, e, w;
};

typedef struct queue_node
{
  int x, y;
  struct queue_node *next;
} queue_node_t;

class world_t
{
public:
  map *w[WORLD_SIZE][WORLD_SIZE];
  pair_t cur_idx; // current map's x and y's position
  map *cur_map;   // current map
  /* Please distance maps in world, not map, since *
   * we only need one pair at any given time.      */
  int hiker_dist[MAP_Y][MAP_X]; // dijkstra map for hiker
  int rival_dist[MAP_Y][MAP_X]; // dijkstra map for rival
  pc pc_t;                      // the pc's location
};



/* Even unallocated, a WORLD_SIZE x WORLD_SIZE array of pointers is a very *
 * large thing to put on the stack.  To avoid that, world is a global.     */
extern world_t world;
extern WINDOW *sub;
extern WINDOW *name;

extern pair_t all_dirs[9];

#define rand_dir(dir)         \
  {                           \
    int _i = rand() & 0x7;    \
    dir[0] = all_dirs[_i][0]; \
    dir[1] = all_dirs[_i][1]; \
  }

#define halt()  \
{  \
  noecho();\
  char ch; \
  do { \
    ch = getch(); \
  } while (ch != '\n' && ch != 27); \
} 

#define make_box()  \
{ \
  wclear(sub);\
  box(sub, 0, 0);\
}

void print_party_list(character *c);
void print_map();
void pokemart();
void print_battle(mon *cur, npc* t, mon* m);
void print_fight_screen();
void enc_screen(mon *m);
void pokecenter();
void print_char_list();
void print_hiker_dist();
void print_rival_dist();
void print_inventory();

void move_hiker_func(character *c, pair_t dest);
void move_rival_func(character *c, pair_t dest);
static void move_pacer_func(character *c, pair_t dest);
static void move_wanderer_func(character *c, pair_t dest);
static void move_sentry_func(character *c, pair_t dest);
static void move_explorer_func(character *c, pair_t dest);
static void move_pc_func(character *c, pair_t dest);
static int32_t path_cmp(const void *key, const void *with);
static int32_t edge_penalty(int8_t x, int8_t y);
static void dijkstra_path(map *m, pair_t from, pair_t to);
static int build_paths(map *m);
static int smooth_height(map *m);
static void find_building_location(map *m, pair_t p);
static int32_t hiker_cmp(const void *key, const void *with);
static int32_t rival_cmp(const void *key, const void *with);
void pathfind(map *m);

#endif 