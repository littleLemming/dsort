# dsort
University Assignment for Operation Systems - C-Program that should be equivalent to the bash-script :

`( $1; $2 ) | sort | uniq -d`

The objective of this assignment is mainly to understand how Pipes and Processes work in C. Furthermore dynamic allocation 
should get practiced once more.

## Proccess:
A process is an instance of a computer program  at runtime. It contains the program code, what is currently being done and 
the context (state of variables,..). On the other side a computer program is just a collection of instructions.

One program may have multiple proccesses running. One proccess may consist of multiple threads. 

Processes can be used to divide up (large) tasks and execute parts of that task concurrently.

Processes have a hirachy - every proccess has a parent, execpt the init Porcess - it is the root. Every process can have
one or multiple child processes. Every porcess has a unique ID, a `pid_t`. 

Processes get executed by the CPU one at a time - a scheduler switches between the tasks in order to use CPU-time most 
efficently. When a process get executed and if it gets executed in one go or only piece by piece depends on the priority of 
the process and the time it takes to execute.

#### fork

This command creates a child process. It does so by simply duplicating the calling process - which is in this context the 
parent process. 

Even though the child process is a duplicate of the parent process it has its own ID, the parent ID, is the one of the 
calling process not the one of the parents parent. Specific locks, pending signal, asynchronous I/O operations and 
context do not get inherented, CPU time counters and resource utilization get reset to zero.

Current values of variables, opened files, signal handling and file buffers do get inherited.

The reurn value of `fork()` is the process ID of the child in the parent and 0 in the child when the operation succeeds, -1
otherwise.

After the fork those two processes run parallel and execute the same program.

#### exec

Exec is not actually one command - it stands for a whole process family. 

```execl, execlp, execle, execv, execvp, execvpe``` are all part of this family.

They load a programm into the process and by doing so replace the old process image by the new one.

They only return -1 to indicate that an error occured - otherwise there is no return value.

###### Example execv:

```
char *cmd[] = { "bash", "-c", command, (char *) 0};
execv("/bin/bash", cmd);
```

This would for instance execute command which in this case is a char* that gets passed to the function. Command in this case
is a bash command. It is necessary to first pass the file that should get executed - `"/bin/bash"` in this case. As second
argument a list of arguments for the file gets passed on, the last one has to be a NULL pointer, the first argument has to
be the program name.

#### exit

This functions terminates a process. The termination status can be read by the parent, the child does not get a return value.
Exit flushes and closees stdio stream buffers, closes all opened files, removes all temporary files that have been created by
`tmpfile` and calls exit-handlers (`atexit`).

#### wait

Wait for the exit of a child - it returns the process ID of the child and the status of the terminated child.  This blocks,
until the child has terminated. 

After wait the child process gets removed from the proccess table, which is important as terminated processes remain in the
process table and no new processes can be started as soon as the table is full.

`waitpid` can be used for waiting for a specific child.

##### Zombie

A zombie is a terminated child process for which the parent did not yet call `wait()`. The state oft the child is set to 
zombie and it remains in the process table until `wait()` is called.

##### Orphan

This is a child process where the parent process terminated before the child process. The orphan gets inherited to the init
process. As soon as the orphan terminates, the init process removes it from the process table.

## Pipes:

A pipe is an undirectional data channel between related processes. 

A pipe can be created by calling `int pipe(int pipefd [2]);` the file descriptors get returned in the specified interger
array pipefd, where `pipefd[0]` is the read and and `pipefd[1]` is the write end.

For the pipe to work as expected all unused ends have to get closed. Read and write are supposed to get used via streams. 
A child process inherets the pipe. 

The read indicates end-of-file, if all write ends of the pipe are closed, write creates the signal `SIGPIPE` if all read 
ends of the pipe are closed.

The kernel automatically removes pipes where all ends have been closed.

It is possible to redirect stin/stdout into one of the pipe ends. This can be done by at first closing all unused pipe-ends,
then duplicating the opened file descriptor to the cloesd one and in the end closing the duplicated file descriptor.

For duplicating the file descriptors `dup` or `dup2` can be used. Afterwards the duplicted (but not wanted) file descriptor should get closed. 

An example with dup2 would be:

`dup2(command_pipe[1], fileno(stdout))`

and on success execute

`close(command_pipe[1])`

This example rewires the output of stdout to the write end of the pipe.

## Dynamic Allocation of memory:

This can be achieved by using `malloc` for reserving a block of memory for the first time, `realloc` for chaning the size of the block of memory and `free` for freeing the memory space as soon as it is no longer needed.
