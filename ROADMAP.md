Possible features to be implemented, not a fully accurate simulation but "good enough":

- [ ] Data driven particle system, particles have following properties, with a universal size:
  - [ ] Boiling point
  - [ ] Melting point
  - [ ] Density (at 0C) - maybe some curve to interpolate between temperatures idk
  - [ ] Colours/visual properties for respective states
    - [ ] Colour
    - [ ] Transparencies
  - [ x ] Name
  - [ x ] Mass for individual particle
  - [ ] Solubility
  - [ ] Flammable
  - [ ] Ignition point
- [ ]  Drawing for inserting new particles in the UI:
  - [ ]  Squares/rectangles
  - [ ]  Circles
  - [ ]  Triangles
  - [ ]  Lines
- [ ]  Saving/loading simulations
- [ ]  Selecting different particles in the UI  
- [ ]  Explosions
- [ ]  Using multithreading to simulate different parts of the screen at once
- [ ]  Break solids if hit by a gas particle with enough kinetic energy
- [ ]  Gas particles disappear if not collided with another particle in a defined time.
- [ ]  Controlling simulation speed
- [ ]  

# Requirements

- CMake
- IWYU
- GCC >= 16.1.1