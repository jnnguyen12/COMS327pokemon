#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <curses.h>
#include <iostream>
#include <fstream>
#include <iomanip>

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
#define MIN_TRAINERS 7
#define ADD_TRAINER_PROB 60

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

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

int32_t move_cost[num_character_types][num_terrain_types] = {
    {INT_MAX, INT_MAX, 10, 10, 10, 20, 10, INT_MAX, INT_MAX, 10},
    {INT_MAX, INT_MAX, 10, 50, 50, 15, 10, 15, 15, INT_MAX},
    {INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX},
    {INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX},
};

class character
{
public:
  pair_t dir;
  pair_t pos;
  char symbol;
  int next_turn;
};

class pc : public character
{
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
world_t world;

static pair_t all_dirs[9] = {
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, -1},
    {0, 1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 0}};

#define rand_dir(dir)         \
  {                           \
    int _i = rand() & 0x7;    \
    dir[0] = all_dirs[_i][0]; \
    dir[1] = all_dirs[_i][1]; \
  }

static void move_hiker_func(character *c, pair_t dest)
{
  int min;
  int base;
  int i;

  base = rand() & 0x7;

  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];
  min = INT_MAX;

  for (i = base; i < 8 + base; i++)
  {
    if ((world.hiker_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                         [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] <=
         min) &&
        (!world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                             [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] ||
         world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                            [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] == &world.pc_t) &&
        c->pos[dim_x] + all_dirs[i & 0x7][dim_x] != 0 &&
        c->pos[dim_x] + all_dirs[i & 0x7][dim_x] != MAP_X - 1 &&
        c->pos[dim_y] + all_dirs[i & 0x7][dim_y] != 0 &&
        c->pos[dim_y] + all_dirs[i & 0x7][dim_y] != MAP_Y - 1)
    {
      dest[dim_x] = c->pos[dim_x] + all_dirs[i & 0x7][dim_x];
      dest[dim_y] = c->pos[dim_y] + all_dirs[i & 0x7][dim_y];
      min = world.hiker_dist[dest[dim_y]][dest[dim_x]];
    }
  }
}

static void move_rival_func(character *c, pair_t dest)
{
  int min;
  int base;
  int i;

  base = rand() & 0x7;

  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];
  min = INT_MAX;

  for (i = base; i < 8 + base; i++)
  {
    if ((world.rival_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                         [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] <
         min) &&
        (!world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                             [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] ||
         world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                            [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] == &world.pc_t) &&
        c->pos[dim_x] + all_dirs[i & 0x7][dim_x] != 0 &&
        c->pos[dim_x] + all_dirs[i & 0x7][dim_x] != MAP_X - 1 &&
        c->pos[dim_y] + all_dirs[i & 0x7][dim_y] != 0 &&
        c->pos[dim_y] + all_dirs[i & 0x7][dim_y] != MAP_Y - 1)
    {
      dest[dim_x] = c->pos[dim_x] + all_dirs[i & 0x7][dim_x];
      dest[dim_y] = c->pos[dim_y] + all_dirs[i & 0x7][dim_y];
      min = world.rival_dist[dest[dim_y]][dest[dim_x]];
    }
  }
}

static void move_pacer_func(character *c, pair_t dest)
{
  terrain_type_t t;

  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];

  t = world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                        [c->pos[dim_x] + c->dir[dim_x]];

  if ((t != ter_path && t != ter_grass && t != ter_clearing) ||
      world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                         [c->pos[dim_x] + c->dir[dim_x]])
  {
    c->dir[dim_x] *= -1;
    c->dir[dim_y] *= -1;
  }

  if ((t == ter_path || t == ter_grass || t == ter_clearing) &&
      !world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                          [c->pos[dim_x] + c->dir[dim_x]])
  {
    dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
    dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
  }
}

static void move_wanderer_func(character *c, pair_t dest)
{
  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];

  if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                         [c->pos[dim_x] + c->dir[dim_x]] !=
       world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) ||
      world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                         [c->pos[dim_x] + c->dir[dim_x]])
  {
    rand_dir(c->dir);
  }

  if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                         [c->pos[dim_x] + c->dir[dim_x]] ==
       world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) &&
      (!world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ||
       world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                          [c->pos[dim_x] + c->dir[dim_x]] == &world.pc_t))
  {
    dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
    dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
  }
}

static void move_sentry_func(character *c, pair_t dest)
{
  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];
}

static void move_explorer_func(character *c, pair_t dest)
{
  dest[dim_x] = c->pos[dim_x];
  dest[dim_y] = c->pos[dim_y];

  if ((move_cost[char_other][world.cur_map->map[c->pos[dim_y] +
                                                c->dir[dim_y]]
                                               [c->pos[dim_x] +
                                                c->dir[dim_x]]] ==
       INT_MAX) ||
      world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                         [c->pos[dim_x] + c->dir[dim_x]])
  {
    rand_dir(c->dir);
  }

  if ((move_cost[char_other][world.cur_map->map[c->pos[dim_y] +
                                                c->dir[dim_y]]
                                               [c->pos[dim_x] +
                                                c->dir[dim_x]]] !=
       INT_MAX) &&
      (!world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ||
       world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                          [c->pos[dim_x] + c->dir[dim_x]] == &world.pc_t))
  {
    dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
    dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
  }
}

static void move_pc_func(character *c, pair_t dest)
{
  dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
  dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
}

void (*move_func[num_movement_types])(character *, pair_t) = {
    move_hiker_func,
    move_rival_func,
    move_pacer_func,
    move_wanderer_func,
    move_sentry_func,
    move_explorer_func,
    move_pc_func,
};

