*This project has been created as part of the 42 curriculum by harenaan.*

# Codexion

> Master the race for resources before the deadline masters you.

## Description

Codexion is a concurrency simulation written in C with POSIX threads.

Several **coders** sit around a circular table. Between each pair of neighbours lies one **USB dongle**, so there are as many dongles as coders. To compile their quantum code, a coder needs **two** dongles at once, the one on their left and the one on their right. A coder cycles through three activities: *compiling*, *debugging* and *refactoring*. If a coder does not start compiling within `time_to_burnout` milliseconds (counted from the start of the simulation or from their previous compile), they **burn out** and the simulation stops.

The goal is to share the dongles so that nobody burns out, without deadlocks, without starvation, and without data races.

What makes this version of the problem harder than the classic dining philosophers:

- **Dongle cooldown**: after a dongle is released, nobody can take it again until `dongle_cooldown` ms have passed.
- **Fair arbitration**: each dongle decides who gets it according to a scheduler, either `fifo` (first request first) or `edf` (earliest deadline first, where deadline = last compile start + `time_to_burnout`).
- **Priority queue**: the scheduling relies on a hand-written binary min-heap.
- **Monitor thread**: a dedicated thread detects burnouts and stops the simulation, and the burnout must be logged within 10 ms of the real burnout.

The simulation stops when a coder burns out, or when every coder has compiled at least `number_of_compiles_required` times.

### Project structure

```
.
├── Makefile
├── README.md
├── includes/
│   └── codexion.h       structures and prototypes
└── src/
    ├── main.c           entry point
    ├── parsing.c        argument validation
    ├── parse_utils.c    strict integer and scheduler parsing
    ├── init.c           allocation and initialisation
    ├── destroy.c        destruction and cleanup
    ├── time_log.c       time helper and serialised logging
    ├── utils.c          stop flag, precise sleep, getters
    ├── threads.c        thread creation and joining
    ├── coder.c          life cycle of a coder
    ├── acquire.c        taking and releasing the two dongles
    ├── dongle.c         per-dongle arbitration and cooldown
    ├── heap.c           binary heap: push, pop, sifting
    ├── heap_utils.c     binary heap: init, destroy, compare, peek
    └── monitor.c        burnout and completion detection
```

## Instructions

### Compilation

```bash
make
```

The project is compiled with `cc -Wall -Wextra -Werror -pthread`. Available rules: `all`, `clean`, `fclean`, `re`. The Makefile does not relink when nothing changed.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning | Accepted values |
|---|---|---|
| `number_of_coders` | number of coders (and of dongles) | integer >= 1 |
| `time_to_burnout` | ms allowed without starting a compile | integer >= 1 |
| `time_to_compile` | ms spent compiling, holding two dongles | integer >= 1 |
| `time_to_debug` | ms spent debugging | integer >= 1 |
| `time_to_refactor` | ms spent refactoring | integer >= 1 |
| `number_of_compiles_required` | compiles each coder must reach for the simulation to succeed | integer >= 1 |
| `dongle_cooldown` | ms a released dongle stays unavailable | integer >= 0 |
| `scheduler` | arbitration policy | exactly `fifo` or `edf` |

All arguments are mandatory. Negative numbers, non-integers, values that overflow an `int`, empty strings and any other scheduler name are rejected with an error message on `stderr` and exit code `1`.

### Output format

Every state change is printed on its own line:

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

`timestamp_in_ms` is the time elapsed since the start of the simulation and `X` is the coder number. No message is printed when the simulation ends successfully.

### Examples

```bash
# Everyone succeeds
./codexion 4 5000 100 50 50 3 10 edf

# A single coder has only one dongle, so they always burn out
./codexion 1 800 200 200 200 5 10 fifo
# 0 1 has taken a dongle
# 801 1 burned out

# Impossible parameters: a burnout is expected
./codexion 5 200 100 100 100 5 10 edf
```

### Testing

```bash
valgrind --leak-check=full ./codexion 4 5000 100 50 50 3 10 edf
valgrind --tool=helgrind   ./codexion 4 5000 100 50 50 3 10 fifo
```

