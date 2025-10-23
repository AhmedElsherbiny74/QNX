# QNX Scheduling 

## 1. What QNX Schedules

* QNX schedules **threads**, not processes
* Processes = containers for threads
* All scheduling decisions are performed **per-thread**

---

## 2. Thread States

| State   | Description                              | Schedulable |
| ------- | ---------------------------------------- | ----------- |
| Running | Currently executing on a CPU core        | ✅ Yes       |
| Ready   | Can run but waiting for a core           | ✅ Yes       |
| Blocked | Waiting for event (message, mutex, etc.) | ❌ No        |
| Dead    | Terminated but not cleaned up yet        | ❌ No        |

Blocked threads are ignored by the scheduler.

---

## 3. Priorities

* Range: **0–255**
* Higher number = higher priority
* Special reserved ranges:

  * **255** → Kernel IPI threads
  * **254** → Timer + in-kernel interrupt service threads
  * **0** → Idle thread (1 per core)
* User threads are **1–253**

---

## 4. Preemptive Priority Scheduling

* QNX uses **strict priority preemption**
* A higher priority READY thread **immediately** preempts a lower one
* Not fair-share and not time-sliced unless Round Robin used

---

## 5. Multicore Scheduling

* One running thread per CPU core
* Modern SoCs have different performance/cache properties
* QNX introduces **clusters** to express allowed cores

---

## 6. Scheduling Clusters

* A cluster = set of CPU cores a thread is allowed to run on
* Every thread belongs to exactly 1 cluster
* Every core belongs to at least 2 clusters:

  1. **Global cluster** (all cores)
  2. **Per-core cluster** (for idle + kernel threads)
* Additional custom clusters are defined in **startup code** or using `startup -c`

Example:

```
-c cluster0:0x7   # cores 0,1,2
-c cluster1:0x9   # cores 0 and 3
```

---

## 7. Thread Affinity (Runmask)

* Set using `ThreadCtl(_NTO_TCTL_RUNMASK, runmask)`
* `runmask` is a 64-bit bitfield (1 bit per core)
* Must match an existing cluster

  * Mismatch → ThreadCtl() fails

Uses:

* Isolate safety-critical threads
* Tie workloads to high-performance cores
* Better cache locality
* Hypervisor / partition isolation

---

## 8. How a Thread Becomes Running

1. A thread becomes READY
2. Added to its cluster ready list (ordered by priority + timestamp)
3. Scheduler checks:

   * Can it preempt this core?
   * Is there an idle core in the cluster?
   * Can it preempt a lower-priority thread on another core?
4. If yes → scheduled
5. If not → waits in READY

---

## 9. Scheduling Policies (Algorithms)

| Policy      | Behavior                               |
| ----------- | -------------------------------------- |
| FIFO        | Runs until it blocks or is preempted   |
| Round Robin | FIFO + rotates after time-slice        |
| Sporadic    | Temporary high priority with budget    |
| HI-PRI      | Special kernel-only 254–255 priorities |

---

## 10. Key Concepts Summary

* **Priority** = who runs first
* **Cluster/runmask** = where it can run
* **Algorithm** = how long before yielding
* Good RT threads spend **most time blocked** until needed

---