static int32_t path_cmp(const void *key, const void *with)
{
  return ((path *)key)->cost - ((path *)with)->cost;
}

static int32_t edge_penalty(int8_t x, int8_t y)
{
  return (x == 1 || y == 1 || x == MAP_X - 2 || y == MAP_Y - 2) ? 2 : 1;
}

static void dijkstra_path(map *m, pair_t from, pair_t to)
{
  static path path_t[MAP_Y][MAP_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint16_t x, y;

  if (!initialized)
  {
    for (y = 0; y < MAP_Y; y++)
    {
      for (x = 0; x < MAP_X; x++)
      {
        path_t[y][x].pos[dim_y] = y;
        path_t[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      path_t[y][x].cost = INT_MAX;
    }
  }

  path_t[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++)
  {
    for (x = 1; x < MAP_X - 1; x++)
    {
      path_t[y][x].hn = heap_insert(&h, &path_t[y][x]);
    }
  }

  while ((p = (path *)heap_remove_min(&h)))
  {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x])
    {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path_t[y][x], x = p->from[dim_x], y = p->from[dim_y])
      {
        mapxy(x, y) = ter_path;
        heightxy(x, y) = 0;
      }
      heap_delete(&h);
      return;
    }

    if ((path_t[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
        (path_t[p->pos[dim_y] - 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1))))
    {
      path_t[p->pos[dim_y] - 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1));
      path_t[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path_t[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path_t[p->pos[dim_y] - 1]
                                             [p->pos[dim_x]]
                                                 .hn);
    }
    if ((path_t[p->pos[dim_y]][p->pos[dim_x] - 1].hn) &&
        (path_t[p->pos[dim_y]][p->pos[dim_x] - 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]))))
    {
      path_t[p->pos[dim_y]][p->pos[dim_x] - 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]));
      path_t[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path_t[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path_t[p->pos[dim_y]]
                                             [p->pos[dim_x] - 1]
                                                 .hn);
    }
    if ((path_t[p->pos[dim_y]][p->pos[dim_x] + 1].hn) &&
        (path_t[p->pos[dim_y]][p->pos[dim_x] + 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]))))
    {
      path_t[p->pos[dim_y]][p->pos[dim_x] + 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]));
      path_t[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path_t[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path_t[p->pos[dim_y]]
                                             [p->pos[dim_x] + 1]
                                                 .hn);
    }
    if ((path_t[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&
        (path_t[p->pos[dim_y] + 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1))))
    {
      path_t[p->pos[dim_y] + 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1));
      path_t[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path_t[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path_t[p->pos[dim_y] + 1]
                                             [p->pos[dim_x]]
                                                 .hn);
    }
  }
}

static int build_paths(map *m)
{
  pair_t from, to;

  /*  printf("%d %d %d %d\n", m->n, m->s, m->e, m->w);*/

  if (m->e != -1 && m->w != -1)
  {
    from[dim_x] = 1;
    to[dim_x] = MAP_X - 2;
    from[dim_y] = m->w;
    to[dim_y] = m->e;

    dijkstra_path(m, from, to);
  }

  if (m->n != -1 && m->s != -1)
  {
    from[dim_y] = 1;
    to[dim_y] = MAP_Y - 2;
    from[dim_x] = m->n;
    to[dim_x] = m->s;

    dijkstra_path(m, from, to);
  }

  if (m->e == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->w == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->n == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->s == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }

    dijkstra_path(m, from, to);
  }

  return 0;
}

static int gaussian[5][5] = {
    {1, 4, 7, 4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1, 4, 7, 4, 1}};

static int smooth_height(map *m)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  /*  FILE *out;*/
  uint8_t height[MAP_Y][MAP_X];

  memset(&height, 0, sizeof(height));

  /* Seed with some values */
  for (i = 1; i < 255; i += 20)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (height[y][x]);
    height[y][x] = i;
    if (i == 1)
    {
      head = tail = (queue_node_t *)malloc(sizeof(*tail));
    }
    else
    {
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);
  */

  /* Diffuse the vaules to fill the space */
  while (head)
  {
    x = head->x;
    y = head->y;
    i = height[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !height[y - 1][x - 1])
    {
      height[y - 1][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !height[y][x - 1])
    {
      height[y][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < MAP_Y && !height[y + 1][x - 1])
    {
      height[y + 1][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !height[y - 1][x])
    {
      height[y - 1][x] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < MAP_Y && !height[y + 1][x])
    {
      height[y + 1][x] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < MAP_X && y - 1 >= 0 && !height[y - 1][x + 1])
    {
      height[y - 1][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < MAP_X && !height[y][x + 1])
    {
      height[y][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < MAP_X && y + 1 < MAP_Y && !height[y + 1][x + 1])
    {
      height[y + 1][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  /* And smooth it a bit with a gaussian convolution */
  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }
  /* Let's do it again, until it's smooth like Kenny G. */
  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->height, sizeof (m->height), 1, out);
  fclose(out);
  */

  return 0;
}

static void find_building_location(map *m, pair_t p)
{
  do
  {
    p[dim_x] = rand() % (MAP_X - 3) + 1;
    p[dim_y] = rand() % (MAP_Y - 3) + 1;

    if ((((mapxy(p[dim_x] - 1, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] - 1, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x] + 2, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] + 2, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] - 1) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] - 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] + 2) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 2) == ter_path))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_center))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_path))))
    {
      break;
    }
  } while (1);
}

static int place_pokemart(map *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_mart;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_mart;

  return 0;
}

static int place_center(map *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_center;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_center;

  return 0;
}

