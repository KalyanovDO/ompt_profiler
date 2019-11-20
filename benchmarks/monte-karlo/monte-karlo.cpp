#include <iostream>
#include <ctime>  
#include <cstdlib>
//#include <fstream>
#include <omp.h>
#include <vector>
//#include "../profiling_modules/profiling_module.h"

using namespace std;
int dimention_of_space = 3;

/*методом монте-карло находим интеграл функции
f(x,y)=(cos(3x) + 1)/(3 + 2y^2) в кубе размеров [0;1]*/


float function(float x, float y)
{
	return  (cos((float)3. * x) + (float)1) / ((float)3. + (float)2. * y * y);
}

float integrate_value(void)
{
	return  ((float)3. + sin((float)3.)) * atan(sqrt((float)2. / (float)3.)) / ((float)3. * sqrt((float)6.));
}

float** make_random_array(int number_of_points)
{
	float** arr = new float* [dimention_of_space];
	for (int i = 0; i < dimention_of_space; i++) {
		arr[i] = new float[number_of_points];
	}
	
	float min_interval = 0.;
	float max_interval = 1.;
	float randnum;
//	ofstream fout("out.txt");


		// здесь должен быть генератор квазислучайных чисел...


	srand(clock());

	for (int i = 0; i < dimention_of_space; i++) {
		for (int j = 0; j < number_of_points; j++) {
			randnum = (float)rand() / RAND_MAX;
			randnum *= (float) 0.99 + (float) 0.02 * (float)rand() / RAND_MAX;//теоретически возможно превышение интервала, но вероятность мала
			arr[i][j] = min_interval + (randnum) * (max_interval - min_interval);
		}
	}
//	fout.close();
	return arr;
}

//близко к правде получается при числе точек не менее е+7
void mk_method(int number_of_points, float err_value, int number_of_threads)
{
	cout << " real integral value = " << integrate_value() << "\n";
	float begin = (float)clock();

	float** bufer = make_random_array(number_of_points);
	float* x;
	float* y;
	float* z;

	x = bufer[0];
	y = bufer[1];
	z = bufer[2];
	
	delete[] bufer;

	
	int counter = 0;//считает точки, попавшие под функцию
	int sum = 0;
	float new_integral = 0;
	float old_integral = 0;
	float delta_integral;
	int err_count = 0;//считает количество проверок на удовлетворение точности
	float output1;
	int output2;
	float output3;
	int residual_n = number_of_points;
	int iteration_n = 1000000;
	float time_of_work_sec1 = 0;


	while (residual_n > 0)
	{

		float begin1 = (float)clock();
#pragma omp parallel num_threads(number_of_threads)
		{
#pragma omp for reduction(+:counter)
			for (int i = number_of_points - residual_n; i < number_of_points - residual_n + iteration_n; i++) {
				if (z[i] <= function(x[i], y[i])) counter++;
			}
		}
		float end1 = (float)clock();
		time_of_work_sec1 += (float)(end1 - begin1) / CLOCKS_PER_SEC;


		//отслеживание числа использованных точек
		if (residual_n / iteration_n > 0) {
			residual_n -= iteration_n;
		}
		else {
			iteration_n = residual_n;
			residual_n = 0;
		}


		old_integral = new_integral;
		new_integral = (float)counter;
		delta_integral = (float)(new_integral - old_integral) / (float)(number_of_points - residual_n);
		if (delta_integral <= err_value && abs((float)new_integral /
			(float)(number_of_points - residual_n)-integrate_value()) <= err_value) {
			err_count++;
		}

		if (err_count == 1 || residual_n == 0) {

			{
				output1 = (float)counter / (float)(number_of_points - residual_n);
				output2 = number_of_points - residual_n;
				output3 = abs((float)counter / (float)(number_of_points - residual_n)-integrate_value());
			}
			break;
		}		
	} 

	cout << " calculated value of integral = " << output1 << "\n";
	cout << " number of used points = " << output2 << " \n";
	cout << " absolute error of integrate = " << output3 << "\n";
	delete[] x;
	delete[] y;
	delete[] z;
	
	float end = (float)clock();
	float time_of_work_sec = (float) (end - begin) / CLOCKS_PER_SEC;
	cout << "time of work = " << time_of_work_sec << " sec\n";
	cout << "time of parallel work = " << time_of_work_sec1 << " sec\n";
}

/*
int main()
{
	int n = 39999999;
	float err = (float) 0.00001;
	mk_method(n, err,4);
	system("pause");
}
//*/

//*
int main(int argc, char* argv[])
{
	//enable_profiling();
	int n; //размерность задачи
	float err;//требуемая точность решения
	int number_of_threads;//число создаваемых потоков

	if (argc != 4) {
		cout << "Incorrect input. Try again.\n" <<
			"The input data should contain two numbers shared by space:\n" <<
			" - number of random points (int n) (not less then 10**6 because of iteration_n);\n" <<
			" - integral calculation accuracy (float err);\n"<<
			" - number of threads (int number_of_threads).\n";
		exit(1);
	}
	else {
		n = atoi(argv[1]);
		err = (float)atof(argv[2]);
		number_of_threads = atoi(argv[3]);
	}

	if (n < 1) {
		cout << "\n" << "Please, enter the correct number of random points \n";
		exit(1);
	}
	if (err <= (float) 0.) {
		cout << "\n" << "Please, enter the correct integral calculation accuracy \n";
		exit(1);
	}
	if (number_of_threads < 1) {
		cout << "\n" << "Please, enter the correct number of threads \n";
		exit(1);
	}


	

	mk_method(n, err, number_of_threads);

	//atexit(disable_profiling);

	return 0;
}//*/