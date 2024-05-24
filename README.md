# 3x3x3 Involution Solver

Solves all 170,911,549,184 involutions in half turn metric.

     d   Total          Unique by symmetry
    --------------------------------------
     0   1              1
     1   6              1
     2   3              1
     3   72             2
     4   39             4
     5   960            25
     6   886            41
     7   12708          292
     8   19526          506
     9   160059         3501
    10   349368         7741
    11   2133822        45543
    12   6903354        146698
    13   33277628       700019
    14   167193510      3500419
    15   932736959      19478862
    16   6251752591     130385528
    17   37364321274    778842829
    18   104805995145   2184417694
    19   21346235247    445145591
    20   456026         10842

# Requirements

* High core count x86 CPU with AVX2 and BMI extensions
* 256 GB memory recommended
* Hugepages (Linux kernel options `hugepagesz=1G hugepages=91`)
* Time (3-4 weeks using reference hardware)
* [A twophase solver](https://github.com/rokicki/cube20src)
* [An optimal solver](https://github.com/Voltara/vcube)
* At least 50GB of disk space

## Reference hardware

* 64-core Threadripper Pro 7985WX
* 256 GB memory
* SSD storage

# Build

    cmake . && make -j

# Run

## Initialize the solution database

This creates and initalizes the file `tables/invo.dat`

    ./invo create

## Run the edge coset solver to depth 15 (optional)

To verify the solver is working correctly, and to estimate how long
the entire process will take, you can run the edge coset solver to
depth 15.  On reference hardware, this takes about 24 minutes to finish.

    ./invo ecoset15

## Run the edge coset solver to depth 18

Find all solutions 18 moves or fewer.  This takes about two weeks
to finish on reference hardware.  The solver can be interrupted
safely with Ctrl-C or SIGTERM and resumed later.

    ./invo ecoset18

## Run the neighbor solver

The neighbor solver uses known solutions `s` to find neighboring
involutions of the form `m' s m` where `m` is any single move.
Because any remaining involutions have been proven by exhaustive
search to require 19 or more moves, any 19-move solutions to
unsolved involutions are guaranteed to be optimal.

This will take less than an hour to run, and will solve the
majority of remaining involutions.

    ./invo neighbor

## Find twophase solutions

Output a list of all unsolved cubes in Reid-Singmaster notation.

    ./invo unsolved > unsolved.txt

Use a twophase solver to find length 19 solutions and ingest them into
the solution database.

    THREADS=128    # Tune this to match your CPU thread count
    PROBES=1000000 # Good balance against diminishing returns
    ./twophase -t $THREADS -M $PROBES -s 19 < unsolved.txt | egrep '^([URFDLB][123]){19}$' | ./invo ingest

## Run the neighbor solver again

    ./invo neighbor

## Find optimal solutions

Again, output a list of all unsolved cubes.  There should be 6-7 million
cubes remaining at this point, which should take 1-2 days to solve.

    ./invo unsolved > unsolved.txt

Solve these with an optimal solver.  If using the vcube solver, be sure
to allocate enough 1GB hugepages to cover the entire pruning table
(this gives a 2x speed difference.)

    # Free up any shared memory hugepages for use by the optimal solver
    ./invo free

    # Run all the cubes through the opitmal sovler
    ./vc-optimal --coord=312 --ordered --format=reid --style=fixed < unsolved.txt > optimal-solutions.txt

    # Ingest the optimal solutions
    ./invo optimal < optimal-solutions.txt

## Get counts, output solutions

    # Quick count of symmetry representatives
    ./invo count

    # Count all solutions (slower)
    ./invo countall

    # Output solutions for symmetry representatives
    ./invo solutions