/**
 * @brief creating map terrain
 *
 * @param m
 * @param n
 * @param s
 * @param e
 * @param w
 * @return int
 */
static int maperrain(map *m, int8_t n, int8_t s, int8_t e, int8_t w)
{
  int32_t i, x, y;
  queue_node_t *head, *tail, *tmp;
  //  FILE *out;
  int num_grass, num_clearing, num_mountain, num_forest, num_total;
  terrain_type_t type;
  int added_current = 0;

  num_grass = rand() % 4 + 2;
  num_clearing = rand() % 4 + 2;
  num_mountain = rand() % 2 + 1;
  num_forest = rand() % 2 + 1;
  num_total = num_grass + num_clearing + num_mountain + num_forest;

  memset(&m->map, 0, sizeof(m->map));

  /* Seed with some values */
  for (i = 0; i < num_total; i++)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (m->map[y][x]); // while m->map[y][x] is not 0
    if (i == 0)
    {
      type = ter_grass;
    }
    else if (i == num_grass)
    {
      type = ter_clearing;
    }
    else if (i == num_grass + num_clearing)
    {
      type = ter_mountain;
    }
    else if (i == num_grass + num_clearing + num_mountain)
    {
      type = ter_forest;
    }
    m->map[y][x] = type;
    if (i == 0)
    {
      head = tail = (queue_node_t *)malloc(sizeof(*tail));
    }
    else
    {
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */

  /* Diffuse the vaules to fill the space */
  while (head)
  {
    x = head->x;
    y = head->y;
    i = m->map[y][x];

    if (x - 1 >= 0 && !m->map[y][x - 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x - 1] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x - 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y - 1 >= 0 && !m->map[y - 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y - 1][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y - 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y + 1 < MAP_Y && !m->map[y + 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y + 1][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y + 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (x + 1 < MAP_X && !m->map[y][x + 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x + 1] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x + 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = (terrain_type_t)i;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    added_current = 0;
    tmp = head;
    head = head->next;
    free(tmp);
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (y == 0 || y == MAP_Y - 1 ||
          x == 0 || x == MAP_X - 1)
      {
        mapxy(x, y) = ter_boulder;
      }
    }
  }

  m->n = n;
  m->s = s;
  m->e = e;
  m->w = w;

  if (n != -1)
  {
    mapxy(n, 0) = ter_exit;
    mapxy(n, 1) = ter_path;
  }
  if (s != -1)
  {
    mapxy(s, MAP_Y - 1) = ter_exit;
    mapxy(s, MAP_Y - 2) = ter_path;
  }
  if (w != -1)
  {
    mapxy(0, w) = ter_exit;
    mapxy(1, w) = ter_path;
  }
  if (e != -1)
  {
    mapxy(MAP_X - 1, e) = ter_exit;
    mapxy(MAP_X - 2, e) = ter_path;
  }

  return 0;
}

static int place_boulders(map *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_BOULDERS || rand() % 100 < BOULDER_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_forest && m->map[y][x] != ter_path)
    {
      m->map[y][x] = ter_boulder;
    }
  }

  return 0;
}

static int place_trees(map *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_TREES || rand() % 100 < TREE_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_mountain && m->map[y][x] != ter_path)
    {
      m->map[y][x] = ter_tree;
    }
  }

  return 0;
}

void rand_pos(pair_t pos)
{
  pos[dim_x] = (rand() % (MAP_X - 2)) + 1;
  pos[dim_y] = (rand() % (MAP_Y - 2)) + 1;
}

