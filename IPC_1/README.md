# QNX Lab 2 – Understanding User-Space “Driver” IPC

## 1️⃣ What’s happening in your lab — the big picture

You wrote two user programs:

- **sensor** → acts like a “driver”
- **app** → acts like an “application” that uses the driver

Both are **normal user-space processes**, not kernel code.  
They run on top of QNX’s **microkernel** and **process manager**.

But they communicate **as if the app were talking to a driver in the kernel** — using **message passing**.  
That’s the magic of QNX.

---

## 2️⃣ How they connect

Let’s follow what happens when you run each one.

### a) When you start **sensor**
```cpp
name_attach(NULL, "sensor", 0);
