#include "poke327.h"
#include "print.cpp"
#include "algorithm.cpp"

using namespace std;

world_t world;
WINDOW *win;

pair_t all_dirs[9] = {
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, -1},
    {0, 1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 0}};

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

void spawn_pokemon(mon *m)
{
  int i = 0;

  //manhantan distance
  int d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  
  // randomly spawned pokemon
  if (m->lv == 0) {

    pokemon_t p = *pkm[rand() % 1092];
    // pokemon_t p = *pkm[520];
    m->id = p.id;
    m->iden = p.iden;

    pokemon_types_t **find_type = find_if (begin(pkm_types), end(pkm_types), 
        [&] (const auto &t) { return t->pokemon_id == m->id; } );
    
    m->type[0] = (*find_type)->type_id;
    (find_type != (end(pkm_types) - 1) && (*(find_type++))->pokemon_id == m->id) ? (m->type[1] = (*find_type)->type_id) : (m->type[1] = 0);

    // printw("%d, 1: %d, 2: %d\n", m->id, m->type[0], m->type[1]);
    // halt();
    // pkm level
    if (d == 0 || d == 1)
    {
      m->lv = 1;
    }
    else if (d == 201) {
      m->lv = rand() % 100;
    }
    else if (d == 400) {
      m->lv = 100;
    }
    else
    {
      m->lv = (d <= 200) ? ((rand() % (int)(d / 2)) + 1) : ((rand() % (100 - (int)((d - 200) / 2))) + (int)((d - 200) / 2));
    }  

  }

  // printw("lv %d id: %d, iden: %s\n", m->lv, m->id, str(m->iden));
  // halt();

  // pkm moves
  auto find_move_list = find_if(begin(pkm_mvs), end(pkm_mvs), [&](const auto &_m) { return _m->id == m->id && _m->ver_grp_id > 8;});
  moves_t **find_move_id;
  int version = (*find_move_list)->ver_grp_id;
  // printw("version: %d\n", version);
  // halt();
  
  for (; find_move_list != (end(pkm_mvs) - 1) && ((*find_move_list)->id == m->id) && (*find_move_list)->ver_grp_id == version ; find_move_list++) {
    if ((*find_move_list)->pkm_move_method_id == 1) {
        find_move_id = find_if(begin(mvs), end(mvs), [&](const auto &_m) { return _m->id == (*find_move_list)->move_id; });
          
        m->all_moves.push_back(mon_move {
          (*find_move_id)->id,
          (*find_move_id)->iden,
          (*find_move_id)->type_id,
          (*find_move_id)->power,
          (*find_move_id)->accuracy,
          (*find_move_id)->priority,
          (*find_move_list)->level
        });

        // printw("%2d: %15s, id: %3d, lv %3d;   ", static_cast<int>(m->all_moves.size()) - 1, str((m->all_moves[static_cast<int>(m->all_moves.size()) - 1]).iden),
        //     (*find_move_list)->move_id, 
        //     m->all_moves[ic(m->all_moves.size()) - 1].level);
        //     halt();
    }

    // if there are moves that are the same but learned at different levels, skip the ones learned at higher levels.
    if ((find_move_list + 1) != (end(pkm_mvs) - 1) && (*(find_move_list + 1))->move_id == (*find_move_list)->move_id) {
      find_move_list++;
    }
  }
  // printw("\n");
  //   halt();

  /* LEVELING MOVESETS*/
    m->moves[0] = new mon_move;
    int a;

  do {
    a = rand() % m->all_moves.size();
    m->moves[0] = &(m->all_moves[a]);

    // printw("a: %d, move 1: %s, lv: %d      ", a, str(m->moves[0]->iden), m->moves[0]->level);
    // halt();
  } while (i++ < 20 && m->moves[0]->level > m->lv);

//  printw("move1: %d, %s\n", m->moves[0]->id, str(m->moves[0]->iden));
//     halt();

i = 0;

    m->moves[1] = new mon_move;
  do {
    a = rand() % m->all_moves.size();
    m->moves[1] = &(m->all_moves[a]);
    // printw("a: %d, move 2: %s, lv: %d     ", a, str(m->moves[1]->iden), m->moves[1]->level);
    // halt();
  } while (i++ < 20 && ((m->all_moves.size() > 1 && (m->all_moves[0].id != m->all_moves[1].id) && (m->moves[0]->id == m->moves[1]->id))
          || m->moves[1]->level > m->lv));

  // printw("move2: %d, %s\n", m->moves[1]->id, str(m->moves[1]->iden));
  // halt();
  // return; 

  // stats
  for (i = 0; i < 6552; i++) {
    if (pkm_sts[i]->pokemon_id == m->id) {
      m->base_stats[0] = pkm_sts[i]->base_stat + (rand() % 16);     // hp
      m->base_stats[1] = pkm_sts[i + 1]->base_stat + (rand() % 16); // atk
      m->base_stats[2] = pkm_sts[i + 2]->base_stat + (rand() % 16); // def
      m->base_stats[3] = pkm_sts[i + 3]->base_stat + (rand() % 16); // spatk
      m->base_stats[4] = pkm_sts[i + 4]->base_stat + (rand() % 16); // spdef
      m->base_stats[5] = pkm_sts[i + 5]->base_stat + (rand() % 16); // spd
      break;
    } 
  }

  m->stats[0] = (m->base_stats[0] * 2 * m->lv) / 100 + m->lv + 10;
  for (i = 1; i < 6; i++) {
    m->stats[i] = (m->base_stats[i] * 2 * m->lv) / 100 + 5;
  } 

  // halt();

  // printw("4\n");
  // halt();

  m->hp = m->stats[0];
  m->gender = (rand() % 2 == 0) ? ("male") : ("female");
  m->shiny = ((rand() % 8192 == 0) ? (true) : (false));

}

