#!/bin/bash
NUM=8
FMT=/share/40014784/mandelbrot_tile_%03d.png
SIZE=1024
MY_JOB_ID=$(sbatch --parsable --array=0-$(($NUM * $NUM - 1)) ./test/mandelbrot.sh $FMT $NUM $SIZE)
sbatch --dependency=afterok:$MY_JOB_ID ./test/stitch.sh $FMT $NUM $SIZE