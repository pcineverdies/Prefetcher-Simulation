# Traces for the project

Since both the traces are big in size, you can download them from [this Gdrive folder](https://drive.google.com/drive/folders/19CfteU82DXrsyzXzm1HmTaKhj6BXxOmd?usp=sharing).

1. The trace `605.mcf_r` comes from the SPEC CPU®2017 benchmark; it comes from [this repository](https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/). According to [its description](https://www.spec.org/cpu2017/Docs/benchmarks/505.mcf_r.html), here is its description:

    *605.mcf_s is a benchmark which is derived from MCF, a program used for single-depot vehicle scheduling in public mass transportation. The program is written in C. The benchmark version uses almost exclusively integer arithmetic.*

    *The program is designed for the solution of single-depot vehicle scheduling (sub-)problems occurring in the planning process of public transportation companies. It considers one single depot and a homogeneous vehicle fleet. Based on a line plan and service frequencies, so-called timetabled trips with fixed departure/arrival locations and times are derived. Each of these timetabled trips has to be serviced by exactly one vehicle. The links between these trips are so-called dead-head trips. In addition, there are pull-out and pull-in trips for leaving and entering the depot.*

    *Cost coefficients are given for all dead-head, pull-out, and pull-in trips. It is the task to schedule all timetabled trips to so-called blocks such that the number of necessary vehicles is as small as possible and, subordinate, the operational costs among all minimal fleet solutions are minimized.*

    *For simplification in the benchmark test, we assume that each pull-out and pull-in trip is defined implicitly with a duration of 15 minutes and a cost coefficient of 15.*

    *For the considered single-depot case, the problem can be formulated as a large-scale minimum-cost flow problem that we solve with a network simplex algorithm accelerated with a column generation. The core of the benchmark 605.mcf_s is the network simplex code "MCF Version 1.2 -- A network simplex implementation", For this benchmark, MCF is embedded in the column generation process.*

    *The network simplex algorithm is a specialized version of the well known simplex algorithm for network flow problems. The linear algebra of the general algorithm is replaced by simple network operations such as finding cycles or modifying spanning trees that can be performed very quickly. The main work of our network simplex implementation is pointer and integer arithmetic.*

    *Because there have been no significant errors or changes during the years 2000 - 2004, most of the source code of the CPU2000 benchmark 181.mcf was not changed in the transition to CPU2017 benchmark 605.mcf_s. However, several central type definitions were changed for the CPU2017 version by the author:*
        
    - *Whenever possible, long typed attributes of struct node and struct arc are replaced by 32 bit integer, for example if used as boolean type. Pointers remain unaffected and map to 32 or 64 bit long, depending on the compilation model, to ensure compatibility to 64 bit systems for truly large scale problem instances.*
        
    - *To reduce cache misses and accelerate program performance somewhat, the elements of struct node and struct arc, respectively, are rearranged according to the proposals made in "Memory Profiling using Hardware Counters" by Marty Itzkowitz, Brian Wylie, Christopher Aoki, and Nicolai Kosche (http://www.sc-conference.org/sc2003/paperpdfs/pap182.pdf)*

2. The second trace is produced from the execution of a [c++ face recognition program](https://github.com/ShiqiYu/libfacedetection/).
All the source files can be found in the folder `/simulation_src/`.
The dataset used to run the algorithm is `WIDER FACE`, which can be found [here](https://shuoyang1213.me/WIDERFACE/).