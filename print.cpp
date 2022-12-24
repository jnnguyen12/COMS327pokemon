#include "poke327.h"

WINDOW *sub;
WINDOW *name;

void print_map()
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

void pokemart()
{
  uint8_t i, j;
  int min_cpx = 4, max_cpx = 54;
  int cur_y = 3, cur_x = min_cpx;
  pc *u = &world.pc_t;
  const char *temp;
  int ch;

  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      // t = 0;
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_center));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_center));
      }
    }
  }
  

  mvprintw(5, 10, "This is the cashier");
  refresh();

  WINDOW *bb = newwin(3, 20, 3, 60);
  sub = newwin(7, 74, 13, 3);
  wrefresh(sub);
  wrefresh(bb);

shop:

  make_box();
  box(bb, 0, 0);
  mvwprintw(sub, 1, 3, "Welcome to Pokémart! How may I help you?");

  mvwprintw(sub, 3, 6, "Pokéballs");
  mvwprintw(sub, 3, 31, "Potions");
  mvwprintw(sub, 3, 56, "Revives");

  mvwprintw(sub, 5, 8, "$200");
  mvwprintw(sub, 5, 32, "$300");
  mvwprintw(sub, 5, 57, "$1000");

  mvwprintw(bb, 1, 3, "Balance: $%d", u->money);

  do
  {
    mvwaddch(sub, cur_y, cur_x, '>');
    wrefresh(sub);
    wrefresh(bb);

    ch = getch();
    switch (ch)
    {
    case KEY_LEFT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = max(min_cpx, cur_x - 25);
      break;
    case KEY_RIGHT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = min(max_cpx, cur_x + 25);
      break;
    case '\n':
      switch (cur_x)
      {
      case 4:
        temp = "Pokéballs";
        break;
      case 29:
        temp = "potions";
        break;
      case 54:
        temp = "revives";
        break;
      }
      make_box();
      mvwprintw(sub, 1, 3, "How many %s?", temp);
      i = 1;
      do
      {
        mvwprintw(sub, 3, 35, "%d", i);
        wrefresh(sub);
        ch = getch();
        if (ch == KEY_UP)
        {
          i = min(i + 1, 100);
        }
        else if (ch == KEY_DOWN)
        {
          i = max(i - 1, 0);
        }
      } while (ch != '\n');
      make_box();
      mvwprintw(sub, 3, 4, "Confirm %d %s? y/n", i, temp);
      do
      {
        wrefresh(sub);
        ch = getch();
        if (ch == 'y')
        {
          switch (cur_x)
          {
          case 4:
            if (u->money - i * 200 < 0) goto error;
            else u->i->balls += i;
            break;
          case 29:
            if (u->money - i * 300 < 0) goto error;
            else u->i->potions += i;
            break;
          case 54:
            if (u->money - i * 1000 < 0) goto error;
            else u->i->potions += i;
            break;
          }
          make_box();
          mvwprintw(sub, 3, 4, "Thank you!");
          wrefresh(sub);
          halt();
          continue;

          error:
          make_box();
          mvwprintw(sub, 3, 4, "You don't have enough money!");
          wrefresh(sub);
          halt();
        }
      } while (ch != 'n' && ch != 'y');
      goto shop;
      break;
    }

  } while (ch != 27 && ch != '<');

  make_box();
  mvwprintw(sub, 3, 4, "Come again!");
  wrefresh(sub);
  halt();
  return;
}

void pokecenter()
{
  int ch;
  uint8_t i, j;
  pc *u = &world.pc_t;

  if (world.cur_map->map[u->pos[dim_y]][u->pos[dim_x]] != ter_center)
  {
    world.cur_map->cmap[u->pos[dim_y]][u->pos[dim_x]] = NULL;
    while (world.cur_map->map[u->pos[dim_y]][u->pos[dim_x]] != ter_center)
    {
      u->pos[dim_x] = rand() % (MAP_X - 2) + 1;
      u->pos[dim_y] = rand() % (MAP_Y - 2) + 1;
    }
    world.cur_map->cmap[u->pos[dim_y]][u->pos[dim_x]] = u;
  }

  clear();
  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      // t = 0;
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_center));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_center));
      }
    }
  }
  refresh();
  sub = newwin(7, 74, 13, 3);

  wclear(sub);
  box(sub, 0, 0);
  mvwprintw(sub, 3, 4, "Welcome to the Pokémon Center.");
  wrefresh(sub);
  halt();

  mvwprintw(sub, 4, 4, "Would you to rest your Pokémons? y/n");
  wrefresh(sub);

  do
  {
    ch = getch();
    if (ch == 'y')
    {
      for (i = 0; i < u->pkms.size(); i++)
      {
        u->pkms[i].hp = u->pkms[i].stats[0];
      }
      make_box();
      mvwprintw(sub, 3, 4, "We have restored your Pokémons to full health!");
      wrefresh(sub);
      halt();
    }
  } while (ch != 'y' && ch != 'n' && ch != '<');

  // for (j = 1; j < 3; j++)
  // {
  //   for (i = 0; i < 8; i++)
  //   {
  //     if (world.cur_map->map[y + all_dirs[i][dim_y] * j][x + all_dirs[i][dim_x] * j] == ter_path)
  //     {
  //       world.cur_map->cmap[u->pos[dim_y]][u->pos[dim_x]] = NULL;

  //       u->pos[dim_y] = y + all_dirs[i][dim_y] * j;
  //       u->pos[dim_x] = x + all_dirs[i][dim_x] * j;

  //       world.cur_map->cmap[u->pos[dim_y]][u->pos[dim_x]] = u;
  //       break;
  //     }
  //   }
  // }

  return;
}

void print_names(mon *cur, mon *m)
{
  int indent = MAP_X - 12 - m->iden.length() - m->gender.length();
  wclear(name);

  mvwprintw(name, 0, indent, "%s (%s)", str(m->iden), str(m->gender));
  mvwprintw(name, 1, indent, "Lv: %d", m->lv);
  mvwprintw(name, 2, indent, "HP: %d/ %d", m->hp, m->stats[0]);
  (m->shiny) && mvwprintw(name, 3, indent, "SHINY");

  mvwprintw(name, 4, 5, "%s (%s)", str(cur->iden), str(cur->gender));
  mvwprintw(name, 5, 5, "Lv: %d", cur->lv);
  mvwprintw(name, 6, 5, "HP: %d/ %d", cur->hp, cur->stats[0]);
  (cur->shiny) && mvwprintw(name, 7, 5, "SHINY");

  wrefresh(name);
}

