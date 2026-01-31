# Installation {#installation}

## Compile from source

1. Git clone the repository:

   ```bash
   git clone https://github.com/YanzhaoW/centipede.cpp.git centipede
   ```

2. Go to the project and load all git submodules:

   ```bash
   cd centipede
   git submodule update --init --recursive
   ```

3. Compile and build the project:

   ```bash
   cmake --workflow --preset default
   ```
