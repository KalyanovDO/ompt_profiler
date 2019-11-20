//#include <C:\Program Files (x86)\Microsoft Visual Studio\Shared\Python36_64\include\Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <iostream>
//#include "omp-tools.h"
#include <omp-tools.h>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <direct.h>

#include "windows.h"

#include <cstring>
#include <cstdio>



//#include "compute.h"

using namespace std;

ofstream fout;
static double begin_epoch;

int set_new_thread_id(void) {
	static int cnt = 0;
	cnt++;
	return cnt;
}

static ompt_get_thread_data_t ompt_get_thread_data;

typedef struct data_callback_ompt_to_file_t {
	ompt_callbacks_t which_callback;
	ompt_scope_endpoint_t begin_or_end;
	double process_time;
	uint64_t thread_id;
}data_callback_ompt_to_file_t;

static vector<data_callback_ompt_to_file_t> data_to_file;

void callback_parallel_begin(
	ompt_data_t* encountering_task_data,         /* data of encountering task           */
	const omp_frame_t *encountering_task_frame,  /* frame data of encountering task     */
	ompt_data_t *parallel_data,                  /* data of parallel region             */
	unsigned int requested_parallelism,          /* requested number of threads in team */
	int flag,                                    /* flag for invocation attribute       */
	const void *codeptr_ra                       /* return address of runtime call      */
) {
	//printf("[INFO] parallel begin (%u threads requested)\n", requested_parallelism);
	data_to_file.reserve(requested_parallelism);
	//fout.open("__time_points_data.txt", ios::out | ios::app);
	fout << ompt_callback_parallel_begin << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
	//fout.close();
}

void callback_parallel_end(
	ompt_data_t *parallel_data,           /* data of parallel region             */
	ompt_data_t *encountering_task_data,  /* data of encountering task           */
	int flag,                             /* flag for invocation attribute       */
	const void *codeptr_ra                /* return address of runtime call      */
) {
	//.open("__time_points_data.txt", ios::out | ios::app);
	for (int i = 0; i < data_to_file.size(); i++) {
		fout << data_to_file[i].which_callback << /*" " << data_to_file[i].begin_or_end << */
			" " << data_to_file[i].process_time << " " << data_to_file[i].thread_id << endl;
	}
	fout << ompt_callback_parallel_end << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
	data_to_file.clear();
	//fout.close();
	//printf("[INFO] parallel end\n");
}

void callback_thread_begin(
	ompt_thread_t thread_type,            /* type of thread                      */
	ompt_data_t *thread_data              /* data of thread                      */
) {
	thread_data->value = set_new_thread_id();
	//fout.open("__time_points_data.txt", ios::out | ios::app);

	//printf("[INFO] %s (%d) thread start (%llu)\n", thread_type == ompt_thread_worker ? "Worker" : "Other", thread_type, thread_data->value);
	fout << ompt_callback_thread_begin << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
	//fout.close();

}

void callback_thread_end(
	ompt_data_t *thread_data              /* data of thread                       */
) {
	//printf("[INFO] threads end (%llu)\n", thread_data->value);
	data_callback_ompt_to_file_t* bufer = new data_callback_ompt_to_file_t;
	omp_lock_t lock;
	omp_init_lock(&lock);
	bufer->which_callback = ompt_callback_thread_end;
	bufer->process_time = (double)omp_get_wtime() - begin_epoch;
	bufer->thread_id = ompt_get_thread_data()->value;
	omp_set_lock(&lock);
	data_to_file.push_back(*bufer);
	delete[] bufer;
	omp_destroy_lock(&lock);
	//fout << ompt_callback_thread_end << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
}

void callback_work(
	ompt_work_t wstype,              /* type of work region                 */
	ompt_scope_endpoint_t endpoint,       /* endpoint of work region             */
	ompt_data_t *parallel_data,           /* data of parallel region             */
	ompt_data_t *task_data,               /* data of task                        */
	uint64_t count,                       /* quantity of work                    */
	const void *codeptr_ra                /* return address of runtime call      */
) {

	data_callback_ompt_to_file_t* bufer = new data_callback_ompt_to_file_t;
	//omp_lock_t lock;
	//omp_init_lock(&lock);
#pragma omp critical
	{
		if (endpoint == ompt_scope_begin) {

			//fout.flush() << ompt_callback_work << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
			bufer->which_callback = ompt_callback_work;
			bufer->begin_or_end = ompt_scope_begin;
			bufer->process_time = (double)omp_get_wtime() - begin_epoch;
			bufer->thread_id = ompt_get_thread_data()->value;
			//printf("[INFO] Worker scope begin (%llu)\n", ompt_get_thread_data()->value);
		}

		if (endpoint == ompt_scope_end) {

			//fout.flush() << ompt_callback_work << " " << (double)omp_get_wtime() - begin_epoch << " " << ompt_get_thread_data()->value << endl;
			//data_callback_ompt_to_file_t* bufer = new data_callback_ompt_to_file_t;

			bufer->which_callback = ompt_callback_work;
			bufer->begin_or_end = ompt_scope_end;
			bufer->process_time = (double)omp_get_wtime() - begin_epoch;
			bufer->thread_id = ompt_get_thread_data()->value;
			//printf("[INFO] Worker scope end (%llu)\n", ompt_get_thread_data()->value);
		}
		//omp_set_lock(&lock);
		data_to_file.push_back(*bufer);
		delete[] bufer;
	}
	//omp_destroy_lock(&lock);
	return;
}