void print_battle(mon *cur, npc *t, mon *m)
{
  int i, j = 0;

  int indent = MAP_X - 12 - m->iden.length() - m->gender.length();
  wclear(name);

  for (i = 0; i < ic(t->pkms.size()); i++)
  {
    if (t->pkms[i].hp)
    {
      mvwprintw(name, 0, indent + 3 * j, " * ");
      j++;
    }
  }
  mvwprintw(name, 1, indent, "%s (%s)", str(m->iden), str(m->gender));
  mvwprintw(name, 2, indent, "Lv: %d", m->lv);
  mvwprintw(name, 3, indent, "HP: %d/ %d", m->hp, m->stats[0]);
  (m->shiny) && mvwprintw(name, 4, indent, "SHINY");

  mvwprintw(name, 5, 5, "%s (%s)", str(cur->iden), str(cur->gender));
  mvwprintw(name, 6, 5, "Lv: %d", cur->lv);
  mvwprintw(name, 7, 5, "HP: %d/ %d", cur->hp, cur->stats[0]);
  (cur->shiny) && mvwprintw(name, 8, 5, "SHINY");

  wrefresh(name);
}

uint8_t battle(int action, mon **cur, npc *t, mon **m)
{
  int i, j;
  int ch;
  int min_cpy, max_cpy, min_cpx, max_cpx;
  uint8_t cur_y, cur_x;
  pc *u = &world.pc_t;
  mon_move cur_move, _cur_move;
  uint8_t g; // 0 if pc goes first, 1 if npc goes first
  int pr;    // priority
  const char *temp;
  int fight; // any for fight, 0 for switching out pkm

  wclear(sub);
  box(sub, 0, 0);

  noecho();
  keypad(stdscr, TRUE);

  fight = rand() % 20;

  if (!action)
  {
    min_cpx = 13, max_cpx = 43;
    min_cpy = 4;
    cur_x = min_cpx;
    cur_y = min_cpy;

    mvwaddstr(sub, 1, 3, "Choose a move:");
    mvwprintw(sub, 4, 15, "%s (%d)", str((*cur)->moves[0]->iden), (((*cur)->moves[0]->power < INT_MAX) ? (*cur)->moves[0]->power : (0)));
    mvwprintw(sub, 4, 45, "%s (%d)", str((*cur)->moves[1]->iden), (((*cur)->moves[1]->power < INT_MAX) ? (*cur)->moves[1]->power : (0)));

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 30);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(max_cpx, cur_x + 30);
        break;
      case '\n':
        cur_move = ((cur_x == min_cpx) ? (*(*cur)->moves[0]) : (*(*cur)->moves[1]));
        pr = cur_move.priority;
        break;
      case 27:
        return 1;
      }
    } while (ch != '\n');
  }

  else if (action == 1)
  {
  bag:
    wclear(sub);
    box(sub, 0, 0);
    min_cpx = 4, max_cpx = 54;
    min_cpy = 4;
    cur_x = min_cpx;
    cur_y = min_cpy;

    mvwaddstr(sub, 1, 3, "Items:");
    mvwprintw(sub, 4, 6, "Pokéballs : %d", u->i->balls);
    mvwprintw(sub, 4, 31, "Potions : %d", u->i->potions);
    mvwprintw(sub, 4, 56, "Revives : %d", u->i->revives);

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 25);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(max_cpx, cur_x + 25);
        break;
      case 27:
        return 1;
      case '\n':
        if (cur_x == 4)
        {
          wclear(sub);
          box(sub, 0, 0);
          mvwprintw(sub, 3, 4, "You can't catch a trainer's pokemon!");
          wrefresh(sub);
          halt();
          goto bag;
        }
        else if (cur_x == 29 && u->i->potions == 0)
        {
          temp = "potions";
          goto error;
        }
        else if (cur_x == 54 && u->i->revives == 0)
        {
          temp = "revives";
          goto error;
        }

        temp = (cur_x == 29) ? ("potions") : ("revives");

        wclear(sub);
        box(sub, 0, 0);
        mvwprintw(sub, 1, 3, "Use %s on which pokemon?", temp);

        min_cpy = 3, max_cpy = 5;

        mvwprintw(sub, 3, 6, "%s (%d/ %d)", str(u->pkms[0].iden), u->pkms[0].hp, u->pkms[0].stats[0]);
        if (u->pkms.size() > 1)
        {
          mvwprintw(sub, 3, 31, "%s (%d/ %d)", str(u->pkms[1].iden), u->pkms[1].hp, u->pkms[1].stats[0]);
        }
        if (u->pkms.size() > 2)
        {
          mvwprintw(sub, 3, 56, "%s (%d/ %d)", str(u->pkms[2].iden), u->pkms[2].hp, u->pkms[2].stats[0]);
        }
        if (u->pkms.size() > 3)
        {
          mvwprintw(sub, 5, 6, "%s (%d/ %d)", str(u->pkms[3].iden), u->pkms[3].hp, u->pkms[3].stats[0]);
        }
        if (u->pkms.size() > 4)
        {
          mvwprintw(sub, 5, 31, "%s (%d/ %d)", str(u->pkms[4].iden), u->pkms[4].hp, u->pkms[4].stats[0]);
        }
        if (u->pkms.size() > 5)
        {
          mvwprintw(sub, 5, 56, "%s (%d/ %d)", str(u->pkms[5].iden), u->pkms[5].hp, u->pkms[5].stats[0]);
        }

        cur_x = min_cpx;
        cur_y = min_cpy;

        do
        {
          mvwaddch(sub, cur_y, cur_x, '>');
          wrefresh(sub);

          ch = getch();
          switch (ch)
          {
          case KEY_UP:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_y = min_cpy;
            break;
          case KEY_DOWN:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_y = u->pkms.size() < 4 ? min_cpy : max_cpy;
            break;
          case KEY_LEFT:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_x = max(min_cpx, cur_x - 25);
            break;
          case KEY_RIGHT:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_x = min(ic((((u->pkms.size() - 1) % 3) % 25)) * 25 + 4, cur_x + 25);
            break;
          case '\n':

            if (!strcmp(temp, "potions"))
            {
              if (u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp == u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0])
              {
                wclear(sub);
                box(sub, 0, 0);
                mvwprintw(sub, 3, 4, "%s is in full health!",
                          str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
                wrefresh(sub);
                halt();
              }
              else
              {
                u->i->potions--;
                pr = 200;
                goto out;
              }
            }
            else if (!strcmp(temp, "revives"))
            {
              if ((*cur)->hp != 0)
              {
                wclear(sub);
                box(sub, 0, 0);
                mvwprintw(sub, 3, 4, "%s does not need a revive.", str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
                wrefresh(sub);
                halt();
              }
              else
              {
                u->i->revives--;
                pr = 200;
                goto out;
              }
            }
            break;
          case 27:
            goto bag;
          }

        } while (ch != '\n');

        break;
      error:
        wclear(sub);
        box(sub, 0, 0);
        mvwprintw(sub, 3, 4, "There are no %s left!", temp);
        wrefresh(sub);
        halt();
        goto bag;
      }
    } while (ch != '\n');
  }
  else if (action == 2)
  {
  pokemons:
    wclear(sub);
    box(sub, 0, 0);
    min_cpx = 2, max_cpx = 52;

    mvwaddstr(sub, 1, 3, "Choose a Pokémon:");
    min_cpy = 3, max_cpy = 5;

    mvwprintw(sub, 3, min_cpx + 2, "%s (%d/ %d)", str(u->pkms[0].iden), u->pkms[0].hp, u->pkms[0].stats[0]);
    if (u->pkms.size() > 1)
    {
      mvwprintw(sub, 3, min_cpx + 27, "%s (%d/ %d)", str(u->pkms[1].iden), u->pkms[1].hp, u->pkms[1].stats[0]);
    }
    if (u->pkms.size() > 2)
    {
      mvwprintw(sub, 3, max_cpx + 2, "%s (%d/ %d)", str(u->pkms[2].iden), u->pkms[2].hp, u->pkms[2].stats[0]);
    }
    if (u->pkms.size() > 3)
    {
      mvwprintw(sub, 5, min_cpx + 2, "%s (%d/ %d)", str(u->pkms[3].iden), u->pkms[3].hp, u->pkms[3].stats[0]);
    }
    if (u->pkms.size() > 4)
    {
      mvwprintw(sub, 5, min_cpx + 27, "%s (%d/ %d)", str(u->pkms[4].iden), u->pkms[4].hp, u->pkms[4].stats[0]);
    }
    if (u->pkms.size() > 5)
    {
      mvwprintw(sub, 5, max_cpx + 2, "%s (%d/ %d)", str(u->pkms[5].iden), u->pkms[5].hp, u->pkms[5].stats[0]);
    }

    cur_x = min_cpx;
    cur_y = min_cpy;

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_UP:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_y = min_cpy;
        break;
      case KEY_DOWN:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_y = u->pkms.size() < 4 ? min_cpy : max_cpy;
        break;
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 25);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(ic((((u->pkms.size() - 1) % 3) % 25)) * 25 + 2, cur_x + 25);
        break;
      case '\n':
        wclear(sub);
        box(sub, 0, 0);
        if (&u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)] == *cur)
        {
          mvwprintw(sub, 3, 4, "%s is already in battle!", str((*cur)->iden));
        }
        else if (u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp == 0)
        {
          mvwprintw(sub, 3, 4, "%s can't battle!", str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
        }
        else
        {
          *cur = &u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)];
          goto out;
        }
        wrefresh(sub);
        halt();
        break;
      case 27:
        return 1;
      }
    } while (1);
  }

