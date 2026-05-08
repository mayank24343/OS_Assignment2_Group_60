# SimpleShell - A Unix Shell in C from Scratch

## Design Document for Operating Systems (OS) Assignment 2

---

# Group Members & Contribution

## 2024138 - Atharva Singh Velpula

### Contributions

* `show_history()` function
* `display_info()` function
* `cd` command implementation
* Recording relevant data for each process

---

## 2024343 - Mayank Yadav

### Contributions

* Pipe commands implementation
* Execution for commands other than `cd` and `history`
* Basic shell loop
* `Ctrl + C` handler

---

## Joint Contribution

Both members contributed to:

* Error handling
* Bug fixing

---

# Supported Commands

The shell supports standard Unix commands including:

```bash id="sx5oz6"
ls, pwd, mkdir, history, cd, exit, cat, head, tail,
less, sort, uniq, nano, rm, rmdir, clear, man,
find, which, grep, echo, mv, cp, htop, ps, etc.
```

---

# Unsupported Commands

## Background Processes

Example:

```bash id="l6lbpk"
./helloworld &
```

### Reason

* The shell calls `wait()` for every child process.
* Execution does not continue until the current command completes.
* Supporting background processes would require:

  * Additional process tracking
  * Separate data structures for active background jobs

---

## Globbing

Example:

```bash id="z3vsgz"
ls *.c
```

### Reason

* The shell currently passes each space-separated string directly as arguments to `execvp()`.
* No preprocessing exists to expand wildcard patterns such as `*.c`.

---

## Tab Autocomplete & Arrow Key Navigation

### Unsupported Features

* Tab autocomplete
* Up/Down arrow command navigation

### Reason

These require:

* Advanced terminal input handling
* Character-level processing
* Autocompletion algorithms

---

## I/O Redirection

Examples:

```bash id="zh2w0w"
<
>
>>
```

### Reason

* Requires additional file descriptor management.
* Can interfere with existing pipe logic.
* Needs more complex parsing and execution handling.

---

# Implementation Details

* The codebase is well documented using:

  * Comments
  * Meaningful variable names
  * Clear function names

---

# `main()`

### Functionality

* Assigns `my_handler()` as the handler for `SIGINT`.
* Similar to implementation shown in lecture slides.
* Calls:

  * `shell_loop()` to run the shell
  * `cleanup_history()` for cleanup on termination

---

# `shell_loop()`

### Functionality

* Runs as a `do-while` loop.
* Commands are accepted from the user using `scanf()`.

### Features

* Maximum input size: **1 KB**
* Commands are passed to a launcher for execution.
* `exit` command:

  * Terminates the loop
  * Calls `display_info()` to show process history

### History Management

* Every command is stored in history.
* Maximum history size: **100 commands**

### History Structure

History is stored as an array of `struct Commands`, containing:

* Command
* PID(s)
* Start time
* End time
* Duration

---

# `launch()`

### Functionality

* Launches commands using `create_process_and_run()`
* Returns execution status

---

# `create_process_and_run()`

## Command Handling Logic

### Exit Command

* If command is `exit`, returns `0`.

---

## Pipe Detection

* Determines whether the command contains pipes.

---

## Non-Piped Commands

### `cd` Command

* Detects if command is `cd`.
* Changes current working directory appropriately.

### Other Commands

* Creates a child process using `fork()`.
* Executes:

  * `history` using `show_history()`
  * Other commands using `execvp()`

### Argument Parsing

* Arguments are generated using `read_command()`.

---

## Piped Commands

### Pipe Creation

For a command with `n` pipes:

* An array `fd` is created to store:

  * `2 × (n - 1)` file descriptors

### Pipe System Calls

* `pipe()` syscall is used in a loop to create pipes.

### Process Creation

* `n` child processes are created using a `for` loop.

### File Descriptor Duplication

Each child:

* Duplicates required file descriptors for:

  * `STDIN`
  * `STDOUT`

### Pipe Convention

* Read end → even index
* Write end → odd index

### Execution

* Subcommands are executed using `execvp()`
* Similar logic as single-command execution

### Cleanup

* Both parent and child processes close unused file descriptors appropriately.

### Synchronization

* Parent waits for all child processes to complete execution.

---

## Process Tracking

### Recorded Information

For every command:

* Start time
* End time
* Duration

Duration is computed as:

```text id="suw7v4"
duration = end_time - start_time
```

### Return Value

* Returns execution status.

### Error Handling

* Throws an error if `fork()` fails.

---

# `read_command()`

### Functionality

* Splits commands into tokens/words using spaces.
* Returns an array of strings.

---

# `show_history()`

### Functionality

* Displays all entries stored in the history array.

---

# `display_info()`

### Functionality

Displays:

* Commands executed
* PID(s)
* Start times
* Durations

### Piped Commands

* Displays PIDs of all child processes created for the pipeline.

---

# `my_handler()`

### Functionality

* Handles `SIGINT` (`Ctrl + C`)
* Ensures:

  * `display_info()`
  * `cleanup()`

are called before shell termination.

---

# Process Flow Summary

```text id="49gjlwm"
User Input
    ↓
shell_loop()
    ↓
launch()
    ↓
create_process_and_run()
    ↓
 ┌───────────────┬────────────────┐
 │ Non-Piped     │ Piped Command  │
 └───────────────┴────────────────┘
        ↓                  ↓
     fork()           pipe() + fork()
        ↓                  ↓
     execvp()         execvp() chain
        ↓                  ↓
      wait()            wait()
        ↓
 History Update
```

---

# Key System Calls & Functions Used

* `fork()`
* `execvp()`
* `wait()`
* `pipe()`
* `dup2()`
* `scanf()`
* `signal()`
* `chdir()`

---

# Notes

* The shell is implemented completely in C using Unix system calls.
* Pipe handling supports multiple chained commands.
* Process execution details are tracked and stored in history.
* Signal handling ensures graceful termination using `Ctrl + C`.
* The shell focuses on core Unix shell functionality while keeping implementation modular and extensible.
