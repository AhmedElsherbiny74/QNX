# QNX Scheduling - Markdown Summary

## ✅1. What QNX Schedules

- QNX schedules **threads**, not processes
- Processes = containers for threads
- All scheduling decisions are performed **per-thread**
---



---
## ✅2. Thread States

| State   | Description                              | Schedulable |
| ------- | ---------------------------------------- | ----------- |
| Running | Currently executing on a CPU core        | ✅ Yes       |
| Ready   | Can run but waiting for a core           | ✅ Yes       |
| Blocked | Waiting for event (message, mutex, etc.) | ❌ No        |
| Dead    | Terminated but not cleaned up yet        | ❌ No        |

Blocked threads are ignored by the scheduler.

---



---
## ✅3. Priorities

- Range: **0–255**
- Higher number = higher priority
- Special reserved ranges:
    - **255** → Kernel IPI threads
    - **254** → Timer + in-kernel interrupt service threads
    - **0** → Idle thread (1 per core)
- User threads are **1–253**
---



---
## ✅4. Preemptive Priority Scheduling

- QNX uses **strict priority preemption**
- A higher priority READY thread **immediately** preempts a lower one
---



---
## ✅5. Multicore Scheduling

A multicore system is simply a system where there is **more than one CPU core** inside the same chip.  
Even though each core can run its own threads independently, **they share a lot of hardware**, like:
- The same RAM
- The same memory bus
- The same I/O devices (network, storage, etc.)
- Sometimes parts of the CPU cache
#### So the cores are **separate brains**, but they all live in the **same body** and share the same organs.
---
## ✅5.1 SMP (Symmetrical Multiprocessing)

QNX normally treats all cores **as if they were identical**.  
That is what **SMP** means: symmetrical → “same”.
From the kernel’s point of view:
> “Any thread can run on any core because all cores are equal.”

The default QNX kernel (`procnto-smp`) uses this model.

---



---
## ✅6. Clusters

#### 6.1 Big idea
	- QNX groups CPU cores into Clusters
	- the kernel keeps a ready list per cluster 
----
#### 6.2 why clusters ? 
* we need something better than one global queue 
* A single global queue makes the scheduler search through many threads to find one that is allowed on a given core — > slow 
- A cluster = set of CPU cores a thread is allowed to run on
- Every thread belongs to exactly 1 cluster
- Every core belongs to at least 2 clusters
- the system keep a **ready list** for each cluster for threads
---
#### 6.3 How scheduling happens 
- Threads are placed on the ready list of the single cluster they belong to.
- When a core becomes free, the scheduler examines the ready lists for **clusters that include that core**
#### How you define clusters
1. **In startup code** (the BSP) — compiled/installed into the startup configuration.
2. **At boot time** using startup’s `-c` command-line option to add clusters.
	
- Additional custom clusters are defined in **startup code** or using `startup -c`
Example:

```
-c cluster0:0x7   # cores 0,1,2
-c cluster1:0x9   # cores 0 and 3
```
Interpretation:
- `0x7` = binary `0111` → cores 0,1,2 are in `cluster0`.
- `0x9` = binary `1001` → cores 0 and 3 are in `cluster1`.
---
### 6.4 Thread states (simplified)

| state       | meaning                                           |
| ----------- | ------------------------------------------------- |
| **running** | actively executing on a core                      |
| **ready**   | can run, but waiting for a core                   |
| **blocked** | cannot run (waiting for mutex, message, I/O, etc) |

If it’s **running**, it is physically on _one_ CPU core — always.  
If it’s **ready**, it just sits in _its cluster’s ready list_ waiting to be chosen.

---
### 6.5 Ready list inside each cluster: priority + timestamp

When a thread becomes “ready” (not blocked anymore), QNX inserts it into the cluster’s ready list.
That list is sorted by:
1. **priority** (highest first)
2. **timestamp** (earliest/oldest first among equal priority)
---
### 6.7 How a Thread Becomes Running
1. A thread becomes READY
2. Added to its cluster ready list (ordered by priority + timestamp)
3. Scheduler checks:
    - Can it preempt this core?
    - Is there an idle core in the cluster?
    - Can it preempt a lower-priority thread on another core?
