#include <iostream>
#include <ctime>  
#include <cstdlib>
#include <omp.h>
#include <fstream>
#include <vector>
#include "../profiling_modules/profiling_module.h"

using namespace std;

// ход мысли под привычную инициализацию матриц
//void matrix(int n) {
//	float **ary;
//	ary = new float* [n];
//	for (int i = 0; i < n; i++) {
//		ary[i] = new float[n];
//	}
//}

//сведение двойного индекса к одинарному
int getSingleIndex(int row, int col, int numberOfColumnsInArray)
{
	return (row * numberOfColumnsInArray) + col;
}


//создание матриц из единиц и двоек
float* make_matrix_of_1(int n)
{
	int m = n * n;
	float *arr = new float[m];
	//int m = sizeof(arr)/sizeof(float);

	// работа с массивом
//#pragma omp parallel for
	for (int i = 0; i < m; i++) {
		arr[i] = 1.;
	}
	return arr;
}

float* make_matrix_of_2(int n)
{
	int m = n * n;
	float *arr = new float[m];
	// работа с массивом
//#pragma omp parallel for
	for (int i = 0; i < m; i++) {
		arr[i] = 2.;
	}
	return arr;
}

//выделение памяти под матрицу результат
float* make_matrix(int n)
{
	int m = n * n;
	float *arr = new float[m];
//#pragma omp parallel for
	for (int i = 0; i < m; i++) {
		arr[i] = 0.;
	}
	return arr;
}

//очистка памяти
void del_matrix(float* ary) {
	delete[] ary;
}

//проверка результата для перемножения марицы единиц на матрицу двоек
int check_res(float* c, int n)
{
	int N = 2 * n;
	int flag = 0;
//#pragma omp parallel for collapse(2) //замедляет в 3 раза
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (c[getSingleIndex(i, j, n)] != N) flag = 1;		
		}
	}
	if (flag == 1) return 1;
	else return 0;
}

//искомая функция перемножения матриц
void mmul(float* a, float* b, float* c, int n) 
{
	if (n < 1) {
		cout << "\n" << "Please, enter the correct dimention of matrix \n";
		exit(1);
	}

	float* b_t = new float[n*n];
	unsigned int start_time1 = clock();
#pragma omp parallel
	{
#pragma omp for //collapse(2) //ускоряет
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				b_t[getSingleIndex(i, j, n)] = b[getSingleIndex(j, i, n)];
			}
		}
	}
	unsigned int start_time2 = clock();
	unsigned int time1 = start_time2 - start_time1; // искомое время
	//cout << "timetransp passed: " << (float)time1 / CLOCKS_PER_SEC << "sec" << "\n";
	del_matrix(b);
	unsigned int start_time = clock(); // начальное время
	/*int i;*/
	int j;
	int k;
	/*float bufer = 0;*/


#pragma omp parallel for //collapse(3) //эффективности коллапса по времени не замечено, разброс больше, как проверить?
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				for (int k = 0; k < n; k++) {
					c[getSingleIndex(i, j, n)] += a[getSingleIndex(i, k, n)] * b_t[getSingleIndex(j, k, n)];
					//x += a[i][k] * b[k][j];
				}
			}
		}
	
	if (check_res(c, n) == 0) cout << "Checking result: OK\n";
	else cout << "Checking results: Error\n";

	unsigned int end_time = clock(); // конечное время
	unsigned int time = end_time - start_time; // искомое время
	cout <<"time passed: "<< (float)time/CLOCKS_PER_SEC<<"sec"<<"\n";

	
	/*//Проверка матрицы
	ofstream fout;
	fout.open("матрица цэ.txt");
	
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			fout << c[getSingleIndex(i, j, n)] << "  ";
		}
		fout << "\n";
	}

	fout.close();*/
	
	del_matrix(a);
	del_matrix(b_t);
	del_matrix(c);
}


/*
int main()
{
	omp_set_num_threads(4);
	//int n = 2667;
	int n = 2667;
	mmul(make_matrix_of_1(n), make_matrix_of_2(n), make_matrix(n), n);
	system("pause");
	return 0;
}
//*/

//*
int main(int argc, char* argv[])
{
	enable_profiling();
	int n;
	int num_threads;
	if (argc != 3) {
		cout << "Incorrect input. Try again.\n" <<
			"    The input data should contain:\n 1)number of threads in integer;\n 2)dimention of matrix in integer shared by space;\n";
		exit(1);
	}
	else {
		n = atoi(argv[2]);
		num_threads = atoi(argv[1]);
	}

	if (num_threads < 1) {
		cout << "\n" << "Please, enter the correct number of threads in integer\n";
		exit(1);
	}
	omp_set_num_threads(num_threads);





	mmul(make_matrix_of_1(n), make_matrix_of_2(n), make_matrix(n), n);

	atexit(disable_profiling);

	return 0;
}//*/