Timings measured under Valgrind are not meaningful, because it serialises and slows down threads. Use it to look for leaks, invalid accesses and data races, and use a normal run to check timings.

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock needs four conditions at the same time:

1. **Mutual exclusion**: a dongle serves one coder at a time. Imposed by the subject.
2. **Hold and wait**: a coder holds one dongle while waiting for the other. Imposed by the subject.
3. **No preemption**: nobody can take a dongle away from its holder. Imposed by the subject.
4. **Circular wait**: coder 1 waits for coder 2, who waits for coder 3, and so on back to coder 1.

Since the first three cannot be removed, the solution breaks the fourth. Dongles are numbered, and **every coder takes the dongle with the lowest index first**, then the other one. Only the last coder sees their order inverted, because their neighbours wrap around the table (for example dongle 0 before dongle 3 with four coders). A coder waiting for their first dongle holds nothing, so the chain of waits always ends and can never close into a cycle.

With a single coder there is a single dongle and no second one, so the coder takes it and waits, which ends in a burnout as the subject expects.

### Starvation prevention

Each dongle owns its own waiting queue, implemented as a binary min-heap. A coder that wants a dongle pushes a request and is served only when their request is at the top of that dongle's heap, the dongle is free and its cooldown has elapsed.

- With `fifo`, the key of a request is a per-dongle ticket counter incremented under the dongle's mutex. Tickets are unique and strictly increasing, so the order is exactly the order of arrival. A timestamp is not used because two requests in the same millisecond would be tied.
- With `edf`, the key is the coder's deadline `last_compile_start + time_to_burnout`, so the coder closest to burnout is served first.
- **Tie-breaker**: when two keys are equal, the smaller coder id goes first. This makes the EDF policy fully deterministic, even in edge cases.

A dongle has only two neighbours and each coder has at most one pending request per dongle, so a dongle's heap never holds more than two requests. Its capacity is therefore fixed at 2.

### Cooldown handling

When a dongle is released, it stores `available_at = now + dongle_cooldown`. A coder may take it only if `now >= available_at`, and this check is made while holding the dongle's mutex.

A waiting coder does not poll. It sleeps in `pthread_cond_timedwait` until the cooldown ends, because nobody signals the condition when a cooldown simply expires. The wait is also capped at a few milliseconds so that the coder regularly notices that the simulation has stopped.

### Precise burnout detection

A separate monitor thread scans all coders about every 0.5 ms. For each coder it reads `compile_count` and `last_compile_start` under that coder's mutex, then compares the elapsed time with `time_to_burnout`. Coders that already reached the required number of compiles are ignored, otherwise their ageing timestamp would trigger a false burnout.

The burnout is printed at most about 1 ms after the real burnout, well within the 10 ms requirement. After announcing it, the monitor wakes every sleeping coder so that all threads exit quickly.

### Log serialisation

All output goes through a single mutex. The check of the `stop` flag and the `printf` happen inside the same critical section, so:

- two messages can never be interleaved on a line;
- once `burned out` has been printed, no other line can appear, since every later logging attempt sees `stop == 1` and prints nothing.

### Other edge cases

- **Sleeping without overshoot**: a long `usleep` could keep a coder asleep after the simulation ended, blocking `pthread_join`. Sleeps are cut into short steps of 200 µs that check the stop flag, and durations are computed in `long` to avoid `int` overflow.
- **Failure of `pthread_create`**: the stop flag is raised and the threads already started are joined before returning an error.
- **Memory**: every allocation is freed, every mutex and condition variable is destroyed, and threads are always joined before the cleanup, which avoids use-after-free.

## Thread synchronization mechanisms

### Primitives used

| Primitive | Where | Purpose |
|---|---|---|
| `pthread_mutex_t` in each dongle | `dongle.c` | protects `taken`, `available_at`, `ticket` and the waiting heap |
| `pthread_cond_t` in each dongle | `dongle.c` | puts waiting coders to sleep; woken by `pthread_cond_broadcast` on release and on stop |
| `pthread_mutex_t` in each coder | `coder.c`, `monitor.c` | protects `last_compile_start` and `compile_count` |
| `print_mutex` | `time_log.c`, `utils.c`, `monitor.c` | protects the output and the `stop` flag |
| `pthread_create` / `pthread_join` | `threads.c`, `main.c` | one thread per coder plus one monitor |

