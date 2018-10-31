/*
 *  mpi-example2.c
 *  
 *  Created by Edwin Montoya on 10/16/18.
 *  Copyright 2018 __Universidad EAFIT__. All rights reserved.
 *
 */

#include <stdio.h>
#include <math.h>

#include "mpi.h"

#define MASTER 0
#define FROM_MASTER 1
#define FROM_WORKER 2

#define LX 10000
#define LY 100000

long vec_sum[LX];
long matrix[LX][LY];

int taskId,
	numTasks,
	numWorkers,
	sourceId,
	destId,
	currentWorker = 0;

MPI_Status status;

void initMPI(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskId);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
	numWorkers = numTasks - 1;
}

void sendRows()
{
	long count = LX;
	long index;
	long i;
	long w;
	for (i = 0; i < LX; i++)
	{
		w = nextWorker();
		MPI_Send(&i, 1, MPI_LONG, w, FROM_MASTER, MPI_COMM_WORLD);
		MPI_Send(&matrix[i][0], count, MPI_LONG, w, FROM_MASTER, MPI_COMM_WORLD);
	}
	long fin = -1;
	for (i = 1; i <= numWorkers; i++)
	{
		w = nextWorker();
		MPI_Send(&fin, 1, MPI_LONG, w, FROM_MASTER, MPI_COMM_WORLD);
	}
}

void recvRows()
{
	long count = LX;
	long index = 0;
	long result;
	while (index != -1)
	{
		MPI_Recv(&index, 1, MPI_LONG, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
		if (index != -1)
		{
			MPI_Recv(&matrix[index][0], count, MPI_LONG, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
			result = processRow(index);
			MPI_Send(&index, 1, MPI_LONG, MASTER, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&result, 1, MPI_LONG, MASTER, FROM_MASTER, MPI_COMM_WORLD);
		}
	}
}

int processRow(int index)
{
	long i;
	long result = 0;
	for (i = 0; i < LX; i++)
		result = result + matrix[index][i];
	return result;
}

int DoSequencial()
{
	long i;
	for (i = 0; i < LX; i++)
	{
		vec_sum[i] = processRow(i);
	}
}

int nextWorker()
{
	if (currentWorker >= numWorkers)
		currentWorker = 0;
	currentWorker++;
	return currentWorker;
}

void recvResults()
{
	long count = LX;
	long i, index, w;
	currentWorker = 0;
	for (i = 0; i < LX; i++)
	{
		w = nextWorker();
		MPI_Recv(&index, 1, MPI_LONG, w, FROM_MASTER, MPI_COMM_WORLD, &status);
		if (index != -1)
		{
			MPI_Recv(&vec_sum[index], 1, MPI_LONG, w, FROM_MASTER, MPI_COMM_WORLD, &status);
		}
	}
}

void fillMatrix()
{
	long i, j;
	long val;
	int prob;
	for (i = 0; i < LX; i++)
		for (j = 0; j < LY; j++)
		{
			val = rand() % 256;
			matrix[i][j] = val;
		}
}
double start;
void main(int argc, char **argv)
{
	initMPI(argc, argv);
	start = MPI_Wtime();
	if (taskId == MASTER)
	{
		fillMatrix();
		sendRows();
		recvResults();
		printf("TOTAL time: %lf\n", MPI_Wtime() - start);
	}
	else
	{
		recvRows();
	}
	MPI_Finalize();
}