void new_hiker()
{
  pair_t pos;
  npc *c;

  do
  {
    rand_pos(pos);
  } while (world.hiker_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  c = (npc *)malloc(sizeof(*c));
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_hiker;
  c->mtype = move_hiker;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->symbol = 'h';
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void new_rival()
{
  pair_t pos;
  npc *c;

  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  c = (npc *)malloc(sizeof(*c));
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_rival;
  c->mtype = move_rival;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->symbol = 'r';
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void new_char_other()
{
  pair_t pos;
  npc *c;

  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  c = (npc *)malloc(sizeof(*c));
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_other;
  switch (rand() % 4)
  {
  case 0:
    c->mtype = move_pace;
    c->symbol = 'p';
    break;
  case 1:
    c->mtype = move_wander;
    c->symbol = 'w';
    break;
  case 2:
    c->mtype = move_sentry;
    c->symbol = 's';
    break;
  case 3:
    c->mtype = move_explore;
    c->symbol = 'e';
    break;
  }
  rand_dir(c->dir);
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void place_characters()
{
  int num_trainers = 2;

  // Always place a hiker and a rival, then place a random number of others
  new_hiker();
  new_rival();
  do
  {
    // higher probability of non- hikers and rivals
    switch (rand() % 10)
    {
    case 0:
      new_hiker();
      break;
    case 1:
      new_rival();
      break;
    default:
      new_char_other();
      break;
    }
  } while (++num_trainers < MIN_TRAINERS ||
           (rand() % 100) < ADD_TRAINER_PROB);
}

int32_t cmp_char_turns(const void *key, const void *with)
{
  return ((character *)key)->next_turn - ((character *)with)->next_turn;
}

void delete_character(void *v)
{
  if (v != &world.pc_t) // if character is the world's pc
  {
    free(v); // free character
  }
}

/**
 * @brief where did the PC come from???
 *
 * @param direction
 */
void init_pc(dir_t direction)
{
  int x, y;

  switch (direction)
  {
  case (north):
    x = world.cur_map->n;
    y = 1;
    break;
  case (east):
    x = MAP_X - 2;
    y = world.cur_map->e;
    break;
  case (south):
    x = world.cur_map->s;
    y = MAP_Y - 2;
    break;
  case (west):
    x = 1;
    y = world.cur_map->w;
    break;
  default:
    do
    {
      x = rand() % (MAP_X - 2) + 1;
      y = rand() % (MAP_Y - 2) + 1;
    } while (world.cur_map->map[y][x] != ter_path); // try again if its not the path
    break;
  }

  world.pc_t.pos[dim_x] = x;
  world.pc_t.pos[dim_y] = y;
  world.pc_t.symbol = '@';

  world.cur_map->cmap[y][x] = &world.pc_t; // set current character map's position to the world's pc

  if (world.cur_map->turn.size == 0)
  {
    world.pc_t.next_turn = 0;
    heap_insert(&world.cur_map->turn, &world.pc_t);
  }
  // insert the heap of turns and the pointer to pc
}

#define ter_cost(x, y, c) move_cost[c][m->map[y][x]]

static int32_t hiker_cmp(const void *key, const void *with)
{
  return (world.hiker_dist[((path *)key)->pos[dim_y]]
                          [((path *)key)->pos[dim_x]] -
          world.hiker_dist[((path *)with)->pos[dim_y]]
                          [((path *)with)->pos[dim_x]]);
}

static int32_t rival_cmp(const void *key, const void *with)
{
  return (world.rival_dist[((path *)key)->pos[dim_y]]
                          [((path *)key)->pos[dim_x]] -
          world.rival_dist[((path *)with)->pos[dim_y]]
                          [((path *)with)->pos[dim_x]]);
}

void pathfind(map *m)
{
  heap_t h;
  uint32_t x, y;
  static path p[MAP_Y][MAP_X], *c;
  static uint32_t initialized = 0;

  if (!initialized)
  {
    initialized = 1;
    for (y = 0; y < MAP_Y; y++)
    {
      for (x = 0; x < MAP_X; x++)
      {
        p[y][x].pos[dim_y] = y;
        p[y][x].pos[dim_x] = x;
      }
    }
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      world.hiker_dist[y][x] = world.rival_dist[y][x] = INT_MAX;
    }
  }
  world.hiker_dist[world.pc_t.pos[dim_y]][world.pc_t.pos[dim_x]] =
      world.rival_dist[world.pc_t.pos[dim_y]][world.pc_t.pos[dim_x]] = 0;

  heap_init(&h, hiker_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++)
  {
    for (x = 1; x < MAP_X - 1; x++)
    {
      if (ter_cost(x, y, char_hiker) != INT_MAX)
      {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      }
      else
      {
        p[y][x].hn = NULL;
      }
    }
  }

  while ((c = (path *)heap_remove_min(&h)))
  {
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x]] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x]] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y]][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y]][c->pos[dim_x] - 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y]][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y]][c->pos[dim_x] + 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x]] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x]] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker)))
    {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
          world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);

  heap_init(&h, rival_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++)
  {
    for (x = 1; x < MAP_X - 1; x++)
    {
      if (ter_cost(x, y, char_rival) != INT_MAX)
      {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      }
      else
      {
        p[y][x].hn = NULL;
      }
    }
  }

  while ((c = (path *)heap_remove_min(&h)))
  {
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x]] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x]] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y]][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y]][c->pos[dim_x] - 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y]][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y]][c->pos[dim_x] + 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x]] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x]] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
             ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival)))
    {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
          world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
          ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);
}

// New map expects cur_idx to refer to the index to be generated.  If that
// map has already been generated then the only thing this does is set
// cur_map.
static int new_map(dir_t direction)
{
  int d, p;
  int e, w, n, s;
  int x, y;

  // if map already exists
  if (world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x]])
  {
    world.cur_map = world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x]];

    init_pc(direction);
    return 0;
  }

  world.cur_map =
      world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x]] = (map *)
          malloc(sizeof(*world.cur_map));

  smooth_height(world.cur_map);

  if (!world.cur_idx[dim_y])
  { // if current map's y is 0
    n = -1;
  }
  else if (world.w[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]])
  {
    n = world.w[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]]->s;
  }
  else
  {
    n = 1 + rand() % (MAP_X - 2);
  }
  if (world.cur_idx[dim_y] == WORLD_SIZE - 1)
  {
    s = -1;
  }
  else if (world.w[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]])
  {
    s = world.w[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]]->n;
  }
  else
  {
    s = 1 + rand() % (MAP_X - 2);
  }
  if (!world.cur_idx[dim_x])
  {
    w = -1;
  }
  else if (world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1])
  {
    w = world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1]->e;
  }
  else
  {
    w = 1 + rand() % (MAP_Y - 2);
  }
  if (world.cur_idx[dim_x] == WORLD_SIZE - 1)
  {
    e = -1;
  }
  else if (world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1])
  {
    e = world.w[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1]->w;
  }
  else
  {
    e = 1 + rand() % (MAP_Y - 2);
  }

  maperrain(world.cur_map, n, s, e, w);

  place_boulders(world.cur_map);
  place_trees(world.cur_map);
  build_paths(world.cur_map);

  d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
       abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  p = d > 200 ? 5 : (50 - ((45 * d) / 200));
  //  printf("d=%d, p=%d\n", d, p);
  if ((rand() % 100) < p || !d)
  {
    place_pokemart(world.cur_map);
  }
  if ((rand() % 100) < p || !d)
  {
    place_center(world.cur_map);
  }

  // end terrain generation

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      world.cur_map->cmap[y][x] = NULL;
    }
  }

  heap_init(&world.cur_map->turn, cmp_char_turns, delete_character);

  init_pc(direction);
  pathfind(world.cur_map);
  place_characters();

  return 0;
}

