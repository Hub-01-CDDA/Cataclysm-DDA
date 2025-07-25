#pragma once
#ifndef CATA_SRC_EXPLOSION_H
#define CATA_SRC_EXPLOSION_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "coordinates.h"
#include "point.h"
#include "type_id.h"

class Creature;
class JsonObject;
class map;
class nc_color;

struct shrapnel_data {
    int casing_mass = 0;
    float fragment_mass = 0.005f;
    // Percentage
    int recovery        = 0;
    itype_id drop       = itype_id::NULL_ID();

    shrapnel_data() = default;
    explicit shrapnel_data( int casing_mass, float fragment_mass = 0.005f, int recovery = 0,
                            itype_id drop = itype_id::NULL_ID() )
        : casing_mass( casing_mass )
        , fragment_mass( fragment_mass )
        , recovery( recovery )
        , drop( drop ) {
    }

    bool was_loaded = false;
    void deserialize( const JsonObject &jo );
};

struct explosion_data {
    float power             = -1.0f;
    float distance_factor   = 0.8f;
    int max_noise           = 90000000; //Noise generated by an exploding mininuke
    bool fire               = false;
    shrapnel_data shrapnel;

    /** Returns the distance at which we have `ratio` of initial power. */
    float expected_range( float ratio ) const;
    /** Returns the expected power at a given distance from epicenter. */
    float power_at_range( float dist ) const;
    /** Returns the distance at which the power drops below 1. */
    int safe_range() const;

    explosion_data() = default;
    explicit explosion_data( float power, float distance_factor = 0.8f, bool fire = false,
                             shrapnel_data shrapnel = {} )
        : power( power )
        , distance_factor( distance_factor )
        , fire( fire )
        , shrapnel( shrapnel ) {
    }

    bool was_loaded = false;
    void deserialize( const JsonObject &jo );
};

// handles explosion related functions
namespace explosion_handler
{
struct queued_explosion {
    const Creature *source;
    const tripoint_abs_ms pos;
    const explosion_data data;

    queued_explosion( const Creature *source, const tripoint_abs_ms &pos, const explosion_data &data )
        : source( source ), pos( pos ), data( data ) {}
};
inline std::vector<queued_explosion> _explosions;

/** Queue an explosion at p of intensity (power) with (shrapnel) chunks of shrapnel.
    Explosion intensity formula is roughly power*factor^distance.
    If factor <= 0, no blast is produced
    The explosion won't actually occur until process_explosions() */
void explosion(
    const Creature *source, const tripoint_bub_ms &p, float power, float factor = 0.8f,
    bool fire = false, int casing_mass = 0, float frag_mass = 0.05
);

// Explosion processing is loading a map on which to execute the explosion. Processing that
// would potentially set off additional explosions should not be performed. They should wait
// until triggered normally.
bool explosion_processing_active();
void explosion( const Creature *source, const tripoint_bub_ms &p, const explosion_data &ex );
void explosion( const Creature *source, map *here, const tripoint_bub_ms &p,
                const explosion_data &ex );
void _make_explosion( map *m, const Creature *source, const tripoint_bub_ms &p,
                      const explosion_data &ex );

/** Triggers a flashbang explosion at p. */
void flashbang( const tripoint_bub_ms &p, bool player_immune = false, int radius = 8 );
/** Triggers a resonance cascade at p. */
void resonance_cascade( const tripoint_bub_ms &p );
/** Triggers a scrambler blast at p. */
void scrambler_blast( const tripoint_bub_ms &p );
/** Triggers an EMP blast at p. */
void emp_blast( const tripoint_bub_ms &p );
// shockwave applies knockback to all targets within radius of p
// parameters force, stun, and dam_mult are passed to knockback()
// ignore_player determines if player is affected, useful for bionic, etc.
void shockwave( const tripoint_bub_ms &p, int radius, int force, int stun, int dam_mult,
                bool ignore_player );

void draw_explosion( const tripoint_bub_ms &p, int radius, const nc_color &col );
void draw_custom_explosion( const std::map<tripoint_bub_ms, nc_color> &area,
                            const std::optional<std::string> &tile_id = std::nullopt );

int ballistic_damage( float velocity, float mass );
float gurney_spherical( double charge, double mass );
void process_explosions();
} // namespace explosion_handler

shrapnel_data load_shrapnel_data( const JsonObject &jo );
explosion_data load_explosion_data( const JsonObject &jo );

#endif // CATA_SRC_EXPLOSION_H
