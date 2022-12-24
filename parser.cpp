#include <iostream>
#include <iomanip>
#include <string.h>
#include <string>
#include <climits>
#include <fstream>

#include "parser.h"
using namespace std;

 pokemon_t *pkm[1093];
 moves_t *mvs[845];
 pokemon_moves_t *pkm_mvs[528239];
 pokemon_species_t *pkm_spec[899];
 experience_t *exps[601];
 type_names_t *types[22];
 pokemon_stats_t *pkm_sts[6553];
 stats_t *sts[9];
 pokemon_types_t *pkm_types[1676];

string file_type[9] = {"pokemon.csv", "moves.csv", "pokemon_moves.csv",
                       "pokemon_species.csv", "experience.csv", "type_names.csv",
                       "pokemon_stats.csv", "stats.csv", "pokemon_types.csv"};

void print_files(int file_type)
{
  int i = 0;
  switch (file_type)
  {
  case 0:
  {
    while (i <= 5)
    {
      cout << setfill(' ') << setw(5) << pkm[i]->id
           << setfill(' ') << setw(30) << pkm[i]->iden
           << setfill(' ') << setw(5) << pkm[i]->spec_id
           << setfill(' ') << setw(5) << pkm[i]->height
           << setfill(' ') << setw(5) << pkm[i]->weight
           << setfill(' ') << setw(5) << ((pkm[i]->base_exp == INT_MAX) ? (" ") : (to_string(pkm[i]->base_exp)))
           << setfill(' ') << setw(5) << ((pkm[i]->order == INT_MAX) ? (" ") : (to_string(pkm[i]->order)))
           << setfill(' ') << setw(5) << pkm[i]->is_default << endl;
      i++;
    }
    break;
  }
  case 1:
  {
    while (mvs[i])
    {
      cout << setfill(' ') << setw(5) << mvs[i]->id
           << setfill(' ') << setw(35) << mvs[i]->iden
           << setfill(' ') << setw(5) << ((mvs[i]->gen_id == INT_MAX) ? (" ") : (to_string(mvs[i]->gen_id)))
           << setfill(' ') << setw(6) << ((mvs[i]->type_id == INT_MAX) ? (" ") : (to_string(mvs[i]->type_id)))
           << setfill(' ') << setw(5) << ((mvs[i]->power == INT_MAX) ? (" ") : (to_string(mvs[i]->power)))
           << setfill(' ') << setw(5) << ((mvs[i]->pp == INT_MAX) ? (" ") : (to_string(mvs[i]->pp)))
           << setfill(' ') << setw(5) << ((mvs[i]->accuracy == INT_MAX) ? (" ") : (to_string(mvs[i]->accuracy)))
           << setfill(' ') << setw(5) << ((mvs[i]->priority == INT_MAX) ? (" ") : (to_string(mvs[i]->priority)))
           << setfill(' ') << setw(5) << ((mvs[i]->target_id == INT_MAX) ? (" ") : (to_string(mvs[i]->target_id)))
           << setfill(' ') << setw(5) << ((mvs[i]->dmg_cls_id == INT_MAX) ? (" ") : (to_string(mvs[i]->dmg_cls_id)))
           << setfill(' ') << setw(6) << ((mvs[i]->effect_id == INT_MAX) ? (" ") : (to_string(mvs[i]->effect_id)))
           << setfill(' ') << setw(5) << ((mvs[i]->effect_chance == INT_MAX) ? (" ") : (to_string(mvs[i]->effect_chance)))
           << setfill(' ') << setw(5) << ((mvs[i]->cntst_type_id == INT_MAX) ? (" ") : (to_string(mvs[i]->cntst_type_id)))
           << setfill(' ') << setw(5) << ((mvs[i]->cntst_eff_id == INT_MAX) ? (" ") : (to_string(mvs[i]->cntst_eff_id)))
           << setfill(' ') << setw(5) << ((mvs[i]->spr_contest_effect_id == INT_MAX) ? (" ") : (to_string(mvs[i]->spr_contest_effect_id)))
           << endl;
      i++;
    }
    break;
  }
  case 2:
    while (pkm_mvs[i])
    {
      cout << setfill(' ') << setw(5) << pkm_mvs[i]->id
           << setfill(' ') << setw(30) << pkm_mvs[i]->ver_grp_id
           << setfill(' ') << setw(5) << pkm_mvs[i]->move_id
           << setfill(' ') << setw(5) << pkm_mvs[i]->pkm_move_method_id
           << setfill(' ') << setw(5) << pkm_mvs[i]->level
           << setfill(' ') << setw(5) << ((pkm_mvs[i]->order == INT_MAX) ? (" ") : (to_string(pkm_mvs[i]->order)))
           << endl;
      i++;
    }
    break;
  case 3:
    while (pkm_spec[i])
    {
      cout << setfill(' ') << setw(5) << to_string(pkm_spec[i]->id)
           << setfill(' ') << setw(35) << pkm_spec[i]->iden
           << setfill(' ') << setw(5) << ((pkm_spec[i]->gen_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->gen_id)))
           << setfill(' ') << setw(6) << ((pkm_spec[i]->evolves_from_species_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->evolves_from_species_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->evolution_chain_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->evolution_chain_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->color_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->color_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->shape_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->shape_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->habitat_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->habitat_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->gender_rate == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->gender_rate)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->capture_rate == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->capture_rate)))
           << setfill(' ') << setw(6) << ((pkm_spec[i]->base_happiness == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->base_happiness)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->is_baby == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->is_baby)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->hatch_counter == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->hatch_counter)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->has_gender_differences == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->has_gender_differences)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->growth_rate_id == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->growth_rate_id)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->forms_switchable == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->forms_switchable)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->is_legendary == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->is_legendary)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->is_mythical == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->is_mythical)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->order == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->order)))
           << setfill(' ') << setw(5) << ((pkm_spec[i]->conquest_order == INT_MAX) ? (" ") : (to_string(pkm_spec[i]->conquest_order)))
           << endl;
      i++;
    }
    break;
  case 4:
    while (exps[i])
    {
      cout << setfill(' ') << setw(5) << exps[i]->growth_rate_id
           << setfill(' ') << setw(5) << ((exps[i]->level == INT_MAX) ? (" ") : (to_string(exps[i]->level)))
           << setfill(' ') << setw(10) << ((exps[i]->experience == INT_MAX) ? (" ") : (to_string(exps[i]->experience)))
           << endl;
      i++;
    }
    break;
  case 5:
    while (types[i])
    {
      cout << setfill(' ') << setw(5) << types[i]->type_id
           << setfill(' ') << setw(5) << types[i]->local_language_id
           << setfill(' ') << setw(10) << types[i]->name
           << endl;
      i++;
    }
    break;
  case 6:
    while (pkm_sts[i])
    {
      cout << setfill(' ') << setw(6) << ((pkm_sts[i]->pokemon_id == INT_MAX) ? (" ") : (to_string(pkm_sts[i]->pokemon_id)))
           << setfill(' ') << setw(5) << ((pkm_sts[i]->stat_id == INT_MAX) ? (" ") : (to_string(pkm_sts[i]->stat_id)))
           << setfill(' ') << setw(5) << ((pkm_sts[i]->base_stat == INT_MAX) ? (" ") : (to_string(pkm_sts[i]->base_stat)))
           << setfill(' ') << setw(5) << ((pkm_sts[i]->effort == INT_MAX) ? (" ") : (to_string(pkm_sts[i]->effort)))
           << endl;
      i++;
    }
    break;
  case 7:
    while (sts[i])
    {
      cout << setfill(' ') << setw(6) << ((sts[i]->id == INT_MAX) ? (" ") : (to_string(sts[i]->id)))
           << setfill(' ') << setw(5) << ((sts[i]->damage_class_id == INT_MAX) ? (" ") : (to_string(sts[i]->damage_class_id)))
           << setfill(' ') << setw(20) << sts[i]->identifier
           << setfill(' ') << setw(5) << ((sts[i]->is_battle_only == INT_MAX) ? (" ") : (to_string(sts[i]->is_battle_only)))
           << setfill(' ') << setw(5) << ((sts[i]->game_index == INT_MAX) ? (" ") : (to_string(sts[i]->game_index)))
           << endl;
      i++;
    }
    break;
  case 8:
    while (pkm_types[i])
    {
      cout << setfill(' ') << setw(6) << pkm_types[i]->pokemon_id
           << setfill(' ') << setw(5) << pkm_types[i]->type_id
           << setfill(' ') << setw(5) << pkm_types[i]->slot
           << endl;
      i++;
    }
    break;
  }
}