static void print_map()
{
  int x, y;
  int default_reached = 0;

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.cur_map->cmap[y][x])
      {
        if (world.cur_map->cmap[y][x] == &world.pc_t)
        {
          attron(COLOR_PAIR(ter_mart));
          mvaddch(y + 1, x, '@');
          attroff(COLOR_PAIR(ter_mart));
        }
        else
        {
          attron(COLOR_PAIR(ter_clearing));
          mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
          attroff(COLOR_PAIR(ter_clearing));
        }
      }
      else
      {
        switch (world.cur_map->map[y][x])
        {
        case ter_boulder:
          /* Special-casing the border.  It needs to be boulder so hikers
           * can't go there, but I like the look of the mountains better.
           * The other option would be to add a border terrain, which is
           * probably the better design decision, actually.
           */
        case ter_mountain:
          attron(COLOR_PAIR(ter_mountain));
          mvaddch(y + 1, x, '%');
          attroff(COLOR_PAIR(ter_mountain));
          break;
        case ter_tree:
        case ter_forest:
          attron(COLOR_PAIR(ter_tree));
          mvaddch(y + 1, x, '^');
          attroff(COLOR_PAIR(ter_tree));
          break;
        case ter_path:
          printw("%c", '#');
          break;
        case ter_mart:
          attron(COLOR_PAIR(ter_center));
          mvaddch(y + 1, x, 'M');
          attroff(COLOR_PAIR(ter_center));
          break;
        case ter_center:
          attron(COLOR_PAIR(ter_center));
          mvaddch(y + 1, x, 'C');
          attroff(COLOR_PAIR(ter_center));
          break;
        case ter_grass:
          attron(COLOR_PAIR(ter_grass));
          mvaddch(y + 1, x, ':');
          attroff(COLOR_PAIR(ter_grass));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(ter_grass));
          mvaddch(y + 1, x, '.');
          attroff(COLOR_PAIR(ter_grass));
          break;
        case ter_exit:
          printw("%c", '#');
          break;
        default:
          default_reached = 1;
          break;
        }
      }
    }
    printw("\n");
  }

  if (default_reached)
  {
    fprintf(stderr, "Default reached in %s\n", __FUNCTION__);
  }
}

/**
 * @brief type is either 'c' or 'm'
 *
 * @param type
 */
void print_interior(int8_t type)
{

  // uint8_t t;
  for (uint8_t j = 0; j < MAP_Y; j++)
  {
    for (uint8_t i = 0; i < MAP_X; i++)
    {
      // t = 0;
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_center));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_center));
      }
      else
      {
        switch (type)
        {
        case 'c':
          printw("%c", 'C');
          break;
        case 'm':
          printw("%c", 'M');
          break;
        }
      }

      // else
      // {
      //   while (j == 5 && t++ < 6) {
      //     printw("%c", '#');
      //   }
      //   if ((j == 6 || j == 7) && ()) {

      //   }
    }
    printw("\n");
  }
}

void print_fight_screen()
{
  clear();
  printw("Hi! I like shorts!\n");
  for (uint8_t j = 0; j < MAP_Y; j++)
  {
    for (uint8_t i = 0; i < MAP_X; i++)
    {
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_clearing));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_clearing));
      }
      else
      {
        printw("%c", '.');
      }
    }
    printw("\n");
  }

  char ch;
  do
  {
    ch = getch();
  } while (ch != 27);

  clear();
  printw("You got $42,069.");
  print_map();
}

/**
 * @brief 
 * 
 * @param p 
 * @param lvl 
 * @param move1 
 * @param move2 
 * @param stats   contains the stats.
 * @param gender  0 for male, 1 for female 
 */
void print_enc_screen(pokemon_t *p, int lvl, string move1, string move2, int stats[6], int gender, int shiny)
{
  int i = 0, j;
  char ch;
  string _shiny = ((shiny) ? ("") : ("SHINY!!!!!11!!! "));
  int lvl_stats[6];
  
  // cout << stats[0] << " " << stats[1] << " " << stats[2] << " " << stats[3] << " " << stats[4] << " " << stats[5] << " " << endl; 
  // printw("%d %d %d %d %d %d\n", stats[0], stats[1], stats[2], stats[3], stats[4], stats[5]);
  // printw("1: %d\n", strlen(const_cast<char*>(move1.c_str())));
  // printw("2: %d\n", strlen(const_cast<char*>(move2.c_str())));
  loop:

  lvl_stats[0] = (stats[0] * 2 * lvl) / 100 + lvl + 10;
  for (i = 1; i < 6; i++) {
    lvl_stats[i] = (stats[i] * 2 * lvl) / 100 + 5;
  } 

  clear();
  printw("A ");
  mvaddstr(0, 2, const_cast<char *>((_shiny).c_str()));
  printw("wild ");
  mvaddstr(0, 7 + strlen(const_cast<char *>((_shiny).c_str())), const_cast<char *>((p->iden).c_str()));
  printw(" appeared!");
  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_grass));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_grass));
      }
      else if (j == 2 && i == 4)
      {
        printw("Lvl: %3d", lvl);
      }
      else if (j == 3 && i == 4)
      {
        printw("Moves: "); 
        mvaddstr(4, 11, const_cast<char*>(move1.c_str()));
        printw(", ");
        mvaddstr(4, 13 + strlen(const_cast<char*>(move1.c_str())), const_cast<char*>(move2.c_str()));
      }
      else if (j == 4 && i == 4)
      {
        printw("HP: %3d, Atk: %3d, Def: %3d, SpAtk: %3d, SpDef: %3d, Speed: %3d", lvl_stats[0], lvl_stats[1], lvl_stats[2], lvl_stats[3], lvl_stats[4], lvl_stats[5]);
      }
      else if (j == 5 && i == 4)
      {
        printw("Gender: %s", ((!gender) ? ("male") : ("female")));
      }
      else if (j == 6 && i == 4)
      {
        printw("(press <l> to level up)");
      }
      else
      {
        printw("%c", '.');
      }
    }
    printw("\n");
  }

  do
  {
    ch = getch();
    if (ch == 'l') {
      lvl += 1;
      goto loop; 
    }
  } while (ch != 27);

  clear();
}

