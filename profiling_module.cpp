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
// #include <direct.h>

// #include "windows.h"

// #include <cstring>
#include <string>
#include <cstdio>



//#include "compute.h"

const std::string dlm = " ";

using namespace std;

ofstream fout;
static double begin_epoch;
static int cnt = 0;

int set_new_thread_id(void) {
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

static vector<data_callback_ompt_to_file_t> begin_threads_data_to_file;
static vector<data_callback_ompt_to_file_t> data_to_file;
static vector<data_callback_ompt_to_file_t> data_to_file_thread_end;

void callback_parallel_begin(
    ompt_data_t* encountering_task_data,         /* data of encountering task           */
    const ompt_frame_t *encountering_task_frame,  /* frame data of encountering task     */
    ompt_data_t *parallel_data,                  /* data of parallel region             */
    unsigned int requested_parallelism,          /* requested number of threads in team */
    int flag,                                    /* flag for invocation attribute       */
    const void *codeptr_ra                       /* return address of runtime call      */
) {
    //printf("[INFO] parallel begin (%u threads requested)\n", requested_parallelism);
    //fout.open("__time_points_data.txt", ios::out | ios::app);
    fout.flush() << ompt_callback_parallel_begin << dlm << (double)omp_get_wtime() - begin_epoch << dlm << ompt_get_thread_data()->value << endl;
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
        fout.flush() << data_to_file[i].which_callback << /*dlm << data_to_file[i].begin_or_end << */
            dlm << data_to_file[i].process_time << dlm << data_to_file[i].thread_id << endl;
    }
    fout << ompt_callback_parallel_end << dlm << (double)omp_get_wtime() - begin_epoch << dlm << ompt_get_thread_data()->value << endl;
    data_to_file.clear();
    //fout.close();
    //printf("[INFO] parallel end\n");
}

static std::vector<data_callback_ompt_to_file_t> log_threads_begin;

void callback_thread_begin(
    ompt_thread_t thread_type,            /* type of thread                      */
    ompt_data_t *thread_data              /* data of thread                      */
) {
    // parser * b = reinterperet_cast<parser *>(a);
    //    lock(b->lock);
    int thread_id = set_new_thread_id(); //use atomic
    thread_data->value = thread_id;
    std::string str = std::to_string(ompt_callback_thread_begin) + dlm + std::to_string((double)omp_get_wtime() - begin_epoch) + dlm + std::to_string(ompt_get_thread_data()->value) + "\n";
    fout << str;
    // unlock(b->mutex); // try to use cout and print with one row

}

unsigned int iter = 0;