out:
  /**
   * @brief PRIORITIES
   *
   */
  if (pr == (fight ? _cur_move.priority : 100))
  {
    ((*cur)->stats[5] == (*m)->stats[5]) ? (g = rand() % 2) : (g = ((*cur)->stats[5] < (*m)->stats[5] ? 1 : 0));
  }
  else
  {
    (pr < (fight ? _cur_move.priority : 100)) ? (g = 1) : (g = 0);
  }

  /**
   * @brief BATTLE LOOP
   *
   */
  for (i = 0; i < 2; i++)
  {
    wclear(sub);
    box(sub, 0, 0);
    wrefresh(sub);

    if (g == 0)
    {
      switch (action)
      {
      case 0:
        if (cur_move.power < INT_MAX)
        {
          if ((rand() % 100) <= cur_move.accuracy)
          {
            (*m)->hp = max(0, (int)((*m)->hp - ((((2 * (*cur)->lv) / 5 + 2) * cur_move.power * ((*cur)->stats[1] / (*m)->stats[2])) / 50 + 2) * ((rand() % 256) < ((*cur)->base_stats[5] / 2) ? 1.5 : 1) * ((rand() % 39 + 217) / (float)255) * (((*cur)->type[0] == cur_move.type || (*cur)->type[1] == cur_move.type) ? (1.5) : 1)));
            mvwprintw(sub, 3, 4, "* %s used %s!", str((*cur)->iden), str(cur_move.iden));
          }
          else
          {
            mvwprintw(sub, 3, 4, "* %s missed!", str((*cur)->iden));
          }
        }
        else
        {
          mvwprintw(sub, 3, 4, "* %s used %s, but nothing happened!", str((*cur)->iden), str(cur_move.iden));
        }
        break;
      case 1:
        if (!strcmp(temp, "potions"))
        {
          u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp = min(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0], u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp + 20);

          mvwprintw(sub, 2, 4, "You used a potion.");
          mvwprintw(sub, 3, 4, "%s's health was restored!",
                    str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
        }
        else if (!strcmp(temp, "revives"))
        {
          u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp = u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp + u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0] / 2;

          mvwprintw(sub, 3, 4, "You used a revive.");
        }
        break;
      case 2:
        mvwprintw(sub, 3, 4, "Go, %s!", str((*cur)->iden));
        break;
      }
      g++;
    }
    else if (g == 1)
    {
      if (fight)
      {
        _cur_move = *(*m)->moves[rand() % 2];
        if (_cur_move.power < INT_MAX)
        {
          if ((rand() % 100) <= _cur_move.accuracy)
          {
            (*cur)->hp = max(0, (int)((*cur)->hp - ((((2 * (*m)->lv) / 5 + 2) * _cur_move.power * ((*m)->stats[1] / (*cur)->stats[2])) / 50 + 2) * ((rand() % 256) < ((*m)->base_stats[5] / 2) ? 1.5 : 1) * ((rand() % 39 + 217) / (float)255) * (((*m)->type[0] == _cur_move.type || (*m)->type[1] == _cur_move.type) ? (1.5) : 1)));

            mvwprintw(sub, 3, 4, "* The opposing %s used %s!", str((*m)->iden), str(_cur_move.iden));
          }
          else
          {
            mvwprintw(sub, 3, 4, "* The opposing %s missed!", str((*m)->iden));
          }
        }
        else
        {
          mvwprintw(sub, 3, 4, "* The opposing %s used %s, but nothing happened!", str((*m)->iden), str(_cur_move.iden));
        }
      }
      else
      {
        *m = &t->pkms[rand() % (ic(t->pkms.size()))];
        mvwprintw(sub, 3, 4, "%s sends out %s!", str(character_name[t->mtype]), str((*m)->iden));
      }
      g--;
    }
    print_battle(*cur, t, *m);
    wrefresh(sub);
    if (!(*m)->hp)
    {
      make_box();
      mvwprintw(sub, 3, 4, "* The opposing %s has fainted!", str((*m)->iden));
      wrefresh(sub);
      halt();

      for (i = 0; i < ic(t->pkms.size()); i++)
      {
        if (t->pkms[i].hp != 0)
        {
          *m = &t->pkms[i];
          make_box();
          mvwprintw(sub, 3, 4, "%s sends out %s!", str(character_name[t->mtype]), str((*m)->iden));
          wrefresh(sub);
          halt();
          return 1;
        }
      }
      i = (*m)->lv + rand() % 100;
      u->money += i;

      for (j = 0; j < 2; j++)
      {
        make_box();
        j == 0 && mvwprintw(sub, 3, 4, "* You defeated %s!", str(character_name[t->mtype]));
        j == 1 && mvwprintw(sub, 3, 4, "* You received $%d.", i);
        wrefresh(sub);
        halt();
      }

      return 0;
    }
    else if (!(*cur)->hp)
    {
      make_box();
      mvwprintw(sub, 3, 4, "* %s has fainted!", str((*m)->iden));
      wrefresh(sub);
      halt();
      if (u->pkms.size() == 1)
      {
        mvwprintw(sub, 4, 4, "You have no Pokémons left!");
        wrefresh(sub);
        halt();
      }
      else if (u->pkms.size() > 1)
      {
        mvwprintw(sub, 4, 4, "Use a different Pokémon? y/n");
        wrefresh(sub);
        do
        {
          ch = getch();
          if (ch == 'y')
          {
            goto pokemons;
          }
        } while (ch != 'n');
      }
      i = (*cur)->lv + rand() % 100;
      u->money = max(0, u->money - i);

      for (j = 0; j < 2; j++)
      {
        make_box();
        j == 0 && mvwprintw(sub, 3, 4, "You lost the battle!");
        mvwprintw(sub, 3, 4, "You lost $%d!", i);
        j == 1 && wrefresh(sub);
        halt();
      }

      pokecenter();
      return 0;
    }
    halt();
  }
  return 1;
}

