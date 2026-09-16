#!/bin/sh
#SBATCH --output=/share/40014784/stitch.txt

FMT=$1
NUM=$2
SIZE=$3
echo /share/40014784/stitch --input $FMT --output /share/40014784/mandelbrot_stitched_$NUMx$NUM_t$SIZE.png --num $NUM
srun /share/40014784/stitch --input $FMT --output /share/40014784/mandelbrot_stitched_$NUMx$NUM_t$SIZE.png --num $NUM