void callback_thread_end(
    ompt_data_t *thread_data              /* data of thread                       */
) {
    //printf("[INFO] threads end (%llu)\n", thread_data->value);
    // // #pragma omp critical // TODO: fix
    // // {
    //  // ofstream fout;
    //  // fout.open("__time_points_data.txt", ios::out | ios::app);
    //  std::string str = std::to_string(ompt_callback_thread_end) + dlm + std::to_string((double)omp_get_wtime() - begin_epoch) + dlm + std::to_string(ompt_get_thread_data()->value) + "\n";
    //  fout << str;
    //  // fout.close();
    // // }
    omp_lock_t lock;
    omp_init_lock(&lock);
    data_callback_ompt_to_file_t* bufer = new data_callback_ompt_to_file_t;
    bufer->which_callback = ompt_callback_thread_end;
    bufer->process_time = (double)omp_get_wtime() - begin_epoch;
    bufer->thread_id = ompt_get_thread_data()->value;
    omp_set_lock(&lock);
    data_to_file_thread_end.push_back(*bufer);
    delete bufer;
    omp_destroy_lock(&lock);
    iter++;
    if (iter == cnt)
    {
        const std::string dlm = " ";
        ofstream fout;
        fout.open("__time_points_data.txt", ios::out | ios::app);
        // std::cout << "finalize vector size = " << data_to_file_thread_end.size() << std::endl;
        // for (int i = 0; i < log_threads_begin.size(); i++) {
        //  fout.flush() << log_threads_begin[i].which_callback <<
        //      dlm << log_threads_begin[i].process_time << dlm << log_threads_begin[i].thread_id << endl;  
        // }
        for (int i = 0; i < data_to_file_thread_end.size(); i++) {
            fout << data_to_file_thread_end[i].which_callback <<
                dlm << data_to_file_thread_end[i].process_time << dlm << data_to_file_thread_end[i].thread_id << endl;
            fout.flush();  
        }
        data_to_file_thread_end.clear();
        fout.close();
    }
    // fout << ompt_callback_thread_end << dlm << (double)omp_get_wtime() - begin_epoch << dlm << ompt_get_thread_data()->value << endl;
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
#pragma omp critical // TODO: fix
    {
    if (endpoint == ompt_scope_begin) {
        // printf("[INFO] Worker scope begin (%llu)\n", ompt_get_thread_data()->value);

        //fout.flush() << ompt_callback_work << dlm << (double)omp_get_wtime() - begin_epoch << dlm << ompt_get_thread_data()->value << endl;
        bufer->which_callback = ompt_callback_work;
        bufer->begin_or_end = ompt_scope_begin;
        bufer->process_time = (double)omp_get_wtime() - begin_epoch;
        bufer->thread_id = ompt_get_thread_data()->value;
        //printf("[INFO] Worker scope begin (%llu)\n", ompt_get_thread_data()->value);
    }

    if (endpoint == ompt_scope_end) {

        //fout.flush() << ompt_callback_work << dlm << (double)omp_get_wtime() - begin_epoch << dlm << ompt_get_thread_data()->value << endl;
        //data_callback_ompt_to_file_t* bufer = new data_callback_ompt_to_file_t;

        bufer->which_callback = ompt_callback_work;
        bufer->begin_or_end = ompt_scope_end;
        bufer->process_time = (double)omp_get_wtime() - begin_epoch;
        bufer->thread_id = ompt_get_thread_data()->value;
        //printf("[INFO] Worker scope end (%llu)\n", ompt_get_thread_data()->value);
    }
    //omp_set_lock(&lock);
    data_to_file.push_back(*bufer);
    delete bufer;
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
    // fout.open("__time_points_data.txt", ios::out | ios::app);
    fout.open("__time_points_data.txt");

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
    printf("[INFO] ompt_initialize is finished\n");

    return 1;// 1 = success
}

void ompt_finalize(ompt_data_t *tool_data) {
    printf("[INFO] ompt_finalize is called\n");
    // ofstream fout;
    // fout.open("__time_points_data.txt", ios::out | ios::app);
    // // std::cout << "finalize vector size = " << data_to_file_thread_end.size() << std::endl;
    // // for (int i = 0; i < log_threads_begin.size(); i++) {
    // //  fout.flush() << log_threads_begin[i].which_callback <<
    // //      dlm << log_threads_begin[i].process_time << dlm << log_threads_begin[i].thread_id << endl;  
    // // }
    // for (int i = 0; i < data_to_file_thread_end.size(); i++) {
    //     fout << data_to_file_thread_end[i].which_callback <<
    //         dlm << data_to_file_thread_end[i].process_time << dlm << data_to_file_thread_end[i].thread_id << endl;
    //     fout.flush();  
    // }
    // data_to_file_thread_end.clear();
    // fout.close();
    return;
    ompt_start_tool_result_t* result = (ompt_start_tool_result_t*)tool_data->ptr;
    delete result;
}

ompt_start_tool_result_t* ompt_start_tool(unsigned int omp_version, const char *runtime_version) {
    printf("[INFO] ompt_start_tool is called with omp_version = %u and runtime_version = %s\n", omp_version, runtime_version);
    ompt_start_tool_result_t* result = new ompt_start_tool_result_t;
    result->initialize = ompt_initialize;
    result->finalize = ompt_finalize;
    result->tool_data.ptr = (void*)result;
    return result;
}
