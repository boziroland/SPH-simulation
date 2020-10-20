#ifndef SPHSIM_CONSTANTS_H
#define SPHSIM_CONSTANTS_H

using Vec2f = sf::Vector2f;

const float PI = 3.14159265359f;
const Vec2f gravity = Vec2f{0.0f, 15'000.0f * 9.81f};

const float default_fluid_density = 1250.0f;
const float background_pressure = 1750.0f;

const float viscosity = 600.0f;

const int particle_rows_amount = 25;
const int particle_cols_amount = 10;

const float particle_radius = 10.0f;
const float mass_const = 100.0f;
const float effect_radius = 25.0f;

#endif //SPHSIM_CONSTANTS_H