void print_char_list()
{
  int ch;
  uint8_t i, j;
  uint32_t count;
  uint32_t arr_size = world.cur_map->turn.size;
  character *arr[arr_size];
  char modifier_x[6], modifier_y[6];
  char fought[11];
  int start = 2;
  uint32_t index = 1;

  count = 0;

  for (i = 0; i < arr_size; i++)
  {
    arr[i] = (character *)malloc(sizeof(character *));
    arr[i] = (character *)heap_remove_min(&world.cur_map->turn);
  }

start:
  printw("Displaying trainers...\n");
  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_tree));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_tree));
      }
      else if (j >= start && i == 4)
      {
        if (count < arr_size)
        {
          if (arr[count] != &world.pc_t)
          {
            ((world.pc_t.pos[dim_x] - arr[count]->pos[dim_x]) < 0) ? (strcpy(modifier_x, "east")) : (strcpy(modifier_x, "west"));

            ((world.pc_t.pos[dim_y] - arr[count]->pos[dim_y]) < 0) ? (strcpy(modifier_y, "south")) : (strcpy(modifier_y, "north"));

            (((npc *)arr[count])->fought) ? (strcpy(fought, "fought")) : (strcpy(fought, "not fought"));

            if (j < MAP_Y - 1)
            {
              printw("%c: %2d %s, %2d %s (%s)", arr[count]->symbol,
                     abs((world.pc_t.pos[dim_x] - arr[count]->pos[dim_x])),
                     modifier_x,
                     abs(world.pc_t.pos[dim_y] - arr[count]->pos[dim_y]),
                     modifier_y, fought);
              i = strlen(modifier_x) + strlen(modifier_y) + 14;
            }
          }
          else
            j--;
          count++;
          continue;
        }
        else
          printw("%s", " ");
      }
      else
        printw("%s", " ");
    }
    printw("\n");
  }

  count = 0;
  keypad(stdscr, TRUE);
  do
  {
    ch = getch();
    if (ch == KEY_DOWN && index < arr_size - 1 && arr_size > MAP_Y - 1)
    {
      if (start <= 1)
      {
        index++;
      }
      count = index;
      start--;
      goto start;
    }
    else if (ch == KEY_UP && index > 1)
    {
      if (start <= 1)
      {
        index--;
      }
      count = index;
      start++;
      goto start;
    }
  } while (ch != 27);

  for (i = 0; i < arr_size; i++)
  {
    heap_insert(&world.cur_map->turn, arr[i]);
  }
  clear();
  printw("Returned to map.\n");
  print_map();
}

void set_color()
{
  start_color();
  init_pair(ter_mountain, COLOR_MAGENTA, -1);
  init_pair(ter_tree, COLOR_GREEN, -1);
  init_pair(ter_path, COLOR_WHITE, -1);
  init_pair(ter_center, COLOR_BLUE, -1);
  init_pair(ter_grass, COLOR_CYAN, -1);
  init_pair(ter_mart, COLOR_YELLOW, -1);  // this is actually pc
  init_pair(ter_clearing, COLOR_RED, -1); // this is actually npc
}

// The world is global because of its size, so init_world is parameterless
void init_world()
{
  world.cur_idx[dim_x] = world.cur_idx[dim_y] = WORLD_SIZE / 2; // 201, 201
  new_map(null);
}

void delete_world()
{
  int x, y;

  // Only correct because current game never leaves the initial map
  // Need to iterate over all maps in 1.05+
  heap_delete(&world.cur_map->turn);

  for (y = 0; y < WORLD_SIZE; y++)
  {
    for (x = 0; x < WORLD_SIZE; x++)
    {
      if (world.w[y][x])
      {
        free(world.w[y][x]);
        world.w[y][x] = NULL;
      }
    }
  }
}

void print_hiker_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.hiker_dist[y][x] == INT_MAX)
      {
        printf("   ");
      }
      else
      {
        printf(" %5d", world.hiker_dist[y][x]);
      }
    }
    printf("\n");
  }
}

void print_rival_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.rival_dist[y][x] == INT_MAX || world.rival_dist[y][x] < 0)
      {
        printf("   ");
      }
      else
      {
        printf(" %02d", world.rival_dist[y][x] % 100);
      }
    }
    printf("\n");
  }
}

void exit_map(uint8_t x, uint8_t y)
{
  if (x == 0 && world.cur_idx[dim_x]) // if exit is west
  {
    world.cur_idx[dim_x]--;
    new_map(east);
  }
  else if (x == MAP_X - 1 && world.cur_idx[dim_x] < WORLD_SIZE - 1) // if exit is east
  {
    world.cur_idx[dim_x]++;
    new_map(west);
  }
  else if (y == 0 && world.cur_idx[dim_y]) // if exit is north
  {
    world.cur_idx[dim_y]--;
    new_map(south);
  }
  else if (y == MAP_Y - 1 && world.cur_idx[dim_y] < WORLD_SIZE - 1) // if exit is south
  {
    world.cur_idx[dim_y]++;
    new_map(north);
  }
}