void rand_pos(pair_t pos)
{
  pos[dim_x] = (rand() % (MAP_X - 2)) + 1;
  pos[dim_y] = (rand() % (MAP_Y - 2)) + 1;
}

void new_hiker()
{
  pair_t pos;
  npc *c = new npc;
  uint8_t spawn = (rand() % 6) + 1;

  do
  {
    rand_pos(pos);
  } while (world.hiker_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_hiker;
  c->mtype = move_hiker;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->symbol = 'h';
  c->next_turn = 0;
  printw("%d\n", spawn);
  for (int i = 0; i < spawn; i++) {
    mon *m = new mon;
    // printw("hiker: \n");
    // halt();
    spawn_pokemon(m);

    c->pkms.push_back(*m);
    // printw("lv: %d ", m->lv);
    // printw("id: %d", m->id);
    // printw("s: %s ", str(m->iden));
    // printw("arr: %s ", str(c->pkms[i].iden));
  }
  // clear();
  // print_party_list(c);

  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void new_rival()
{
  pair_t pos;
  npc *c = new npc;
  uint8_t spawn = (rand() % 6) + 1;

  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_rival;
  c->mtype = move_rival;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->symbol = 'r';
  c->next_turn = 0;
  // printw("in rival2\n");

  for (int i = 0; i < spawn; i++) {
    mon *m = new mon;
    // printw("rival: \n");
    spawn_pokemon(m);
    c->pkms.push_back(*m);
    // printw("lv: %d ", m->lv);
    // printw("id: %d", m->id);
    // printw("s: %s ", str(m->iden));
    // printw("arr: %s ", str(c->pkms[i].iden));
  }

  // clear();
  // print_party_list(c);

  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
 
}

void new_char_other()
{
  pair_t pos;
  npc *c = new npc;
  uint8_t spawn = (rand() % 6) + 1;
  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

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

  for (int i = 0; i < spawn; i++) {
    // printw("other:\n");
    mon *m = new mon;
    spawn_pokemon(m);
    c->pkms.push_back(*m);
  }

  // clear();
  // print_party_list(c);

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

  /* init pc for the first time */
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
  // printw("You are in map (%d, %d)\n", world.cur_idx[dim_x], world.cur_idx[dim_y]);
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
  // printw("before character placed");
  place_characters();

  return 0;
}

/**
 * @brief where you get to choose a new pokemon
 * 
 */
void oaks_lab() 
{
  int i, j;
  int ch;
  int cur = 5;

  pokemon_t *pkm1 = pkm[rand() % 1092];
  pokemon_t *pkm2, *pkm3;

  printw("here\n");

  do {
    pkm2 = pkm[rand() % 1092];  
  } while (pkm1->id == pkm2->id);

  do {
      pkm3 = pkm[rand() % 1092];  
    } while (pkm2->id == pkm3->id || pkm1->id == pkm3->id);

  clear();

  print:

  for (j = 0; j < MAP_Y; j++) 
  {
    for (i = 0; i < MAP_X; i++) 
    {
        printw("%c", '.');

      if (i == 0 || i == MAP_X - 1 || j == 0 || j == MAP_Y - 1) {
        attron(COLOR_PAIR(ter_mart));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_mart));
      }
      else if (j == 2 && i == 7) {
        printw("%s", "CHOOSE YOUR STARTER");
      }
      else if (j == 4 && i == 4) {
        printw("....");
        mvaddstr(5, 8, const_cast<char *>((pkm1->iden).c_str()));
      }

      else if (j == 5 && i == 4) {
        printw("....");
        mvaddstr(6, 8, const_cast<char *>((pkm2->iden).c_str()));
      }
      else if (j == 6 && i == 4) {
        printw("....");
        mvaddstr(7, 8, const_cast<char *>((pkm3->iden).c_str()));
      }
      else if (j == 8 && i == 4) {
        printw("Press <UP> or <DOWN> to scroll, <ENTER> to select.");
      }
      
    }
    printw("\n");
  }

  keypad(stdscr, TRUE);
  noecho();
  do {
    mvaddch(cur, 5, '>');

    ch = getch();
    if (ch == KEY_DOWN) {
      mvaddch(cur, 5, '.');
      cur = (cur + 1 == 8) ? (5) : (cur + 1);

    }
    else if (ch == KEY_UP) {
      mvaddch(cur, 5, '.');
      cur = (cur - 1 == 4) ? (7) : (cur - 1);
    }
    else if (ch == '\n') {
      // clear();
      mvaddstr(0, 0, "Are you sure? y/n\n");

      do {
        ch = getch();
        if (ch == 'y') {
          goto exit;
        } else if (ch == 'n') {
          goto print;
        }
      } while (1);
      
    }
    mvaddch(cur, 5, '>');
  } while (1);

  exit:

  switch (cur) {
    case 5: 
      world.pc_t.pkms[0].id = pkm1->id;
      world.pc_t.pkms[0].iden = pkm1->iden;
      break;
    case 6:
      world.pc_t.pkms[0].id = pkm2->id;
      world.pc_t.pkms[0].iden = pkm2->iden;
      break;
    case 7:
      world.pc_t.pkms[0].id = pkm3->id;
      world.pc_t.pkms[0].iden = pkm3->iden;
      break;
  }

  world.pc_t.i = new inv;
  world.pc_t.i->balls = world.pc_t.i->potions = world.pc_t.i->revives = 5;
  world.pc_t.money = 1000;

  world.pc_t.pkms[0].lv = 3;
  spawn_pokemon(&world.pc_t.pkms[0]);

  clear();
  printw("Welcome to the wonderful world of pokemon!");
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
  world.pc_t.pkms.push_back(mon{});
  new_map(null);
  oaks_lab();
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
    mon *m = new mon;
    spawn_pokemon(m);
    string _shiny = ((!m->shiny) ? ("") : ("SHINY!!!!!11!!! "));

    clear();
    printw("A ");
    mvaddstr(0, 2, const_cast<char *>((_shiny).c_str()));
    printw("wild ");
    mvaddstr(0, 7 + strlen(const_cast<char *>((_shiny).c_str())), const_cast<char *>((m->iden).c_str()));
    printw(" appeared!");
    enc_screen(m);

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
        print_fight_screen((npc*)other);
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

        print_fight_screen((npc *)c);
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
  char ch;

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

// return 0;
  initscr();
  curs_set(0);
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

  // printw("init");
  init_world();
  // return 0;

  noecho();
  
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
        clear();
        if (world.cur_map->map[world.pc_t.pos[dim_y]]
                              [world.pc_t.pos[dim_x]] == ter_center)
        {
          pokecenter();
        }
        else if (world.cur_map->map[world.pc_t.pos[dim_y]]
                                   [world.pc_t.pos[dim_x]] == ter_mart)
        {
          pokemart();
        }
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
    case 'p':
      print_party_list(&world.pc_t);
      break;
    case 'B':
      print_inventory();
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
