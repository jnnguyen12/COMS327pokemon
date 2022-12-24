#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <string>
#include <string.h>

using namespace std;

typedef struct pokemon
{
  int id;
  string iden;
  int spec_id;
  int height;
  int weight;
  int base_exp;
  int order;
  int is_default;
} pokemon_t;

typedef struct moves
{
  int id;
  string iden;
  int gen_id;
  int type_id;
  int power;
  int pp;
  int accuracy;
  int priority;
  int target_id;
  int dmg_cls_id;
  int effect_id;
  int effect_chance;
  int cntst_type_id;
  int cntst_eff_id;
  int spr_contest_effect_id;
} moves_t;

typedef struct pokemon_moves
{
  int id;
  int ver_grp_id;
  int move_id;
  int pkm_move_method_id;
  int level;
  int order;
}pokemon_moves_t;

typedef struct pokemon_species
{
  int id;
  string iden;
  int gen_id;
  int evolves_from_species_id;
  int evolution_chain_id;
  int color_id;
  int shape_id;
  int habitat_id;
  int gender_rate;
  int capture_rate;
  int base_happiness;
  int is_baby;
  int hatch_counter;
  int has_gender_differences;
  int growth_rate_id;
  int forms_switchable;
  int is_legendary;
  int is_mythical;
  int order;
  int conquest_order;
} pokemon_species_t;

typedef struct experience
{
  int growth_rate_id;
  int level;
  int experience;
} experience_t; 

typedef struct type_names
{
  int type_id;
  int local_language_id;
  string name;
} type_names_t;

typedef struct pokemon_stats
{
  int pokemon_id;
  int stat_id;
  int base_stat;
  int effort;
} pokemon_stats_t;

typedef struct stats
{
  int id;
  int damage_class_id;
  string identifier;
  int is_battle_only;
  int game_index;
} stats_t;

typedef struct pokemon_types
{
  int pokemon_id;
  int type_id;
  int slot;
} pokemon_types_t;

extern pokemon_t *pkm[1093];
extern moves_t *mvs[845];
extern pokemon_moves_t *pkm_mvs[528239];
extern pokemon_species_t *pkm_spec[899];
extern experience_t *exps[601];
extern type_names_t *types[22];
extern pokemon_stats_t *pkm_sts[6553];
extern stats_t *sts[9];
extern pokemon_types_t *pkm_types[1676];

void print_files(int file_type);
void init_files();
#endif