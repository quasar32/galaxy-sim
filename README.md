# Galaxy Simulator

Multi-threaded barnes-hut simulation written in pure C using SDL3 and OpenGL for rendering.
Use `git clone https://github.com/quasar32/galaxy-sim --recursive` to clone repository.

## Build

Use `make` with the name of the program you wish to create.
If no name is provided will create `bin/test`.

## Run

All simulations work using standard input.
Pipe in sample galaxies from `txt`.

## `bin/test`

`bin/test` outputs the time it takes to simulate 10
seconds of simulation.

## `bin/vid`

Outputs 10 seconds of simulation as a video. If video path
not provided, default is `vid.mp4`.

## `bin/wnd`
`bin/wnd` opens window with a continuous simulation.
