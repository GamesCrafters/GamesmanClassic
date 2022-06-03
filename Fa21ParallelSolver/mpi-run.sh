#! /bin/bash
#SBATCH --job-name=connect4
#SBATCH --account=jyokota
#SBATCH --partition=savio3
#SBATCH --ntasks=20
#SBATCH --time=01:00:00

./build/connect4mpi.exe workingfolder