#include <iostream>
#include <ctime>  
#include <cstdlib>
#include <fstream>
#include <omp.h>
#include <vector>
// #include "../../profiling_module.h"

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


//задание матрицы с диагональным преобладанием
float* make_random_system(int n)
{
	int m = n * (n + 1);
	float *arr = new float[m];
	   
	// задание матрицы случайных чисел и свободного вектора вместе
	srand((unsigned)time(0));
	float min_interval = 0.1f;
	float max_interval = 1.1f;
	for (int i = 0; i < m; i++) {
		arr[i] = min_interval + ((float)rand() / RAND_MAX) * (max_interval - min_interval);
	}

	min_interval = (float) 1.01;
	max_interval = (float) 1.5;
	//установление диагонального преобладания в матрице
	int count_number_col = 0;
	for (int i = 0; i < m-n; i += n+1 ) {
		float bufer = 0;
		for (int j = 0; j < n; j++) {
			bufer += arr[i + j - count_number_col];
		}
		arr[i] = bufer * (min_interval + ((float)rand() / RAND_MAX) * (max_interval - min_interval));
		count_number_col++;
	}
	return arr;
}


//возвращает произвольную матрицу с диагональным преобладанием
float* make_random_a(int n)
{
	int m = n * n ;
	float *arr = new float[m];
	float* bufer = make_random_system(n);
	for (int i = 0; i < m; i++) {
		arr[i] = bufer[i];
	}
	delete[] bufer;
	return arr;
}

//возвращает вектор свободных членов
float* make_random_b(int n)
{
	int m = n * n;
	float *arr = new float[n];
	float* bufer = make_random_system(n);
	for (int i = m; i < m + n; i++) {
		arr[i-m] = n*bufer[i];//порядок свободных членов близок к диагональным членам
	}
	delete[] bufer;
	return arr;
}


//метод Якоби во плоти
void jakobi_method(float* a, float* b, int n, float err, int end_iter)
{
	float begin = (float)clock();
	//0е приближение по x
	float *x_iter_old = new float[n];
	float *x_iter_young = new float[n];

	
//#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		x_iter_old[i] = b[i] / a[getSingleIndex(i, i, n)];
		x_iter_young[i] = (float)0.;
	}

	
	float norma;
	int k = 0;//счётчик итераций
	float *x_iter_err = new float[n];
	float *b_err = new float[n];
	
	//последующие приближения
	do 
	{
#pragma omp parallel for
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i != j) x_iter_young[i] += - (a[getSingleIndex(i, j, n)] * x_iter_old[j]) / a[getSingleIndex(i, i, n)];
				else x_iter_young[i] += b[i] / a[getSingleIndex(i, i, n)];
			}
		}


//#pragma omp parallel for
		for (int i = 0; i < n; i++) {
			b_err[i] = 0;
			for (int j = 0; j < n; j++) {
				b_err[i] += a[getSingleIndex(i, j, n)] * x_iter_young[j];
			}
			b_err[i] = abs(b[i] - b_err[i]) / abs(b[i]);
		}

		norma = abs(x_iter_young[0] - x_iter_old[0]);

//#pragma omp parallel for
		for (int i = 0; i < n; i++) {
			x_iter_err[i] = x_iter_young[i] - x_iter_old[i];
			if (abs(x_iter_err[i]) > norma)
				norma = abs(x_iter_err[i]);
			x_iter_old[i] = x_iter_young[i];
			x_iter_young[i] = 0;
		}

		k++;

	} while (norma > err && k < end_iter);

	float end = (float)clock();
	float time_of_work_sec = (float) (end - begin) / CLOCKS_PER_SEC;


	/*
	ofstream fout;
	fout.open("filetest.txt");
	fout << "number of iteration = " << k << "\n\n";
	fout << "vector delta_x = \n";
	for (int i = 0; i < n; i++) {
		fout << "( " << x_iter_err[i] << " )\n";
	}
	fout << "\n";
	fout << " vector accuracy of solution(|b-b'|) = \n";
	for (int i = 0; i < n; i++) {
		fout << "( " << b_err[i] << " )\n";
	}
	fout << "\n";
	fout << "time of work = " << time_of_work_sec << " sec\n";
	fout.close();
	//*/

	cout << "\n" << "number of iteration = " << k << "\n";
	cout << "maximum delta_x = ";
	float max_iter_err = 0;
	for (int i = 0; i < n; i++) {
		if (max_iter_err < abs(x_iter_err[i])) max_iter_err = abs(x_iter_err[i]);
	}
	cout << max_iter_err << "\n";
	

	cout << "maximum accuracy of solution(|b-b'|/|b|) = ";
	float max_b_err = 0;
	for (int i = 0; i < n; i++) {
		if (max_b_err < abs(b_err[i])) max_b_err = abs(b_err[i]);
	}
	cout  << max_b_err << "\n";
	cout << "\n";
	cout << "time of work = " << time_of_work_sec << " sec\n\n";


	delete[] x_iter_err;
	delete[] x_iter_old;
	delete[] x_iter_young;
	delete[] b_err;
	delete[] a;
	delete[] b;
}

/*
int main() {
	int n = 5001; //размерность задачи
	float err = (float) 0.0001;//требуемая точность решения
	int max_k = 500;//допустимое число итераций
	int num_threads = 5;
	omp_set_num_threads(num_threads);
	jakobi_method(make_random_a(n), make_random_b(n), n, err, max_k);
	system("pause");
	return 0;
}
//*/

//*
int main(int argc, char* argv[])
{
	// enable_profiling();
	int n; //размерность задачи
	float err;//требуемая точность решения
	int max_k;//допустимое число итераций
	int num_threads;
	if (argc != 5) {
		cout << "Incorrect input. Try again.\n" <<
			 "The input data should contain three numbers shared by space:\n"<<
			 " - dimention of system (int n);\n"<<
			 " - accuracy of the solution (float err);\n"<<
			 " - maximum number of iterations (int max_k);\n"<<
			 " - number of threads (int  num_threads).\n";
		exit(1);
	}
	else {
		n = atoi(argv[1]);
		err = (float)atof(argv[2]);
		max_k = atoi(argv[3]);
		num_threads = atoi(argv[4]);
	}

	if (n < 1) {
		cout << "\n" << "Please, enter the correct dimention of system \n";
		exit(1);
	}	
	if (err <= (float) 0.) {
		cout << "\n" << "Please, enter the correct accuracy of the solution \n";
		exit(1);
	}
	if (max_k < 1) {
		cout << "\n" << "Please, enter the correct maximum number of iterations \n";
		exit(1);
	}
	if (num_threads < 1) {
		cout << "\n" << "Please, enter the correct number of threads in integer\n";
		exit(1);
	}
	omp_set_num_threads(num_threads);

	
	//вызываем функцию метода якоби


	jakobi_method(make_random_a(n), make_random_b(n), n, err, max_k);

	// atexit(disable_profiling);
	// exit(0);
	//time_points.push_back(make_pair(2, omp_get_wtime()));
	return 0;
}//*/