void print_fight_screen(npc *t)
{
  int i = 0, j;
  int ch;
  int min_cpy = 3, max_cpy = 5, min_cpx = 13, max_cpx = 43;
  uint8_t cur_y = min_cpy, cur_x = min_cpx;
  mon_move _cur_move;
  mon **cur;
  mon *temp1, *temp2 = &t->pkms[0];
  mon **m = &temp2;

  for (i = 0; i < ic(world.pc_t.pkms.size()); i++)
  {
    if (world.pc_t.pkms[i].hp)
    {
      temp1 = &world.pc_t.pkms[i];
      break;
    }
  }
  cur = &temp1;

  if (cur == NULL)
  {
    mvwprintw(sub, 3, 4, "None of your Pokémons can battle!");
    halt();
    pokecenter();
    return;
  }

  clear();
  sub = newwin(7, 74, 13, 3);
  name = newwin(8, 74, 4, 3);

  noecho();
  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      // mvaddch(j + 1, i, '.');
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_grass));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_grass));
      }
    }
    printw("\n");
  }
  refresh();
  print_battle(*cur, t, *m);
  make_box();
  mvwprintw(sub, 3, 4, "%s wants to battle!", str(character_name[t->mtype]));
  wrefresh(sub);
  halt();

/**
 * @brief loop 1: while the battle commmences
 *
 */
loop:
  print_battle(*cur, t, *m);
  make_box();

  mvwprintw(sub, 1, 3, "What will %s do?", str((*cur)->iden));

  mvwaddstr(sub, 3, 15, "Fight");
  mvwaddstr(sub, 3, 45, "Bag");
  mvwaddstr(sub, 5, 15, "Pokemon");
  mvwaddstr(sub, 5, 45, "Run");

  do
  {
    mvwaddch(sub, cur_y, cur_x, '>');
    wrefresh(sub);

    ch = getch();
    switch (ch)
    {
    case KEY_UP:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_y = max(min_cpy, cur_y - 2);
      break;
    case KEY_DOWN:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_y = min(max_cpy, cur_y + 2);
      break;
    case KEY_LEFT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = max(min_cpx, cur_x - 30);
      break;
    case KEY_RIGHT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = min(max_cpx, cur_x + 30);
      break;
    case '\n':
      if (cur_y == min_cpy && cur_x == min_cpx)
      {
        if (!battle(0, cur, t, m))
        {
          return;
        } // fight
      }
      else if (cur_y == min_cpy && cur_x == max_cpx)
      {
        if (!battle(1, cur, t, m))
        {
          return;
        }; // bag
      }
      else if (cur_y == max_cpy && cur_x == min_cpx)
      {
        if (!battle(2, cur, t, m))
        {
          return;
        }; // pokemon
      }
      else if (cur_y == max_cpy && cur_x == max_cpx)
      {
        make_box();
        mvwprintw(sub, 3, 4, "You can't run away from a trainer battle!");
        wrefresh(sub);
        halt();
        // flee
      }
      goto loop;
      break;
    }
  } while (1);

  clear();
  printw("You got $42,069.");
  print_map();
}

