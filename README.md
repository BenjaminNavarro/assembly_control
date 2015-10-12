# Assembly control

## Project organization
- CMakeLists.txt : CMake configuration file
- src : All source files
  - application (where to put your code)
  - example (some code examples)
  - lib (V-REP interface library)
- build : Build directory
- bin : executables output directory
- doc : Documentation
  - Makefile (to update the documentation)
  - html (Doxygen documentation, index.html)
  - Sujet\_Cellule\_Robotisee.doc (project subject)
- nets : Petri nets
  - full (full Petri nets, with in/out places)
  - analyze (analyzable Petri nets, without in/out places)
 
## Building the project
CMake v2.6 and GCC v4.7 (or newer) required

V-REP needs to be installed within a folder called 'V-REP' under:
- Your home folder (Linux)
- Your 'Applications' folder (Mac) 

In a terminal, do the following:
```
cd build
cmake ..
make
```

## Running the code
Start V-REP (open ~/vrep.sh in a terminal for Linux users) then do:
```
cd bin
./your_application
```
Where 'your\_application' can be one of the following (by default):
- 'assembly\_control' (your application)
- 'simple\_assembly\_control' (your application, simplifed version)
- 'example' (example)
- 'simple\_example' (example, simplifed version)
- 'tasks\_example' (example with two tasks)
