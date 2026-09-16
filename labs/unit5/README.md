## Introduction

For this lab, we'll be connecting to the university's compute cluster, HPC2. We'll compile and run a few applications, to demonstrate a few typical use-cases of the cluster, based on what you've already been taught thus far. By the end of this lab we'll make:

* A simple application to run on multiple nodes, demonstrating simple jobs.
* A Mandelbrot set generator, that runs on a small region and outputs an image, demonstrating job arrays.
* A stitching application, that combines multiple tile image into a big one, demonstrating job dependencies.

## Connection essentials

To connect to the cluster, you need to be in the university network, physically or connected to a remote desktop via VPN/VDS. When in that network, you can access the [HPC2 webpage](https://hpc2.soc.napier.ac.uk/).

We have to connect to the cluster's head node via SSH. If you're using your own machine, you can install your favourite SSH client, otherwise you can download and run free and portable software, e.g. PuTTY and WinSCP. 

PuTTY will be used for the ssh access to the shell, where you'll run the various commands. You can download a standalone executable [here](https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe)

WinSCP will be used for easy explorer-like functionality, e.g. navigating folders, creating files, and using your favourite editor to edit text and source files. You can find a portable version of the software [here](https://winscp.net/download/WinSCP-5.17.3-Portable.zip)

Note: When editing files via your text editor, make sure you use Unix line endings.

To make the connection process easier, you can integrate PuTTY in WinSCP, by going to WinSCP's Options->Preferences->Integration->Applications and setting up the path to PuTTY executable. 

To connect, specify the target server and use your Napier username/password. After you connect, you should have a file explorer window, and you can start a PuTTY shell with Ctrl-P. 

## Linux environment

The cluster runs on Ubuntu, so we'll assume basic familiarity with the bash shell. We're not going to be using many commands, so here are the essentials:

* **ls**: list directory contents
* **cp**: copy files and directories
* **rm**: remove files and directories
* **mkdir**: make directories
* **pwd**: print name of current working directory
* **cat**: concatenate files and print on the standard output

And remember, the path separator is slash (/).

## Compilation with g++ (threaded program)

The standard C++ compiler in Linux is g++. Let's build a simple application to test the cluster. 

**hello_cluster.cpp**
```cpp
#include <thread>
#include <cstdio>
#include <climits>
#include <unistd.h>

int main(int argc, char ** argv)
{
    const auto num_threads = std::thread::hardware_concurrency();
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    printf("Hello from %s, with %d threads\n", hostname, int(num_threads));
    return 0;
}
```

You compile this into an executable as follows:

```bash
g++ -pthread -o hello_cluster hello_cluster.cpp
```

The above links the required posix thread library, outputs the executable "hello_cluster" and compiles the source file "hello_cluster.cpp"

When you run the application (calling ./hello_cluster), you should get the following output:

```bash
Hello from HPC2, with 32 threads
```
As this is the login node, we should not be doing any hard work here, but we should schedule jobs for the cluster nodes

## Cluster command basics

You can find info for using the cluster in the [tutorial page](https://hpc2.soc.napier.ac.uk/tutorial.html). The commands of interest for this lab are:

* **sinfo**: cluster information
* **squeue**: current list of queued jobs
* **srun**: Runs a specified command as a job
* **sbatch**: Runs a batch job

As `srun` executes on nodes, we need to place any executables and data somewhere accessible to the nodes. `/tmp` is a local folder already set up on each node, and `/share` is Network File Storage accessible by all nodes. In both, you should be creating subdirectories with your name or ID to avoid collisions, e.g. `/share/bkoniaris`. For ease of use, we'll be placing executables and data in `/share`, so that they're accessible by all nodes. Results can be placed there too, and be retrieved after all jobs finish.

Below, you'll see a lot the following appearing in srun and sbatch execution: `-p hpc2_oldNodes`.
Since the cluster is being currently used for important projects, we're going to be using a partition with some older nodes, which should not make much difference really for what we're trying to achieve today. 

## Simple job execution

`srun -p hpc2_oldNodes hostname` runs a single job on a single node: the command hostname. You should get a result such as `node3`

We can run the specified job a number of times (`n`), in a number of nodes (`N`), where `n > N`. Since nodes are a shared resource (and are currently being used by other researchers for experiments), you should be using a small `N`. For example:

`srun -p hpc2_oldNodes -n 8 -N 2 hostname` will run hostname 8 times on 2 nodes. Results could be:
```
node14
node14
node13
node14
node13
node13
node14
node13
```

Copy `hello_cluster` to your shared folder (e.g. `cp ./hello_cluster /share/40014784`) and run it (create it if it does not exist!). You should see something like:

```
Hello from node13, with 32 threads
Hello from node13, with 32 threads
Hello from node13, with 32 threads
Hello from node13, with 32 threads
Hello from node14, with 32 threads
Hello from node14, with 32 threads
Hello from node14, with 32 threads
Hello from node14, with 32 threads
```

The `srun` command is not ideal when we want to execute more things at a time, with more control over execution. For that we need to use `sbatch`.

## Batch jobs and scripts

Batch are a very convenient way of queuing up work. We typically provide a script that configures the batch job. The script is a simple shell script. Here's a tiny example, that runs our `hello_cluster` script using a job *array*.

We will be executing the scripts through sbatch, e.g. 
```
sbatch --extra-params /path/to/myscript.sh
```
where your shell scripts can be in the home folder. But the contents of the scripts should not reference the home folder!

```bash
#!/bin/sh
#SBATCH --output=/share/40014784/hello_cluster.%a.log
#SBATCH --array=0-7
#SBATCH --partition=hpc2_oldNodes
srun echo $SLURM_ARRAY_TASK_ID
srun /share/40014784/hello_cluster
srun sleep 4
```
Above:

* Line 1 tells Unix that the file is to be executed by /bin/bash.
* Line 2 configures the `sbatch` command to redirect the application's output to `hello_cluster.log`. The `%a` specifies the array index
* Line 3 configures the `sbatch` command with the specified number of jobs, as an array
* Line 4 configures the `sbatch` command with the specified partition
* Line 5 displays the task ID, which will be between 0 and 7
* Line 6 executes the `hello_cluster` command
* Line 7 sleeps a while so that we have time to call `squeue` to see what's going on

`squeue` would show something like this:
```
  40052 hpc2_main   pomicm 40014467  R    1:08:49      1 node12
  40054 hpc2_main   pomicm 40014467  R    1:07:25      1 node17
40088_0 hpc2_main hello_cl 40014784  R       0:01      1 node4
40088_1 hpc2_main hello_cl 40014784  R       0:01      1 node4
40088_2 hpc2_main hello_cl 40014784  R       0:01      1 node10
40088_3 hpc2_main hello_cl 40014784  R       0:01      1 node10
40088_4 hpc2_main hello_cl 40014784  R       0:01      1 node18
40088_5 hpc2_main hello_cl 40014784  R       0:01      1 node18
40088_6 hpc2_main hello_cl 40014784  R       0:01      1 node13
40088_7 hpc2_main hello_cl 40014784  R       0:01      1 node13
```

Above, you can see that the job array has been divided to multiple nodes for execution. You can call `cat /share/yourpath/hello_cluster.3.log` to see the contents of a log file.

## Running the Mandelbrot set calculator as a batched job array

The Mandelbrot generator from previous lectures has been modified to output colour, and to only calculate a square region out of the entire region of interest. It requires a number of parameters:

* `--output`: Output PNG format (printf style, e.g. /path/to/myimage%02.png)
* `--num`: Number of tile rows/columns
* `--size`: Size of square tile in pixels
* `--job`: Job array id, in [0,N*N-1]

Running the command to generate a single tile (1x1), e.g.

`./mandelbrot --output /share/40014784/mandelbrot_tile_%03d.png --num 1 --size 1024 --job 0`

should generate the following image:

![Mandelbrot](img/mandelbrot.png)

To better utilise the cluster, we'll define a larger tile grid (e.g. 4x4), and create a job array so that we calculate each tile as a separate task. The job script can be as follows:

```bash
#!/bin/sh
#SBATCH --output=/share/40014784/mandelbrot.%a.log
#SBATCH --array=0-15
#SBATCH --partition=hpc2_oldNodes
srun /share/40014784/mandelbrot --output /share/40014784/mandelbrot_tile_%03d.png --num 4 --size 512 --job $SLURM_ARRAY_TASK_ID
```

In the above, we use a 4x4 tile grid (so we specify an array 0-15), where each tile is 512 pixels. We run the program, passing the appropriate command line arguments. The argument of interest here is `$SLURM_ARRAY_TASK_ID` which will take values between 0 and 15. After you run the batch job, you should get 16 image files, which when pieced together should make the familiar image of the set:

![Mandelbrot tiles](img/mandelbrot_tiles.png)

You might notice that the array specification (--array=0-15) is in the comments, making parameterisation difficult. So, we can modify the script to the following:

```bash
#!/bin/sh
#SBATCH --output=/share/40014784/mandelbrot.%a.log
#SBATCH --partition=hpc2_oldNodes
NUM=$1
srun /share/40014784/mandelbrot --output /share/40014784/mandelbrot_tile_%03d.png --num $NUM --size 512 --job $SLURM_ARRAY_TASK_ID
```

and configure the `--array` parameter explicitly in the `sbatch` command line:

```bash
sbatch --partition=hpc2_oldNodes --array=0-15 ./mandelbrot.sh 4
```

The above command evaluates the expression `4*4-1`, substituting to `sbatch --array=0-15 ./mandelbrot.sh 4`
Now, you should be able to run the batched Mandelbrot tile calculation where you can easily adjust the tile grid dimensions.

## Job dependencies

So far we've seen how to schedule a multitude of jobs in parallel, in different machines. In some cases, we want to do some further processing after some jobs are finished, but we don't want to manually check *when* those jobs are finished. We want to be able to specify that our processing task **depends** on a number of tasks, completing successfully. As an example use-case scenario, we want to run a stitching application that will put together all generated tiles and generate a combined big image. This task of course depends on the successful completion of all the other jobs that are responsible of generating the image tiles. The `sbatch` command supports specifying depedencies, by providing the job ID(s) to depend on. You can easily obtain the job id by passing the "--parsable" option in `sbatch`, which will cause the command to just return the job ID as output, which can be easily retrieved and used for the next, dependent job.

First, we write a simple batch script for the stitch command:
```bash
#!/bin/sh
#SBATCH --output=/share/40014784/stitch.txt
#SBATCH --partition=hpc2_oldNodes
NUM=$1
srun /share/40014784/stitch --input /share/40014784/mandelbrot_tile_%03d.png --output /share/40014784/mandelbrot_stitched.png --num $NUM
```

As can be seen, the `stitch` application takes as parameters the input format, the output image filename and the number of tiles per dimension. The size of each tile is not necessary, as it can be obtained from any image. 

In order to chain these two scripts together, we write a 3rd script, `mandelbrot_and_stitch.sh`. This executes the batched mandelbrot script, obtains the job ID, and uses it as a dependency for the stitching script:

```bash
#!/bin/bash
NUM=$1
MY_JOB_ID=$(sbatch --partition=hpc2_oldNodes --parsable --array=0-$(($NUM * $NUM - 1)) ./mandelbrot.sh $NUM )
sbatch --partition=hpc2_oldNodes --dependency=afterok:$MY_JOB_ID ./stitch.sh $NUM
```

The "afterok" dependency specification implies that the stitching job will only be executed if the mandelbrot job has completed successfully. The size of the tile grid is again given as a command line parameter, and is also used in the first `sbatch` command to evaluate the expression in the parentheses.

## Further

HPC2 is a shared resource, and is used a lot for research, so it's not ideal to run toy tests that consume a lot of resources, and you need to be mindful when using it. As a university, we have access to [ARCHER](http://www.archer.ac.uk/training/online/), so that could be used for training. Additionally, you might find free evaluation periods for services such as AWS and GCP.