uint8_t wild(int action, mon **cur, mon *m, int *attempts)
{
  int i;
  int ch;
  int min_cpy, max_cpy, min_cpx, max_cpx;
  uint8_t cur_y, cur_x;
  pc *u = &world.pc_t;
  // mon *cur = &u->pkms[0];
  mon_move cur_move, _cur_move;
  uint8_t g; // 0 if pc goes first, 1 if npc goes first
  int pr;    // priority
  const char *temp;
  int fight;

  wclear(sub);
  box(sub, 0, 0);

  noecho();
  keypad(stdscr, TRUE);

  fight = rand() % 20;

  if (!action)
  {
    min_cpx = 13, max_cpx = 43;
    min_cpy = 4;
    cur_x = min_cpx;
    cur_y = min_cpy;

    mvwaddstr(sub, 1, 3, "Choose a move:");
    mvwprintw(sub, 4, 15, "%s (%d)", str((*cur)->moves[0]->iden), (((*cur)->moves[0]->power < INT_MAX) ? (*cur)->moves[0]->power : (0)));
    mvwprintw(sub, 4, 45, "%s (%d)", str((*cur)->moves[1]->iden), (((*cur)->moves[1]->power < INT_MAX) ? (*cur)->moves[1]->power : (0)));

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 30);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(max_cpx, cur_x + 30);
        break;
      case '\n':
        cur_move = ((cur_x == min_cpx) ? (*(*cur)->moves[0]) : (*(*cur)->moves[1]));
        pr = cur_move.priority;
        break;
      case 27:
        return 1;
      }
    } while (ch != '\n');
  }

  else if (action == 1)
  {
  bag:
    wclear(sub);
    box(sub, 0, 0);
    min_cpx = 4, max_cpx = 54;
    min_cpy = 4;
    cur_x = min_cpx;
    cur_y = min_cpy;

    mvwaddstr(sub, 1, 3, "Items:");
    mvwprintw(sub, 4, 6, "Pokéballs : %d", u->i->balls);
    mvwprintw(sub, 4, 31, "Potions : %d", u->i->potions);
    mvwprintw(sub, 4, 56, "Revives : %d", u->i->revives);

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 25);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(max_cpx, cur_x + 25);
        break;
      case 27:
        return 1;
      case '\n':
        if (cur_x == 4)
        {
          if (u->i->balls == 0)
          {
            temp = "Pokéballs";
            goto error;
          }
          else if (u->pkms.size() == 6)
          {
            wclear(sub);
            box(sub, 0, 0);
            mvwprintw(sub, 3, 4, "Your Pokémon party is full!");
            wrefresh(sub);
            halt();
            goto bag;
          }
          else
          {
            u->i->balls--;
            pr = 200;
            goto out;
          }
        }
        else if (cur_x == 29 && u->i->potions == 0)
        {
          temp = "potions";
          goto error;
        }
        else if (cur_x == 54 && u->i->revives == 0)
        {
          temp = "revives";
          goto error;
        }

        temp = (cur_x == 29) ? ("potions") : ("revives");

        wclear(sub);
        box(sub, 0, 0);
        mvwprintw(sub, 1, 3, "Use %s on which pokemon?", temp);

        min_cpy = 3, max_cpy = 5;

        mvwprintw(sub, 3, 6, "%s (%d/ %d)", str(u->pkms[0].iden), u->pkms[0].hp, u->pkms[0].stats[0]);
        if (u->pkms.size() > 1)
        {
          mvwprintw(sub, 3, 31, "%s (%d/ %d)", str(u->pkms[1].iden), u->pkms[1].hp, u->pkms[1].stats[0]);
        }
        if (u->pkms.size() > 2)
        {
          mvwprintw(sub, 3, 56, "%s (%d/ %d)", str(u->pkms[2].iden), u->pkms[2].hp, u->pkms[2].stats[0]);
        }
        if (u->pkms.size() > 3)
        {
          mvwprintw(sub, 5, 6, "%s (%d/ %d)", str(u->pkms[3].iden), u->pkms[3].hp, u->pkms[3].stats[0]);
        }
        if (u->pkms.size() > 4)
        {
          mvwprintw(sub, 5, 31, "%s (%d/ %d)", str(u->pkms[4].iden), u->pkms[4].hp, u->pkms[4].stats[0]);
        }
        if (u->pkms.size() > 5)
        {
          mvwprintw(sub, 5, 56, "%s (%d/ %d)", str(u->pkms[5].iden), u->pkms[5].hp, u->pkms[5].stats[0]);
        }

        cur_x = min_cpx;
        cur_y = min_cpy;

        do
        {
          mvwaddch(sub, cur_y, cur_x, '>');
          wrefresh(sub);

          ch = getch();
          switch (ch)
          {
          case KEY_UP:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_y = min_cpy;
            break;
          case KEY_DOWN:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_y = u->pkms.size() < 4 ? min_cpy : max_cpy;
            break;
          case KEY_LEFT:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_x = max(min_cpx, cur_x - 25);
            break;
          case KEY_RIGHT:
            mvwaddch(sub, cur_y, cur_x, ' ');
            cur_x = min(ic((((u->pkms.size() - 1) % 3) % 25)) * 25 + 4, cur_x + 25);
            break;
          case '\n':

            if (!strcmp(temp, "potions"))
            {
              if (u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp == u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0])
              {
                wclear(sub);
                box(sub, 0, 0);
                mvwprintw(sub, 3, 4, "%s is in full health!",
                          str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
                wrefresh(sub);
                halt();
              }
              else
              {
                u->i->potions--;
                pr = 200;
                goto out;
              }
            }
            else if (!strcmp(temp, "revives"))
            {
              if ((*cur)->hp != 0)
              {
                wclear(sub);
                box(sub, 0, 0);
                mvwprintw(sub, 3, 4, "%s does not need a revive.", str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
                wrefresh(sub);
                halt();
              }
              else
              {
                u->i->revives--;
                pr = 200;
                goto out;
              }
            }
            break;
          case 27:
            goto bag;
          }

        } while (ch != '\n');

        break;
      error:
        wclear(sub);
        box(sub, 0, 0);
        mvwprintw(sub, 3, 4, "There are no %s left!", temp);
        wrefresh(sub);
        halt();
        goto bag;
      }
    } while (ch != '\n');
  }
  else if (action == 2)
  {
  pokemons:
    wclear(sub);
    box(sub, 0, 0);
    min_cpx = 4, max_cpx = 54;
    min_cpy = 3, max_cpy = 5;

    mvwaddstr(sub, 1, 3, "Choose a Pokémon:");

    mvwprintw(sub, 3, 6, "%s (%d/ %d)", str(u->pkms[0].iden), u->pkms[0].hp, u->pkms[0].stats[0]);
    if (u->pkms.size() > 1)
    {
      mvwprintw(sub, 3, 31, "%s (%d/ %d)", str(u->pkms[1].iden), u->pkms[1].hp, u->pkms[1].stats[0]);
    }
    if (u->pkms.size() > 2)
    {
      mvwprintw(sub, 3, 56, "%s (%d/ %d)", str(u->pkms[2].iden), u->pkms[2].hp, u->pkms[2].stats[0]);
    }
    if (u->pkms.size() > 3)
    {
      mvwprintw(sub, 5, 6, "%s (%d/ %d)", str(u->pkms[3].iden), u->pkms[3].hp, u->pkms[3].stats[0]);
    }
    if (u->pkms.size() > 4)
    {
      mvwprintw(sub, 5, 31, "%s (%d/ %d)", str(u->pkms[4].iden), u->pkms[4].hp, u->pkms[4].stats[0]);
    }
    if (u->pkms.size() > 5)
    {
      mvwprintw(sub, 5, 56, "%s (%d/ %d)", str(u->pkms[5].iden), u->pkms[5].hp, u->pkms[5].stats[0]);
    }

    cur_x = min_cpx;
    cur_y = min_cpy;

    do
    {
      mvwaddch(sub, cur_y, cur_x, '>');
      wrefresh(sub);

      ch = getch();
      switch (ch)
      {
      case KEY_UP:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_y = min_cpy;
        break;
      case KEY_DOWN:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_y = u->pkms.size() < 4 ? min_cpy : max_cpy;
        break;
      case KEY_LEFT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = max(min_cpx, cur_x - 25);
        break;
      case KEY_RIGHT:
        mvwaddch(sub, cur_y, cur_x, ' ');
        cur_x = min(ic((((u->pkms.size() - 1) % 3) % 25)) * 25 + 4, cur_x + 25);
        break;
      case '\n':
        wclear(sub);
        box(sub, 0, 0);
        if (&u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)] == *cur)
        {
          mvwprintw(sub, 3, 4, "%s is already in battle!", str((*cur)->iden));
        }
        else if (u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp == 0)
        {
          mvwprintw(sub, 3, 4, "%s can't battle!", str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
        }
        else
        {
          *cur = &u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)];
          goto out;
        }
        wrefresh(sub);
        halt();
        break;
      case 27:
        return 1;
      }
    } while (1);
  }
  else if (action == 3)
  {
    // fleeing needs nothing filled other than down there
  }

