# Walkthrough World - README
Created by:
- Matthew Salaciak
- Jeremy Gaudet
- Elsa Donovan

## Software Requirements

* We're using assimp to load objects and their data into the program, and it is therefore necessary to have assimp installed to run correctly. On a mac, this can be done using the command "brew install assimp".
* For Windows, the download page can be accessed [here](http://www.assimp.org/index.php/downloads).

## Graphical User Interface Panel

* active/deactivate using TAB
* *Terrain slider*: quickly alter the height and topography of the terrain
* *X, Y, Z sliders*: quickly change the position of the light source
* Alternate between one of the following terrain rendering options:
    1. *Jagged* - Default.
    2. *Smooth* - Terrain is smooth, from having no added 'freq' calculation
    3. *Block* - Terrain is cut into squares, from having a constant for the height
* *Flat shading*: Disables the interpolation that is done by the fragment shader
* *Textures*: turn textures on/off

## Controls

### Camera Movement
| Button | Effect |
| -------- | -----: |
| a     |       camera move left |
| d     |       camera move right |
| s     |       camera move backwards |
| w    |        camera move forwards |
| z     |       camera move downwards |
| x     | camera move upwards |
| mouse cursor | change camera direction (yaw & pitch) |
| trackpad & mouse wheel | zoom in/out |

### Sunlight Movement
| Button | Effect |
| -------- | -----: |
| 1 | move sunlight down along Y-axis |
| 2 | move sunlight up along Y-axis |
| 3 | move sunlight along positive X-axis |
| 4 | move sunlight along negative X-axis |
| 5 | move sunlight along positive Y-axis |
| 6 | move sunlight along negative Y-axis |
| 7 | move sunlight along posiitve Z-axis |
| 8 | move sunlight along negative Z-axis |

### Rendering
| Button | Effect |
| -------- | -----: |
| t | render the scene using triangles |
| p | render the scene using points |
| l | render the scene using lines |

### World
| Button | Effect |
| -------- | -----: |
| up arrow | move terrain upwards |
| down arrow | move terrain downwards |

### Toggles
| Button | Effect |
| -------- | -----: |
| q | flat shading on/off |
| b | textures on/off |
| m | camera view between first & third person |
| tab | display GUI controls on/off |

