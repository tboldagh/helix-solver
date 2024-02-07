# helix-solver

# Old project structure and README
All the files of helix-solver present before pl-new-project-structure have been moved to the `old` directory. Project's components will be gradually moved/copied to the root directory with changes neccessary to make them compliant with the new structure, build system, and conventions.

# Documentation
Documetnation is placed in `docs` directory.


# Setup dev environment
* Build docker image with tag helix-solver-docker: `docker build -t helix-solver-docker .`
* Run the image in interactive mode: `docker run --gpus all --rm -it -v.:/helix/repo -v /usr/local/cuda-12.1:/cuda helix-solver-docker`
* At this point you should have root console like looking like: `root@ec333231c56e:/helix/repo# `
* Prepare environment: `source prepare_environment.sh`
* Build the app: `./build.sh`
* Obtain `spacepoints.root` file (e.g. scp it)
* Edit `/code/config.json` to point to it and run the code: `./build/application/HelixSolver/HelixSolver config.json`