out:
  /**
   * @brief PRIORITIES
   *
   */
  if (pr == (fight ? _cur_move.priority : 100))
  {
    ((*cur)->stats[5] == m->stats[5]) ? (g = rand() % 2) : (g = ((*cur)->stats[5] < m->stats[5] ? 1 : 0));
  }
  else
  {
    (pr < (fight ? _cur_move.priority : 100)) ? (g = 1) : (g = 0);
  }

  /**
   * @brief BATTLE LOOP
   *
   */
  for (i = 0; i < 2; i++)
  {
    wclear(sub);
    box(sub, 0, 0);
    wrefresh(sub);

    if (g == 0)
    {
      switch (action)
      {
      case 0:
        if (cur_move.power < INT_MAX)
        {
          if ((rand() % 100) <= cur_move.accuracy)
          {
            m->hp = max(0, (int)(m->hp - ((((2 * (*cur)->lv) / 5 + 2) * cur_move.power * ((*cur)->stats[1] / m->stats[2])) / 50 + 2) * ((rand() % 256) < ((*cur)->base_stats[5] / 2) ? 1.5 : 1) * ((rand() % 39 + 217) / (float)255) * (((*cur)->type[0] == cur_move.type || (*cur)->type[1] == cur_move.type) ? (1.5) : 1)));
            mvwprintw(sub, 3, 4, "* %s used %s!", str((*cur)->iden), str(cur_move.iden));
          }
          else
          {
            mvwprintw(sub, 3, 4, "* %s missed!", str((*cur)->iden));
          }
        }
        else
        {
          mvwprintw(sub, 3, 4, "* %s used %s, but nothing happened!", str((*cur)->iden), str(cur_move.iden));
        }
        break;
      case 1:
        if (!strcmp(temp, "potions"))
        {
          u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp = min(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0], u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp + 20);

          mvwprintw(sub, 2, 4, "You used a potion.");
          mvwprintw(sub, 3, 4, "%s's health was restored!",
                    str(u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].iden));
        }
        else if (!strcmp(temp, "revives"))
        {
          u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp = u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].hp + u->pkms[(int)(cur_x / 25) + ((cur_y == min_cpy) ? 0 : 3)].stats[0] / 2;

          mvwprintw(sub, 3, 4, "You used a revive.");
        }
        else
        {
          mvwprintw(sub, 3, 4, "You used PokéBall.");

          wrefresh(sub);
          halt();
          wclear(sub);
          box(sub, 0, 0);

          if (rand() % 100 <= (100 - (m->hp / m->stats[0]) * 100))
          {
            mvwprintw(sub, 3, 4, "Gotcha! %s was caught!", str(m->iden));
            mvwprintw(sub, 4, 4, "*victory music plays*");
            wrefresh(sub);
            halt();
            u->pkms.push_back(*m);
            return 0;
          }
          else
          {
            mvwprintw(sub, 3, 4, "Aww! It appeared to be caught!");
          }
        }

        break;
      case 2:
        mvwprintw(sub, 3, 4, "Go, %s!", str((*cur)->iden));
        break;
      case 3:
        ++(*attempts);
        if ((rand() % 256) < ((*cur)->stats[5] * 32) / (m->stats[5] / 4 % 256) + 30 * (*attempts))
        {
          mvwprintw(sub, 3, 4, "Got away safely!");
          wrefresh(sub);
          halt();
          delete m;
          return 0;
        }
        else
        {
          mvwprintw(sub, 3, 4, "You tried to escape, but couldn't!");
        }
        break;
      }
      g++;
    }
    else if (g == 1)
    {
      if (fight)
      {
        _cur_move = *m->moves[rand() % 2];
        if (_cur_move.power < INT_MAX)
        {
          if ((rand() % 100) <= _cur_move.accuracy)
          {
            (*cur)->hp = max(0, (int)((*cur)->hp - ((((2 * m->lv) / 5 + 2) * _cur_move.power * (m->stats[1] / (*cur)->stats[2])) / 50 + 2) * ((rand() % 256) < (m->base_stats[5] / 2) ? 1.5 : 1) * ((rand() % 39 + 217) / (float)255) * ((m->type[0] == _cur_move.type || m->type[1] == _cur_move.type) ? (1.5) : 1)));

            mvwprintw(sub, 3, 4, "* The wild %s used %s!", str(m->iden), str(_cur_move.iden));
          }
          else
          {
            mvwprintw(sub, 3, 4, "* The wild %s missed!", str(m->iden));
          }
        }
        else
        {
          mvwprintw(sub, 3, 4, "* The wild %s used %s, but nothing happened!", str(m->iden), str(_cur_move.iden));
        }
      }
      else
      {
        mvwprintw(sub, 3, 4, "The wild %s has fled!", str(m->iden));
        wrefresh(sub);
        halt();
        return 0;
      }
      g--;
    }
    print_names(*cur, m);
    wrefresh(sub);
    if (!m->hp)
    {
      for (i = 0; i < 2; i++)
      {
        wclear(sub);
        box(sub, 0, 0);
        (i == 0) && mvwprintw(sub, 3, 4, "* The opposing %s has fainted!", str(m->iden));
        (i == 1) && mvwprintw(sub, 3, 4, "You won!");
        wrefresh(sub);
        halt();
      }
      return 0;
    }
    else if (!(*cur)->hp)
    {
      make_box();
      mvwprintw(sub, 3, 4, "* %s has fainted!", str(m->iden));
      wrefresh(sub);
      halt();
      if (u->pkms.size() == 1)
      {
        mvwprintw(sub, 4, 4, "You have no Pokémons left!");
        wrefresh(sub);
        halt();
      }
      else if (u->pkms.size() > 1)
      {
        mvwprintw(sub, 4, 4, "Use a different Pokémon? y/n");
        wrefresh(sub);
        do
        {
          ch = getch();
          if (ch == 'y')
          {
            goto pokemons;
          }
        } while (ch != 'n');
      }
      make_box();
      mvwprintw(sub, 3, 4, "You lost the battle!");
      wrefresh(sub);
      halt();
      pokecenter();
      return 0;
    }
    halt();
  }
  return 1;
}