void callback_master(
	ompt_scope_endpoint_t endpoint,       /* endpoint of master region           */
	ompt_data_t *parallel_data,           /* data of parallel region             */
	ompt_data_t *task_data,               /* data of task                        */
	const void *codeptr_ra                /* return address of runtime call      */
) {
	//if (endpoint == ompt_scope_begin) printf("[INFO] Master scope begin (%llu)\n", ompt_get_thread_data()->value);
}

int ompt_initialize(ompt_function_lookup_t lookup, ompt_data_t *tool_data) {
	fout.open("__time_points_data.txt", ios::out | ios::app);

	printf("[INFO] ompt_initialize is called\n");
	begin_epoch = omp_get_wtime();        /*time normalization                    */

	ompt_set_callback_t set_callback = (ompt_set_callback_t)lookup("ompt_set_callback");
	set_callback(ompt_callback_parallel_begin, (ompt_callback_t)callback_parallel_begin);
	set_callback(ompt_callback_parallel_end, (ompt_callback_t)callback_parallel_end);
	set_callback(ompt_callback_thread_begin, (ompt_callback_t)callback_thread_begin);
	set_callback(ompt_callback_thread_end, (ompt_callback_t)callback_thread_end);
	set_callback(ompt_callback_work, (ompt_callback_t)callback_work);
	set_callback(ompt_callback_master, (ompt_callback_t)callback_master);

	
	ompt_get_thread_data = (ompt_get_thread_data_t)lookup("ompt_get_thread_data");

	return 1;// 1 = success
}

void ompt_finalize(ompt_data_t *tool_data) {
	printf("[INFO] ompt_finalize is called\n");
	//fout.open("__time_points_data.txt", ios::out | ios::app);
	for (int i = 0; i < data_to_file.size(); i++) {
		fout << data_to_file[i].which_callback <<
			" " << data_to_file[i].process_time << " " << data_to_file[i].thread_id << endl;
	}
	data_to_file.clear();
	fout.close();
	return;
	ompt_start_tool_result_t* result = (ompt_start_tool_result_t*)tool_data->ptr;
	delete[] result;
}

ompt_start_tool_result_t* ompt_start_tool(unsigned int omp_version, const char *runtime_version) {
	printf("[INFO] ompt_start_tool is called with omp_versoin = %u and runtime_version = %s\n", omp_version, runtime_version);
	ompt_start_tool_result_t* result = new ompt_start_tool_result_t;
	result->initialize = ompt_initialize;
	result->finalize = ompt_finalize;
	result->tool_data.ptr = (void*)result;
	return result;
}


void run_py_script() {
	//Py_Initialize();
	//const char* script_path = "..\\profiling_modules\\imaging_profiling.py";
	//char* py_argv[] = { strdup(script_path), strdup("") };
	//PySys_SetArgv(2, (wchar_t**)py_argv);
	//PyRun_SimpleFileEx(fopen(script_path, "r"), script_path, 1);
	//free(py_argv[0]);
	//free(py_argv[1]);
	//Py_Finalize();

	//setlocale(0, "rus");
	//wchar_t* buffer;
	//// Get the current working directory: 
	//if ((buffer = _wgetcwd(NULL, 0)) == NULL)
	//	perror("_getcwd error");
	//else
	//{
	//	wprintf(L"%s \n", buffer);
	//	//free(buffer);
	//}

	_chdir("..");
	_chdir("..");
	//_chdir("matrixmul");
	_chdir("profiling_modules");
	system(R"(py imaging_profiling_1.1.py)");
}

//int main(int argc, char* argv[]) {
//	fout.open("time_points_data.txt");
//	main_func(argc, argv);
//	fout << ompt_callback_thread_end << " " << (double)omp_get_wtime() - begin_epoch << endl;
//
//	fout.close();
//	atexit(run_py_script);
//
//	return 0;
//}

void enable_profiling() {
	fout.open("__time_points_data.txt");
}

void disable_profiling(){
	fout.close();
	run_py_script();
	return;
}