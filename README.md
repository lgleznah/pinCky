# PinCky

C implementation of the [Pinky programming language](https://pinky-lang.org/), done for Gustavo Pezzi's compilers course

# How to build

The build process has been tested on a WSL Ubuntu 24.04, and the output of `uname -a` is `Linux DESKTOP-BBS5DPR 6.6.87.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun  5 18:30:46 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux`

Make sure you have both `make` and `gcc` installed (either with `apt` or with the package manager of your distribution).

Go to the root of the repository and write in a terminal `make`.

# How to run

Go to the `bin` subfolder after compiling, and write in a terminal `pinky <path_of_script>`. There are some example scripts under `scripts`.

For example, both `mandelbrot.pinky` and `pikumandel.pinky` output a render of the Mandelbrot set. `dragon.pinky` generates code for drawing a Dragon curve, which can be run at [easylang](https://easylang.online/show/).
