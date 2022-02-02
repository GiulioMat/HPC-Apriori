# Parallel Apriori Algorithm with MPI and OpenMP

### Project Objective
The project objective is to present a parallel implementation of the Apriori algorithm using both MPI and OpenMP in order to efficiently extract frequent itemsets from a large dataset containing transactions made by clients. The algorithm has been implemented in different versions and the performaces have been analysed.

### Project Structure
- `utils` folder: contains the code used to preprocessed the dataset, in order to obtain a structure that can be easily read and handled by the implemented solution, and the code used to analyse the performaces of the different versions.
- `apriori.cpp`: serial implementation of the Apriori algorithm
- `apriori_mpi.cpp`: parallel implementation of the Apriori algorithm using MPI
- `apriori_omp.cpp`: parallel implementation of the Apriori algorithm using OMP
- `apriori_mpi_omp.cpp`: parallel implementation of the Apriori algorithm using both MPI and OMP


### Dataset
The dataset on which the algorithm was tested is the Instacart Market Basket Analysis dataset that can be found on Kaggle.

### Usage
In order to execute the algorithm on a computer cluster it is necessary to run a PBS script in which specify both the dataset to analyse and the minimum support to consider.
- Using the serial version:
```
#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb
#PBS -l walltime=5:00:00
#PBS -q short_cpuQ
./apriori ./order_products__prior.txt 0.01
```

- Using the MPI version:
```
#!/bin/bash
#PBS -l select=10:ncpus=1:mem=2gb
#PBS -l walltime=1:00:00
#PBS -q short_cpuQ
module load mpich-3.2
mpirun.actual -n 10 ./apriori_mpi ./order_products__prior.txt 0.01
```

- Using the OMP version:
```
#!/bin/bash
#PBS -l select=1:ncpus=10:mem=2gb
#PBS -l walltime=1:00:00
#PBS -q short_cpuQ
export OMP_NUM_THREADS=10
./apriori_omp ./order_products__prior.txt 0.01
```

- Using the MPI + OMP version:
```
#!/bin/bash
#PBS -l select=10:ncpus=10:mem=2gb
#PBS -l walltime=1:00:00
#PBS -q short_cpuQ
module load mpich-3.2
export OMP_NUM_THREADS=10
mpirun.actual -n 10 ./apriori_mpi_omp ./order_products__prior.txt 0.01
```
