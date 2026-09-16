#!/bin/sh
#SBATCH --output=/share/40014784/hello_cluster.%a.log
#SBATCH --array=0-7
srun echo $SLURM_ARRAY_TASK_ID
srun /share/40014784/hello_cluster
srun sleep 4