There are no global variables. Every thread reaches shared data through the `t_sim` structure, and each coder keeps a pointer to it.

### How a coder takes a dongle

```
lock(dongle.mutex)
push my request into dongle.heap
while not (I am at the top AND dongle is free AND cooldown is over):
    if the simulation is stopped: unlock and give up
    pthread_cond_timedwait(dongle.cond, dongle.mutex, until cooldown end or +5 ms)
pop my request ; taken = 1
unlock(dongle.mutex)
```

`pthread_cond_timedwait` atomically unlocks the mutex while sleeping and locks it again before returning. The wait is inside a `while` loop and not an `if`, because after waking up the condition may be false again: another coder may have been served first, and spurious wakeups are allowed by POSIX.

Releasing a dongle locks its mutex, sets `taken = 0`, sets `available_at`, and calls `pthread_cond_broadcast`. It is a broadcast and not a signal because both neighbours may be waiting, and it is the heap, not the wake-up order, that decides who goes first.

### Race conditions prevented

| Shared data | Written by | Read by | Protected by |
|---|---|---|---|
| `last_compile_start`, `compile_count` | the coder | the monitor | that coder's mutex |
| `stop` | the monitor | all coders, through the logger | `print_mutex` |
| `taken`, `available_at`, heap | the coders | the coders | that dongle's mutex |

Example of a race avoided. Without a mutex around `stop`, a coder could read `stop == 0`, then the monitor could set `stop = 1` and print `burned out`, and finally the coder would print its own line after the burnout. Testing `stop` and printing inside the same critical section makes this impossible.

A value shared between threads is always copied into a local variable while the mutex is held, and the mutex is released before returning the copy. A `return` placed before the `unlock` would leave the mutex locked forever.

### Lock ordering

To avoid introducing a new deadlock with the mutexes themselves, locks are always taken in the same order:

```
dongle mutex  ->  coder mutex
dongle mutex  ->  print mutex
```

The monitor never holds `print_mutex` while locking a dongle. It prints the burnout first, releases the print mutex, and only then locks each dongle to wake the sleeping coders.

### Communication between coders and monitor

The threads never message each other directly:

- the **coders publish** their progress by updating `last_compile_start` and `compile_count` under their own mutex;
- the **monitor reads** those values under the same mutex and decides;
- the **monitor signals the end** by raising `stop` under `print_mutex` and broadcasting every dongle's condition variable;
- the **coders notice** the stop flag when they next log, wait for a dongle or sleep, and exit their loop.

`main` then joins all coder threads and the monitor, and only after that destroys the mutexes and frees the memory.

## Resources

### Documentation and references

- `man pthread_create`, `man pthread_join`
- `man pthread_mutex_lock`, `man pthread_mutex_init`
- `man pthread_cond_wait`, `man pthread_cond_timedwait`, `man pthread_cond_broadcast`
- `man gettimeofday`, `man usleep`
- POSIX Threads Programming (Lawrence Livermore National Laboratory tutorial)
- Binary heap (Wikipedia) and the courses on the array representation of a heap
- Dining philosophers problem and Coffman conditions (Wikipedia)
- Earliest deadline first scheduling (Wikipedia)
- Valgrind manual, sections on Memcheck and Helgrind

### Use of AI

<!--
Edit this section so that it is exactly true for YOU.
Describe honestly which tasks AI helped with and which parts you wrote and tested yourself.
-->

AI (Claude) was used as a tutor and code reviewer, not as a code generator to copy from:

- **Explaining concepts**: deadlock and Coffman's conditions, data races, condition variables and why `pthread_cond_timedwait` is needed, how a binary heap is stored in an array, and how to read Valgrind and Helgrind reports.
- **Reviewing my code**: pointing out bugs and Norm issues in code I had written (for example a missing tie-breaker in the heap comparison, unprotected reads of the stop flag, and a wrong handling of the single-coder case).
- **Suggesting tests**: edge-case arguments, single coder, and parameters that are feasible or impossible.

I checked, ran and understood every part of the project, and I can explain it during the evaluation.

## License

Educational project, written for the 42 curriculum.
