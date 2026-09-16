#!/bin/sh
#SBATCH --output=/share/40014784/mandelbrot.%a.txt

FMT=$1
NUM=$2
SIZE=$3
srun /share/40014784/mandelbrot --output $FMT --num $NUM --size $SIZE --job $SLURM_ARRAY_TASK_ID