void init_files()
{
  string base1 = "/share/cs327/pokedex/pokedex/data/csv/";
  string base2 = (string)getenv("HOME") + "/pokedex/pokedex/data/csv/";
  string file;
  ifstream f;
  string s;
  string d = ",";
  int i = 0;

  /**
   * @brief pokemon.csv =================================================================================
   *
   */
  file = base1 + file_type[0];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[0];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[0] << " would not open." << endl;
    }
  }

  getline(f, s);
  while (getline(f, s))
  {
    pkm[i] = new struct pokemon;
    pkm[i]->id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->iden = s.substr(0, s.find(d));
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->spec_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->height = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->weight = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->base_exp = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->order = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm[i]->is_default = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief moves.csv ==========================================================================================
   * 
   */
  file = base1 + file_type[1];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[1];

    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[1] << " would not open." << endl;
    }
  }
  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    mvs[i] = new struct moves;
    mvs[i]->id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->iden = s.substr(0, s.find(d));
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->gen_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->type_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->power = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->pp = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->accuracy = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->priority = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->target_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->dmg_cls_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->effect_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->effect_chance = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->cntst_type_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->cntst_eff_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    mvs[i]->spr_contest_effect_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    i++;
  }
  f.close();

  /**
   * @brief pokemon moves ====================================================================================================
   *
   */
  file = base1 + file_type[2];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[2];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[2] << " would not open." << endl;
    }
  }
  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    pkm_mvs[i] = new struct pokemon_moves;
    pkm_mvs[i]->id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_mvs[i]->ver_grp_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_mvs[i]->move_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_mvs[i]->pkm_move_method_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_mvs[i]->level = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_mvs[i]->order = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief pokemon species ========================================================================================
   *
   */
  file = base1 + file_type[3];
  cout << file << endl;
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[3];

    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[3] << " would not open." << endl;
    }
  }
  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    pkm_spec[i] = new struct pokemon_species;
    pkm_spec[i]->id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->iden = s.substr(0, s.find(d));
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->gen_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->evolves_from_species_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->evolution_chain_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->color_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->shape_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->habitat_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->gender_rate = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->capture_rate = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->base_happiness = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->is_baby = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->hatch_counter = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->has_gender_differences = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->growth_rate_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->forms_switchable = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->is_legendary = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->is_mythical = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->order = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_spec[i]->conquest_order = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief ecxpericen.csv ========================================================================================
   *
   */
  file = base1 + file_type[4];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[4];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[4] << " would not open." << endl;
    }
  }

  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    exps[i] = new struct experience;
    exps[i]->growth_rate_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    exps[i]->level = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    exps[i]->experience = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief types.csv =================================================================================
   *
   */
  file = base1 + file_type[5];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[5];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[5] << " would not open." << endl;
    }
  }

  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    types[i] = new struct type_names;
    types[i]->type_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    types[i]->local_language_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    if (types[i]->local_language_id != 9)
    {
      continue;
    }
    s = s.substr(s.find(d) + 1, s.length());
    types[i]->name = s.substr(0, s.find(d));
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief pokemon_stats.csv =================================================================================
   *
   */
  file = base1 + file_type[6];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[6];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[6] << " would not open." << endl;
    }
  }

  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    pkm_sts[i] = new struct pokemon_stats;
    pkm_sts[i]->pokemon_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_sts[i]->stat_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_sts[i]->base_stat = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_sts[i]->effort = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief stats.csv =================================================================================
   *
   */
  file = base1 + file_type[7];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[7];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[7] << " would not open." << endl;
    }
  }

  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    sts[i] = new struct stats;
    sts[i]->id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    sts[i]->damage_class_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    sts[i]->identifier = s.substr(0, s.find(d));
    s = s.substr(s.find(d) + 1, s.length());
    sts[i]->is_battle_only = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    sts[i]->game_index = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    i++;
  }
  f.close();
  f.clear();

  /**
   * @brief types.csv =================================================================================
   *
   */
  file = base1 + file_type[8];
  f.open(file);
  if (f.fail())
  {
    file = base2 + file_type[8];
    f.open(file);
    if (f.fail())
    {
      cout << "well shit, " << file_type[8] << " would not open." << endl;
    }
  }

  i = 0;
  getline(f, s);
  while (getline(f, s))
  {
    pkm_types[i] = new struct pokemon_types;
    pkm_types[i]->pokemon_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_types[i]->type_id = ((s.substr(0, s.find(d)).length()) != 0) ? (stoi(s.substr(0, s.find(d)))) : (INT_MAX);
    s = s.substr(s.find(d) + 1, s.length());
    pkm_types[i]->slot = stoi(s.substr(0, s.find(d)));
    i++;
  }
  f.close();
  f.clear();
}