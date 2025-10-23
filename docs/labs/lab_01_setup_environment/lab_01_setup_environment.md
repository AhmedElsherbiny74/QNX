# Lab 01: Setting Up QNX Environment

## Objective
Install QNX SDP and run a program.

## Steps
1. Download QNX SDP from [www.qnx.com](https://www.qnx.com).
2. Install QNX SDP.
3. Write `code/hello.c`:
   ```c
   #include <stdio.h>
   int main() {
       return 0;
   }
   ```
4. Compile: `qcc -o hello code/hello.c`
5. Run: `./hello`

## Deliverables
- Screenshot of output.
- Answer: What is `qcc`?
