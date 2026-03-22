# Linux tools reimplement

The goal of this project is to understand how Linux tools work.

# Simple ps implementation (C)

## Project Overview
This projects is a simplified implementation of the ps command in Linux

It reads information directly from '/proc/[pid]/status' and display key information(PID, PPID, state and VmRSS memory usage).

## Feature
- List all processes
- Filter by PID (use '-p')
- Filter by process name (use '-n')
- Display limited number of outputs (use '-l')
- Sort by:
    - PID
    - PPID
    - Memory usage (VmRSS)

---
### 2. Process Parsing
- Read file using 'open()' and 'read()'
- Parse key-value pairs using 'sscanf'
- List information:
    - Name
    - PID
    - PPID
    - State
    - VmRSS

--- 

### 3. Sorting
Sorting is using 'qsort()' STD function with custom comparators.

Available sorting options:
- '--sort=pid'
- '--sort=ppid'
- '--sort=vmrss'

---

### 4. Filtering
There are two kind of filtering
- PID filter (use -p)
- Name filter (use -n)

---

## usage

./main 
./main -l 10
./main -p 1
./main -n bash
./main --sort=pid
./main --sort=ppid
./main --sort=vmrss