void enc_screen(mon *m)
{
  int i = 0, j;
  int ch;
  int min_cpy = 3, max_cpy = 5, min_cpx = 13, max_cpx = 43;
  uint8_t cur_y = min_cpy, cur_x = min_cpx;
  mon_move _cur_move;
  int attempts = 0;
  mon **cur;
  mon *temp;

  for (i = 0; i < ic(world.pc_t.pkms.size()); i++)
  {
    if (world.pc_t.pkms[i].hp)
    {
      temp = &world.pc_t.pkms[i];
      break;
    }
  }

  cur = &temp;

  if (!cur)
  {
    mvwprintw(sub, 3, 4, "None of your Pokémons can battle!");
    halt();
    pokecenter();
    return;
  }

  clear();
  sub = newwin(7, 74, 13, 3);
  name = newwin(7, 74, 4, 3);

  noecho();
  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      // mvaddch(j + 1, i, '.');
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_grass));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_grass));
      }
    }
    printw("\n");
  }
  refresh();

/**
 * @brief loop 1: while the battle commmences
 *
 */
loop:
  print_names((*cur), m);
  wclear(sub);
  box(sub, 0, 0);

  mvwprintw(sub, 1, 3, "What will %s do?", str((*cur)->iden));

  mvwaddstr(sub, 3, 15, "Fight");
  mvwaddstr(sub, 3, 45, "Bag");
  mvwaddstr(sub, 5, 15, "Pokemon");
  mvwaddstr(sub, 5, 45, "Run");

  do
  {
    mvwaddch(sub, cur_y, cur_x, '>');
    wrefresh(sub);

    ch = getch();
    switch (ch)
    {
    case KEY_UP:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_y = max(min_cpy, cur_y - 2);
      break;
    case KEY_DOWN:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_y = min(max_cpy, cur_y + 2);
      break;
    case KEY_LEFT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = max(min_cpx, cur_x - 30);
      break;
    case KEY_RIGHT:
      mvwaddch(sub, cur_y, cur_x, ' ');
      cur_x = min(max_cpx, cur_x + 30);
      break;
    case '\n':
      if (cur_y == min_cpy && cur_x == min_cpx)
      {
        if (!wild(0, cur, m, &attempts))
        {
          return;
        } // fight
      }
      else if (cur_y == min_cpy && cur_x == max_cpx)
      {
        if (!wild(1, cur, m, &attempts))
        {
          return;
        }; // bag
      }
      else if (cur_y == max_cpy && cur_x == min_cpx)
      {
        if (!wild(2, cur, m, &attempts))
        {
          return;
        }; // pokemon
      }
      else if (cur_y == max_cpy && cur_x == max_cpx)
      {
        if (!wild(3, cur, m, &attempts))
        {
          return;
        }; // flee
      }
      goto loop;
      break;
    }
  } while (1);
}

void print_party_list(character *c)
{
  int i = 0, j;
  int ch;
  int page = 0;
  mon *cur_mon;

loop:
  clear();

  cur_mon = &c->pkms[page];

  printw("%s's pokemon %d / %d", str((c == &world.pc_t) ? ("PC") : (character_name[((npc *)c)->mtype])),
         page + 1, static_cast<int>(c->pkms.size()));

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
      else if (j == 2 && i == 4)
      {
        printw("%s", str(cur_mon->iden));
      }
      else if (j == 3 && i == 4)
      {
        printw("Lvl: %3d, HP: %3d/%3d", cur_mon->lv, cur_mon->hp, cur_mon->stats[0]);
      }
      else if (j == 4 && i == 4)
      {
        printw("Moves: ");
        printw(str(cur_mon->moves[0]->iden));
        //   mvaddstr(4, 11, const_cast<char*>(cur_mon->moves[0]->iden.c_str()));
        printw(", ");
        printw(str(cur_mon->moves[1]->iden));
        //   mvaddstr(4, 13 + strlen(const_cast<char*>(cur_mon->moves[0]->iden.c_str())), const_cast<char*>(cur_mon->moves[1]->iden.c_str()));
      }
      else if (j == 5 && i == 4)
      {
        printw("HP: %3d, Atk: %3d, Def: %3d, SpAtk: %3d, SpDef: %3d, Speed: %3d", cur_mon->stats[0], cur_mon->stats[1], cur_mon->stats[2], cur_mon->stats[3], cur_mon->stats[4], cur_mon->stats[5]);
      }
      else if (j == 6 && i == 4)
      {
        printw("Gender: %s", str(cur_mon->gender));
      }
      else if (j == 7 && i == 4)
      {
        printw("Shiny: %s", ((!cur_mon->shiny) ? ("no") : ("yes")));
      }
      else if (j == 9 && i == 4)
      {
        printw("(press <LEFT> or <RIGHT> to go to next party member.)");
      }
      else if (j == 10 && i == 4)
      {
        printw("(press <L> to level up)");
      }
      else
      {
        printw("%c", '.');
      }
    }
    printw("\n");
  }

  noecho();
  keypad(stdscr, TRUE);
  do
  {
    ch = getch();
    if (ch == 'L')
    {
      c->pkms[page].lv += 1;

      cur_mon->stats[0] = (cur_mon->base_stats[0] * 2 * cur_mon->lv) / 100 + cur_mon->lv + 10;
      for (i = 1; i < 6; i++)
      {
        cur_mon->stats[i] = (cur_mon->base_stats[i] * 2 * cur_mon->lv) / 100 + 5;
      }
      // c->pkms[page].hp = cur_mon->stats[0];
      goto loop;
    }
    else if (ch == KEY_LEFT)
    {
      page = (max(0, page - 1));
      goto loop;
    }
    else if (ch == KEY_RIGHT)
    {
      page = (min(static_cast<int>(c->pkms.size()) - 1, page + 1));
      goto loop;
    }
  } while (ch != 27 && ch != '\n');

  clear();
  print_map();
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
  } while (ch != 27 && ch != '\n');

  for (i = 0; i < arr_size; i++)
  {
    heap_insert(&world.cur_map->turn, arr[i]);
  }
  clear();
  printw("Returned to map.\n");
  print_map();
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

