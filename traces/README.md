# Traces for the project

1. The trace `505.mcf_r` comes from the SPEC CPU®2017 benchmark. Because of its size, it is not included in the project, but can be found [here](https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/). According to [its description](https://www.spec.org/cpu2017/Docs/benchmarks/505.mcf_r.html):

        505.mcf_r is a benchmark which is derived from MCF, a program used for single-depot vehicle scheduling in public mass transportation. The program is written in C. The benchmark version uses almost exclusively integer arithmetic.
        The program is designed for the solution of single-depot vehicle scheduling (sub-)problems occurring in the planning process of public transportation companies. It considers one single depot and a homogeneous vehicle fleet. Based on a line plan and service frequencies, so-called timetabled trips with fixed departure/arrival locations and times are derived. Each of these timetabled trips has to be serviced by exactly one vehicle. The links between these trips are so-called dead-head trips. In addition, there are pull-out and pull-in trips for leaving and entering the depot.
        Cost coefficients are given for all dead-head, pull-out, and pull-in trips. It is the task to schedule all timetabled trips to so-called blocks such that the number of necessary vehicles is as small as possible and, subordinate, the operational costs among all minimal fleet solutions are minimized.

2. The second trace is produced from the execution of a [c++ face recognition program](https://github.com/ShiqiYu/libfacedetection/).
All the source files can be found in the folder `/simulation_src/`.
The dataset used to run the algorithm is `WIDER FACE`, which can be found [here](https://shuoyang1213.me/WIDERFACE/).