void spawn_pokemon()
{
  int i, j = 0, t = 0, lvl = 0;
  int d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  int moves_id[1000];
  int stats[6] = {0};
  int moveid1 = -1, moveid2 = -1;
  string move1, move2;
  pokemon_t* p = pkm[rand() % 1092];
  int id = p->spec_id;

  // pkm level
  if (d == 0 || d == 1)
  {
    lvl = 1;
  }
  else if (d == 201) {
    lvl = rand() % 100;
  }
  else if (d == 400) {
    lvl = 100;
  }
  else
  {
    lvl = (d <= 200) ? ((rand() % (int)(d / 2)) + 1) : ((rand() % (100 - (int)((d - 200) / 2))) + (int)((d - 200) / 2));
  }

  // pkm moves
  for (i = 0; !t; i++) {
    for (j = j; !t; j++) {
      if (pkm_mvs[j]->id == id && pkm_mvs[j]->pkm_move_method_id == 1) {
        moves_id[i] = *((int *) malloc (sizeof(int*)));
        moves_id[i] = pkm_mvs[j]->move_id;
        j++;
        break;
      }  
      if (pkm_mvs[j]->id > id) {
        t = 1;
        break;
      }
    }
  }

  moveid1 = moves_id[rand() % i];
  do {
    moveid2 = moves_id[rand() % i];

  } while (moveid2 == moveid1);

  i = 0;
  while (((move1 == "") || (move2 == "")) && (i < 845)) {
    if (mvs[i]->id == moveid1) {move1 = mvs[i]->iden;}
    else if (mvs[i]->id == moveid2) {move2 = mvs[i]->iden; }
    i++;
  } 

  // stats
  for (i = 0; i < 6553; i++) {
    if (pkm_sts[i]->pokemon_id == id) {
      stats[0] = pkm_sts[i]->base_stat + (rand() % 16);     // hp
      stats[1] = pkm_sts[i + 1]->base_stat + (rand() % 16); // atk
      stats[2] = pkm_sts[i + 2]->base_stat + (rand() % 16); // def
      stats[3] = pkm_sts[i + 3]->base_stat + (rand() % 16); // spatk
      stats[4] = pkm_sts[i + 4]->base_stat + (rand() % 16); // spdef
      stats[5] = pkm_sts[i + 5]->base_stat + (rand() % 16); // spd
      break;
    } 
  }


  print_enc_screen(p, lvl, move1, move2, stats, rand() % 2, rand() % 8192);

  // char ch;
  // do
  // {
  //   ch = getch();
  // } while (ch != 27);
  // free(p);
}

void game_loop(int move)
{
  character *c;
  pair_t d = {world.pc_t.pos[dim_y], world.pc_t.pos[dim_x]};
  uint8_t moved = 0;
  uint8_t x = world.pc_t.pos[dim_x] + all_dirs[move][dim_x];
  uint8_t y = world.pc_t.pos[dim_y] + all_dirs[move][dim_y];
  character *other = world.cur_map->cmap[y][x];

  if (!other && world.cur_map->map[y][x] == ter_grass && rand() % 10 == 0)
  {
    spawn_pokemon();
    goto exit;
  }
  if (move_cost[char_pc][world.cur_map->map[y][x]] == INT_MAX)
  {
    printw("My g, my man, my homie, you cannot go there.\n");
    goto exit;
  }
  else if (other && other != &world.pc_t && ((npc *)other)->fought)
  {
    printw("My g, my man, my homie, you just beat him up.\n");
    world.cur_map->cmap[world.pc_t.pos[dim_y]][world.pc_t.pos[dim_x]] = other;
    world.cur_map->cmap[y][x] = &world.pc_t;
    world.pc_t.pos[dim_x] = x;
    world.pc_t.pos[dim_y] = y;
    // FIXME
    goto exit;
  }
  else if (world.cur_map->map[y][x] == ter_exit)
  {
    // if the character move into an exit, create a new map, world location, etc.
    // also update the world's pc location, in character map
    // when returning back to map, the queues should remain the same.
    exit_map(x, y);
    printw("Entered map (%d, %d).\n", world.cur_idx[dim_x], world.cur_idx[dim_y]);
    print_map();
    goto exit;
  }

  printw("\n");
  while (!moved)
  {
    c = (character *)heap_remove_min(&world.cur_map->turn);
    if (c == &world.pc_t)
    {
      moved = 1;

      if (other && other != &world.pc_t && !(((npc *)other)->fought))
      {
        print_fight_screen();
        ((npc *)other)->mtype = move_sentry;
        ((npc *)other)->fought = 1;
        goto heap;
      }

      c->dir[dim_x] = all_dirs[move][dim_x];
      c->dir[dim_y] = all_dirs[move][dim_y];

      move_pc_func(c, d);

      world.cur_map->cmap[c->pos[dim_y]][c->pos[dim_x]] = NULL;
      world.cur_map->cmap[d[dim_y]][d[dim_x]] = c;

      c->next_turn += move_cost[char_pc][world.cur_map->map[d[dim_y]]
                                                           [d[dim_x]]];
      c->pos[dim_y] = d[dim_y];
      c->pos[dim_x] = d[dim_x];

      pathfind(world.cur_map);
    }
    else
    {
      move_func[((npc *)c)->mtype](c, d);

      if (world.cur_map->cmap[d[dim_y]][d[dim_x]] &&
          world.cur_map->cmap[d[dim_y]][d[dim_x]] == &world.pc_t &&
          !((npc *)c)->fought)
      {

        print_fight_screen();
        ((npc *)c)->mtype = move_sentry;
        ((npc *)c)->fought = 1;
        moved = 1;
        goto exit;
      }

      world.cur_map->cmap[c->pos[dim_y]][c->pos[dim_x]] = NULL;
      world.cur_map->cmap[d[dim_y]][d[dim_x]] = c;
      c->next_turn += move_cost[((npc *)c)->ctype][world.cur_map->map[d[dim_y]]
                                                                     [d[dim_x]]];
      c->pos[dim_y] = d[dim_y];
      c->pos[dim_x] = d[dim_x];
    }
  heap:
    heap_insert(&world.cur_map->turn, c);
  }
exit:
  print_map();
}