void print_inventory()
{
  int i, j;
  int ch;
  uint8_t cur;

loop:
  cur = 7;

  for (j = 0; j < MAP_Y; j++)
  {
    for (i = 0; i < MAP_X; i++)
    {
      printw("%c", '.');
      if (j == 0 || j == MAP_Y - 1 ||
          i == 0 || i == MAP_X - 1)
      {
        attron(COLOR_PAIR(ter_tree));
        mvaddch(j + 1, i, '%');
        attroff(COLOR_PAIR(ter_tree));
      }
      else if (j == 4 && i == 10)
      {
        mvaddstr(5, 10, "pokeBalls: ");
        mvaddstr(5, 22, strint(world.pc_t.i->balls));
      }
      else if (j == 6 && i == 10)
      {
        mvaddstr(7, 10, "potions: ");
        mvaddstr(7, 20, strint(world.pc_t.i->potions));
      }
      else if (j == 8 && i == 10)
      {
        mvaddstr(9, 10, "revives: ");
        mvaddstr(9, 20, strint(world.pc_t.i->revives));
      }
      else if (j == 17 && i == 8)
      {
        mvaddstr(18, 8, "(Press <UP> or <DOWN> to navigate, <ENTER> to use)");
      }
    }
    printw("\n");
  }

  mvaddstr(13, 10, "Balance: ");
  mvaddstr(13, 20, strint(world.pc_t.money));

  keypad(stdscr, TRUE);
  noecho();
  mvaddch(cur, 8, '>');

  do
  {
    ch = getch();
    if (ch == KEY_UP)
    {
      mvaddch(cur, 8, '.');
      cur = max(7, cur - 2);
    }
    else if (ch == KEY_DOWN)
    {
      mvaddch(cur, 8, '.');
      cur = min(9, cur + 2);
    }
    else if (ch == '\n')
    {
      if (cur == 7)
      {
        goto potions;
      }
      else if (cur == 9)
      {
        goto revives;
      }
    }
    else if (ch == 27)
    {
      clear();
      print_map();
      return;
    }
    mvaddch(cur, 8, '>');
  } while (1);

potions:
  move(0, 0);
  clrtoeol();
  printw("Use potion on which pokemon?");
  for (i = 0; i < (int)(world.pc_t.pkms.size()); i++)
  {
    mvaddstr(4 + 2 * i, 50, str(world.pc_t.pkms[i].iden));
    mvaddstr(5 + 2 * i, 50, "HP: ");
    mvaddstr(5 + 2 * i, 55, strint(world.pc_t.pkms[i].hp));
    mvaddstr(5 + 2 * i, 59, "/ ");
    mvaddstr(5 + 2 * i, 61, strint(world.pc_t.pkms[i].stats[0]));
  }

  mvaddch(cur, 8, '.');
  mvaddch(cur = 4, 48, '>');

  do
  {
    ch = getch();
    if (ch == KEY_UP)
    {
      mvaddch(cur, 48, '.');
      cur = max(4, cur - 2);
    }
    else if (ch == KEY_DOWN)
    {
      mvaddch(cur, 48, '.');
      cur = min(4 + 2 * (i - 1), cur + 2);
    }
    else if (ch == '\n')
    {
      move(0, 0);
      if (world.pc_t.pkms[(cur - 4) / 2].hp < world.pc_t.pkms[(cur - 4) / 2].stats[0])
      {
        if (world.pc_t.i->potions == 0)
        {
          clrtoeol();
          printw("There are no potions left!");
          goto potions;
        }
        world.pc_t.i->potions--;

        world.pc_t.pkms[(cur - 4) / 2].hp = min(world.pc_t.pkms[(cur - 4) / 2].stats[0], world.pc_t.pkms[(cur - 4) / 2].hp + 20);

        clrtoeol();
        printw("%s's health has been restored %s.", str(world.pc_t.pkms[(cur - 4) / 2].iden),
               (world.pc_t.pkms[(cur - 4) / 2].hp == world.pc_t.pkms[(cur - 4) / 2].stats[0] ? "" : "by 20"));
        halt();
        goto potions;
      }
      else
      {
        clrtoeol();
        printw("%s's health is full!", str(world.pc_t.pkms[(cur - 4) / 2].iden));
        halt();
        goto potions;
      }
    }
    else if (ch == 27)
    {
      goto loop;
    }
    mvaddch(cur, 48, '>');
  } while (1);

revives:
  move(0, 0);
  clrtoeol();
  printw("Use revive on which pokemon?");
  for (i = 0; i < (int)(world.pc_t.pkms.size()); i++)
  {
    mvaddstr(4 + 2 * i, 50, str(world.pc_t.pkms[i].iden));
    mvaddstr(5 + 2 * i, 50, "HP: ");
    mvaddstr(5 + 2 * i, 55, strint(world.pc_t.pkms[i].hp));
    mvaddstr(5 + 2 * i, 59, "/ ");
    mvaddstr(5 + 2 * i, 61, strint(world.pc_t.pkms[i].stats[0]));
  }

  mvaddch(cur, 8, '.');
  mvaddch(cur = 4, 48, '>');

  do
  {
    ch = getch();
    if (ch == KEY_UP)
    {
      mvaddch(cur, 48, '.');
      cur = max(4, cur - 2);
    }
    else if (ch == KEY_DOWN)
    {
      mvaddch(cur, 48, '.');
      cur = min(4 + 2 * (i - 1), cur + 2);
    }
    else if (ch == '\n')
    {
      move(0, 0);
      if (world.pc_t.pkms[(cur - 4) / 2].hp == 0)
      {
        if (world.pc_t.i->revives == 0)
        {
          clrtoeol();
          printw("There are no revives left!");
          goto revives;
        }

        world.pc_t.i->revives--;

        world.pc_t.pkms[(cur - 4) / 2].hp = world.pc_t.pkms[(cur - 4) / 2].stats[0] / 2;

        clrtoeol();
        printw("%s's health has been revived.", str(world.pc_t.pkms[(cur - 4) / 2].iden));
        halt();
        goto revives;
      }
      else
      {
        clrtoeol();
        printw("%s is healthy!", str(world.pc_t.pkms[(cur - 4) / 2].iden));
        halt();
        goto revives;
      }
    }
    else if (ch == 27)
    {
      goto loop;
    }
    mvaddch(cur, 48, '>');
  } while (1);
}