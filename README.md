# helix-solver

# Old project structure and README
All the files of helix-solver present before pl-new-project-structure have been moved to the `old` directory. Project's components will be gradually moved/copied to the root directory with changes neccessary to make them compliant with the new structure, build system, and conventions.

# Documentation
Documetnation is placed in `docs` directory.


# setup dev environment
* Make new empty dir here (any name)
* copy Dockerfile into it
* build docker image: `docker build .`
* tag this image with `docker tag  image-hash helix-solver-docker` where image hash is this long id printed at the end of `docker build` command: someth like: 2833a458019508f55441adae07357857a6b45fa6445d617a356f7c71472deac0
* go to the main dir of the repository and run the image: `docker run --gpus all --rm -it -v.:/code -v /usr/local/cuda-12.1:/cuda helix-solver-docker`
* at this point you should have root console like looking like: `root@ec333231c56e:/helix# `
* setup ROOT: `source lib/root/bin/thisroot.sh`
* make build dir: `mkdir build` and change to it
* build the app: `cmake /code/` and `make`
* obtain `spacepoints.root` file (e.g. scp it)
* edit `/code/config.json` to point to it and run teh code: `./application/HelixSolver/HelixSolver /code/config.json`
