#! /bin/bash
#SBATCH --job-name=connect4
#SBATCH --account=jyokota
#SBATCH --partition=savio3
#SBATCH --ntasks=960
#SBATCH --time=24:00:00

mpirun -n 960 ./build/connect4mpi.exe workingfolder