int main(int argc, char *argv[])
{
  struct timeval tv;
  uint32_t seed;
  //  int x, y;

  if (argc == 2)
  {
    seed = atoi(argv[1]);
  }
  else
  {
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  printf("Using seed: %u\n", seed);
  srand(seed);

  init_files();
  // read file here
  // if (argc > 1)
  // {
  //   if (!strcmp(argv[1], "pokemon"))
  //   {
  //     print_files(0);
  //   }
  //   else if (!strcmp(argv[1], "moves"))
  //   {
  //     print_files(1);
  //   }
  //   else if (!strcmp(argv[1], "pokemon_moves"))
  //   {
  //     print_files(2);
  //   }
  //   else if (!strcmp(argv[1], "pokemon_species"))
  //   {
  //     print_files(3);
  //   }
  //   else if (!strcmp(argv[1], "experience"))
  //   {
  //     print_files(4);
  //   }
  //   else if (!strcmp(argv[1], "type_names"))
  //   {
  //     print_files(5);
  //   }
  //   else if (!strcmp(argv[1], "pokemon_stats"))
  //   {
  //     print_files(6);
  //   }
  //   else if (!strcmp(argv[1], "stats"))
  //   {
  //     print_files(7);
  //   }
  //   else if (!strcmp(argv[1], "pokemon_types"))
  //   {
  //     print_files(8);
  //   }
  //   else
  //   {
  //     cout << "Invalid input" << endl;
  //   }
  // }
  // else
  // {
  //   cout << "No files specified. exiting." << endl;
  // }

  initscr();
  use_default_colors();
  if (has_colors() == FALSE)
  {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  set_color();
  refresh();
  clear();
  init_world();

  noecho();
  int8_t ch;

  printw("You are in map (%d, %d)\n", world.cur_idx[dim_x], world.cur_idx[dim_y]);
  print_map();
  do
  {
    ch = getch();
    clear();
    switch (ch)
    {
    case ('7'):
    case ('y'):
      game_loop(0);
      break;
    case ('8'):
    case 'k':
      game_loop(3);
      break;
    case ('9'):
    case 'u':
      game_loop(5);
      break;
    case ('4'):
    case 'h':
      game_loop(1);
      break;
    case ('2'):
    case 'j':
      game_loop(4);
      break;
    case ('6'):
    case 'l':
      game_loop(6);
      break;
    case ('1'):
    case 'b':
      game_loop(2);
      break;
    case ('3'):
    case 'n':
      game_loop(7);
      break;
    case 'f':
    {
      uint8_t x = 0, y = 0;
      echo();
    loop:
      clear();
      printw("Where to? ");
      print_map();
      move(0, 10);

      printw("x: ");
      do
      {
        ch = getch();
        if (isdigit(ch))
        {
          x = (ch - 48) + (x * 10);
        }
        else if (ch == ' ' || ch == '\n')
          break;
        else
          goto loop;
      } while (1);

      printw("y: ");
      do
      {
        ch = getch();
        if (isdigit(ch))
        {
          y = (ch - 48) + (y * 10);
        }
        else if (ch == ' ' || ch == '\n')
          break;
        else
          goto loop;
      } while (1);

      if (x >= -(WORLD_SIZE / 2) && x <= WORLD_SIZE / 2 &&
          y >= -(WORLD_SIZE / 2) && y <= WORLD_SIZE / 2)
      {
        world.cur_idx[dim_x] = x + (WORLD_SIZE / 2);
        world.cur_idx[dim_y] = y + (WORLD_SIZE / 2);
        world.cur_map->cmap[world.pc_t.pos[dim_y]][world.pc_t.pos[dim_x]] = NULL;
        new_map(null);
        clear();
        printw("You are in map (%d, %d)\n", world.cur_idx[dim_x], world.cur_idx[dim_y]);
        print_map();
      }
      noecho();
      break;
    }
    case '>':
      do
      {
        clear();
        if (world.cur_map->map[world.pc_t.pos[dim_y]]
                              [world.pc_t.pos[dim_x]] == ter_center)
        {
          printw("\"Welcome to the Pokémon Center, where we will heal your Pokémon back to full health.\"\n");
          print_interior('c');
        }
        else if (world.cur_map->map[world.pc_t.pos[dim_y]]
                                   [world.pc_t.pos[dim_x]] == ter_mart)
        {
          printw("\"...Pokédoll for $799,999.\"\n");
          print_interior('m');
        }
        ch = getch();
      } while (ch != '<');
      clear();
      printw("\"Whirrr\"\n");
      print_map();
      break;
    case '.':
    case '5':
    case ' ':
      game_loop(8);
      break;
    case 't':
      print_char_list();
      break;
    default:
      printw("\n");
      print_map();
      break;
    }
  } while (ch != 'Q');

  delete_world();
  endwin();
  return 0;
}