4. If yes → scheduled
5. If not → waits in READY
---



---
## ✅7 Core Affinity (very important in QNX)

Because not all cores are equal, sometimes we want to **tell QNX that a particular thread should only run on specific cores**.
Core affinity means:
> “Which cores is this thread _allowed_ to run on?”

And in QNX, affinity is done **by mapping the thread to a cluster** (because a cluster already defines a set of cores).
So when you set affinity, you’re basically saying:

> “Put this thread into _this_ cluster.”

Example:  
If a thread needs high performance, maybe you want it to always run on a “big” core.  
Or if two threads share a lot of data, you want them on cores that share the same cache.

---
### 7.1 How do we set affinity in QNX?

You use the `ThreadCtl()` API with the command `_NTO_TCTL_RUNMASK`.

The **runmask** is just a bitmask:
- Each **bit** represents a CPU core.
- 1 = allowed
- 0 = not allowed

Example:

```
0x1  → 0001  → only core 0
0x3  → 0011  → core 0 and 1 
0x9  → 1001  → core 0 and 3`
```
Because QNX supports up to **64 cores**, the mask is a **64-bit** value.

---


---
## ✅8. Scheduling Policies (Algorithms)

###  Key idea

Every **thread** in QNX has:
1. A **priority**
2. A **scheduling algorithm**
The **priority** determines _who runs first_.  
The **algorithm** controls **how** a thread _uses_ the CPU **after** it has been chosen.

So:

> Priority decides **IF** it gets CPU  
> Algorithm decides **HOW LONG / HOW** it stays on CPU

| Algorithm             | Meaning                                   | Behaviour                                    |
| --------------------- | ----------------------------------------- | -------------------------------------------- |
| **FIFO**              | First In, First Out                       | Runs until it blocks (never time-sliced)     |
| **Round Robin**       | Fair sharing among equal-priority threads | Each gets a time slice, then rotates         |
| **Sporadic**          | Priority changes over time                | Protects CPU from overuse                    |
| **High-Priority ISD** | Special kernel-internal threads           | Used for interrupt service and timer threads |

## 1️⃣ FIFO (simple one)

- Thread runs **as long as it wants** until it _blocks_ (ex: waiting on I/O, mutex, etc.)
- If nothing blocks it and no higher priority preempts it, **it keeps the CPU**    
- No time slicing
Think: "I stay in front of the line until _I_ decide to step aside"
---
## 2️⃣ Round Robin

- Used when **multiple threads have the same priority**
- They **take turns**, each for a **time slice**
- After a thread uses its slice, it moves to the back of the queue
    
Example with A, B, C on 1 core:
``
A → B → C → A → B → C → ...
``
If on **2 cores**, rotation overlaps:
```
Core1: A → C → A → C ... 
Core2: B → A → B → A ..
```

---
## 3️⃣ Sporadic Scheduling (more advanced)

This one is used to **prevent CPU starvation** and keep the system responsive.
**Sporadic** = high priority for limited time, then low priority
A sporadic thread has **4 parameters**

| Parameter                | Meaning                                           |
| ------------------------ | ------------------------------------------------- |
| **priority**             | its normal (high) scheduling priority             |
| **low priority**         | fallback priority once budget is used             |
| **budget**               | how much CPU time it can run at **high priority** |
| **replenishment period** | when the budget resets                            |

---
## 4️⃣ High Priority ISD (Kernel-only)

- Used internally by QNX for interrupt service and clock/timer threads
- Runs at **very high reserved priorities** (254–255)
- Behaves **like FIFO**, but with special privilege to use those top priorities
- Used when you call `InterruptAttachEvent()` in drivers
    
So:  
FIFO → normal user threads  
High-Priority ISD → kernel-level interrupt service threads

--------
